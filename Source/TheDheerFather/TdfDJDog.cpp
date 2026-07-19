#include "TdfDJDog.h"
#include "TdfAttributeSet.h"
#include "TdfGameState.h"
#include "TdfKillerCharacter.h"
#include "TdfRunnerCharacter.h"
#include "TdfDeployables.h"
#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

ATdfDJDog::ATdfDJDog()
{
	// Dog-sized and self-driving.
	GetCapsuleComponent()->SetCapsuleSize(45.f, 55.f);
	if (BodyMesh)
	{
		BodyMesh->SetRelativeScale3D(FVector(1.1f, 0.6f, 0.55f));
		BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, -55.f));
		BodyMesh->SetOwnerNoSee(false); // nobody is inside the dog
	}

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AAIController::StaticClass();

	MoveSpeed = 115.f; // faster than every runner except a sprinting Shaun
}

void ATdfDJDog::InitializeAttributes()
{
	if (AttributeSet)
	{
		AttributeSet->SetMaxHealth(200.f);
		AttributeSet->SetHealth(200.f);
		AttributeSet->SetMaxStamina(100.f);
		AttributeSet->SetStamina(100.f);
		AttributeSet->SetMoveSpeed(MoveSpeed);
	}
}

void ATdfDJDog::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || bIsDown || bIsStunned)
	{
		return;
	}

	// Belt and braces: a dog with no brain or no legs is just a rug.
	if (!GetController())
	{
		SpawnDefaultController();
	}
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 460.f; // absolute cm/s — bypasses the player speed pipeline
		Move->MaxAcceleration = 2048.f;
		if (Move->MovementMode == MOVE_None || Move->MovementMode == MOVE_Flying)
		{
			Move->SetMovementMode(MOVE_Walking);
		}
	}

	// PUSH: runners can shove him around — he's a dog, not a boulder.
	for (TActorIterator<ATdfCharacterBase> It(GetWorld()); It; ++It)
	{
		ATdfCharacterBase* Other = *It;
		if (!Other || Other == this || Other->IsDead())
		{
			continue;
		}
		const FVector Away = GetActorLocation() - Other->GetActorLocation();
		if (Away.Size2D() < 130.f && Other->GetVelocity().Size2D() > 30.f)
		{
			if (UCharacterMovementComponent* Move = GetCharacterMovement())
			{
				Move->AddImpulse(Away.GetSafeNormal2D() * 420.f, true);
			}
			break;
		}
	}

	// TEMP diagnostics (remove once DJ is confirmed moving in the wild).
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(77, 1.f, FColor::Yellow,
			FString::Printf(TEXT("DJ: mode=%d ctrl=%s speed=%.0f v=%.0f"),
				GetCharacterMovement() ? (int32)GetCharacterMovement()->MovementMode.GetValue() : -1,
				GetController() ? TEXT("yes") : TEXT("NO"),
				GetCharacterMovement() ? GetCharacterMovement()->MaxWalkSpeed : -1.f,
				GetVelocity().Size2D()));
	}

	BiteCooldownRemaining = FMath::Max(0.f, BiteCooldownRemaining - DeltaSeconds);
	MarkCooldownRemaining = FMath::Max(0.f, MarkCooldownRemaining - DeltaSeconds);
	GrabCooldownRemaining = FMath::Max(0.f, GrabCooldownRemaining - DeltaSeconds);

	// --- DRAGGING: haul the victim back to Lucki or the Prius, whichever's closer ---
	if (DraggedRunner.IsValid())
	{
		ATdfRunnerCharacter* Victim = DraggedRunner.Get();
		if (Victim->IsDead() || Victim->HasEscaped())
		{
			ReleaseDragged(false);
		}
		else
		{
			FVector Dest = GetActorLocation();
			float BestDestDist = 1e12f;
			if (const AActor* Master = GetOwner())
			{
				Dest = Master->GetActorLocation();
				BestDestDist = FVector::Dist(Dest, GetActorLocation());
			}
			for (TActorIterator<ATdfPriusProp> It(GetWorld()); It; ++It)
			{
				const float Dist = FVector::Dist((*It)->GetActorLocation(), GetActorLocation());
				if (Dist < BestDestDist)
				{
					BestDestDist = Dist;
					Dest = (*It)->GetActorLocation();
				}
			}

			if (BestDestDist < 260.f)
			{
				// Delivered: dump them at the boss's feet, dazed.
				ReleaseDragged(true);
			}
			else
			{
				if (AAIController* AI = Cast<AAIController>(GetController()))
				{
					AI->MoveToLocation(Dest, 120.f, true, false, false, false);
				}
				AddMovementInput((Dest - GetActorLocation()).GetSafeNormal2D(), 1.f);
			}
			return; // dragging overrides everything else
		}
	}

	// The nearest living runner.
	ATdfRunnerCharacter* Target = nullptr;
	float BestDist = AggroRange;
	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		ATdfRunnerCharacter* Runner = *It;
		if (!Runner || Runner->IsDead() || Runner->HasEscaped() || Runner->bIsDown)
		{
			continue;
		}
		const float Dist = FVector::Dist(Runner->GetActorLocation(), GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist = Dist;
			Target = Runner;
		}
	}

	// Biting only once the power's out; moving and sniffing around is always allowed.
	const bool bMayBite = ATdfGameState::KillersUnleashed(GetWorld());
	const AActor* Master = GetOwner();

	auto MoveToward = [this](const FVector& Loc) {
		// Primary: direct AI move request (no navmesh needed, drives the CMC itself).
		if (AAIController* AI = Cast<AAIController>(GetController()))
		{
			AI->MoveToLocation(Loc, 90.f, true, /*bUsePathfinding*/ false, false, false);
		}
		// Backup: raw movement input for the same frame.
		AddMovementInput((Loc - GetActorLocation()).GetSafeNormal2D(), 1.f);
	};
	auto TryBite = [&](ATdfRunnerCharacter* Victim, float Dist) {
		if (!bMayBite || !Victim || Dist > BiteRange || BiteCooldownRemaining > 0.f)
		{
			return;
		}
		BiteCooldownRemaining = BiteCooldown;
		if (UTdfAttributeSet* Attr = Victim->GetTdfAttributeSet())
		{
			const float NewHealth = FMath::Max(0.f, Attr->GetHealth() - BiteDamage);
			Attr->SetHealth(NewHealth);
			Victim->ApplyStun(BiteStun); // dragged down by the leg
			if (NewHealth <= 0.f)
			{
				Victim->OnHealthDepleted();
			}
		}
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("DJ BIT SOMEONE"));
		}
	};

	switch (DJMode)
	{
	case 0: // SCOUT — track and mark, never bite.
		if (Target)
		{
			if (BestDist > 700.f)
			{
				MoveToward(Target->GetActorLocation());
			}
			if (BestDist <= 1500.f && MarkCooldownRemaining <= 0.f)
			{
				MarkCooldownRemaining = 8.f;
				Multicast_MarkTarget(Target->GetActorLocation());
			}
		}
		break;

	case 2: // DEFEND — shadow Lucki; maul anyone who gets close to him.
		{
			ATdfRunnerCharacter* Threat = nullptr;
			float ThreatDist = 0.f;
			if (Target && Master
				&& FVector::Dist(Target->GetActorLocation(), Master->GetActorLocation()) < 900.f)
			{
				Threat = Target;
				ThreatDist = BestDist;
			}
			if (Threat)
			{
				MoveToward(Threat->GetActorLocation());
				TryBite(Threat, ThreatDist);
			}
			else if (Master && FVector::Dist(Master->GetActorLocation(), GetActorLocation()) > 350.f)
			{
				MoveToward(Master->GetActorLocation());
			}
		}
		break;

	default: // ATTACK — lone wolf: bite, then grab the leg and drag them home.
		if (Target)
		{
			MoveToward(Target->GetActorLocation());
			const bool bInGrabRange = BestDist <= BiteRange + 30.f;
			TryBite(Target, BestDist);
			if (bMayBite && bInGrabRange && GrabCooldownRemaining <= 0.f && !DraggedRunner.IsValid()
				&& !Target->IsDead() && !Target->bTiedToShrine && !Target->bBeingCarried)
			{
				GrabRunner(Target);
			}
		}
		break;
	}
}

