#include "TdfDJDog.h"
#include "TdfAttributeSet.h"
#include "TdfGameState.h"
#include "TdfKillerCharacter.h"
#include "TdfRunnerCharacter.h"
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
		if (Move->MovementMode == MOVE_None)
		{
			Move->SetMovementMode(MOVE_Walking);
		}
	}

	BiteCooldownRemaining = FMath::Max(0.f, BiteCooldownRemaining - DeltaSeconds);
	MarkCooldownRemaining = FMath::Max(0.f, MarkCooldownRemaining - DeltaSeconds);

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

	default: // ATTACK — lone wolf.
		if (Target)
		{
			MoveToward(Target->GetActorLocation());
			TryBite(Target, BestDist);
		}
		break;
	}
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