void ATdfDJDog::GrabRunner(ATdfRunnerCharacter* Victim)
{
	DraggedRunner = Victim;
	Victim->bDraggedByDJ = true;
	Victim->SetActorEnableCollision(false);
	if (UCharacterMovementComponent* VictimMove = Victim->GetCharacterMovement())
	{
		VictimMove->DisableMovement();
	}
	Victim->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Victim->SetActorRelativeLocation(FVector(-130.f, 0.f, 5.f)); // dragged behind by the leg
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("DJ HAS SOMEONE BY THE LEG - dragging them to Lucki!"));
	}
}

void ATdfDJDog::ReleaseDragged(bool bDeliveredToMaster)
{
	ATdfRunnerCharacter* Victim = DraggedRunner.Get();
	DraggedRunner = nullptr;
	GrabCooldownRemaining = bDeliveredToMaster ? 3.f : 6.f;
	if (!Victim)
	{
		return;
	}
	Victim->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Victim->SetActorEnableCollision(true);
	Victim->bDraggedByDJ = false;
	if (bDeliveredToMaster)
	{
		Victim->ApplyStun(2.5f); // gift-wrapped for Lucki
	}
	if (!Victim->bIsDown && !Victim->bIsStunned)
	{
		if (UCharacterMovementComponent* VictimMove = Victim->GetCharacterMovement())
		{
			VictimMove->SetMovementMode(MOVE_Walking);
		}
	}
}

bool ATdfDJDog::TryStruggleFree(ATdfRunnerCharacter* Runner)
{
	if (DraggedRunner.Get() != Runner)
	{
		return false;
	}
	if (FMath::FRand() < 0.01f) // 1% per press
	{
		ReleaseDragged(false);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("BROKE FREE FROM DJ!"));
		}
		return true;
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(78, 0.5f, FColor::Orange, TEXT("struggling..."));
	}
	return false;
}

void ATdfDJDog::Multicast_MarkTarget_Implementation(FVector Location)
{
	// Only the killer sees DJ's marks.
	const APlayerController* LocalPC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (LocalPC && Cast<ATdfKillerCharacter>(LocalPC->GetPawn()))
	{
		DrawDebugSphere(GetWorld(), Location + FVector(0, 0, 100.f), 70.f, 10, FColor::Orange, false, 5.f, SDPG_Foreground, 2.f);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("*bark bark* DJ found someone!"));
		}
	}
}

void ATdfDJDog::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATdfDJDog, DJMode);
}

void ATdfDJDog::OnHealthDepleted()
{
	Super::OnHealthDepleted();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Silver, TEXT("DJ is down."));
	}
	if (HasAuthority())
	{
		SetLifeSpan(3.f); // body lingers, then despawns
	}
}
