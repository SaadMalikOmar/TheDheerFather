// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#include "TdfKillers.h"
#include "TdfAttributeSet.h"
#include "TdfCharacterAbilities.h"
#include "TdfAbility_GroundSmash.h"
#include "TdfDJDog.h"
#include "TdfDeployables.h"
#include "TdfGameMode.h"
#include "TdfGameState.h"
#include "TdfRunnerCharacter.h"
#include "TdfShrine.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

// --- Skinny Bear (Mimic) ---

ATdfKiller_SkinnyBear::ATdfKiller_SkinnyBear()
{
	KillerType = ETdfKillerType::Mimic_SkinnyBear;
	Stats.Stamina = 200.f;
	Stats.Speed = 190.f;    // still faster than anyone, but no longer teleport-tier
	Stats.Kealth = 350.f;
	Stats.Damage = 999.f;   // one shot, one kill

	DefaultAbilities[3] = UTdfAbility_EchoScream::StaticClass();  // R
	DefaultAbilities[5] = UTdfAbility_Sniff::StaticClass();       // RMB (and E falls through to it)

	// The only third-person character: you should see the monster you are.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetCapsuleComponent());
	CameraBoom->TargetArmLength = 450.f;
	CameraBoom->SocketOffset = FVector(0.f, 60.f, 90.f);
	CameraBoom->bUsePawnControlRotation = true;

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(CameraBoom);
	ThirdPersonCamera->bAutoActivate = true;

	if (FirstPersonCamera)
	{
		FirstPersonCamera->SetAutoActivate(false);
		FirstPersonCamera->Deactivate();
	}
	if (BodyMesh)
	{
		BodyMesh->SetOwnerNoSee(false); // he sees his own body in third person
	}
}

void ATdfKiller_SkinnyBear::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	RevealTimeRemaining = FMath::Max(0.f, RevealTimeRemaining - DeltaSeconds);

	// His curse, applied on HIS screen only: still runners simply are not there.
	if (IsLocallyControlled())
	{
		for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
		{
			ATdfRunnerCharacter* Runner = *It;
			if (!Runner)
			{
				continue;
			}
			const bool bVisible = Runner->GetVelocity().Size2D() > StillnessSpeedThreshold
				|| Runner->bIsDown
				|| Runner->IsDead()
				|| RevealTimeRemaining > 0.f;
			Runner->SetBodyVisibleLocal(bVisible);
		}
	}
}

void ATdfKiller_SkinnyBear::StartSniffSlow(float Duration)
{
	if (!HasAuthority() || !AttributeSet)
	{
		return;
	}
	AttributeSet->SetMoveSpeed(Stats.Speed * 0.5f);
	GetWorldTimerManager().SetTimer(SniffTimerHandle, this, &ATdfKiller_SkinnyBear::EndSniffSlow, Duration, false);
}

void ATdfKiller_SkinnyBear::EndSniffSlow()
{
	if (!bIsDead && AttributeSet)
	{
		AttributeSet->SetMoveSpeed(Stats.Speed);
	}
}

// --- Lucki (Stalker) ---

ATdfKiller_Lucki::ATdfKiller_Lucki()
{
	PrimaryActorTick.bCanEverTick = true;

	KillerType = ETdfKillerType::Stalker_Lucki;
	Stats.Stamina = 50.f;
	Stats.Speed = 85.f;     // brisk waddle — the Prius is still the play
	Stats.Kealth = 150.f;
	Stats.Damage = 49.f;

	DefaultAbilities[1] = UTdfAbility_PlaceNaniTrap::StaticClass();  // Q
	DefaultAbilities[3] = UTdfAbility_ReleaseDJ::StaticClass();      // R
	DefaultAbilities[4] = UTdfAbility_DietCoke::StaticClass();       // T

	CarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarMesh"));
	CarMesh->SetupAttachment(GetCapsuleComponent());
	CarMesh->SetRelativeLocation(FVector(0.f, 0.f, -40.f));
	CarMesh->SetRelativeScale3D(FVector(4.2f, 1.9f, 1.2f));
	CarMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CarMesh->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CarAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CarAsset.Succeeded())
	{
		CarMesh->SetStaticMesh(CarAsset.Object);
	}
}

void ATdfKiller_Lucki::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// His pride and joy, parked next to wherever he wakes up.
	if (HasAuthority() && !ParkedCar.IsValid() && GetWorld())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		ParkedCar = GetWorld()->SpawnActor<ATdfPriusProp>(ATdfPriusProp::StaticClass(),
			GetActorLocation() + GetActorRightVector() * 350.f, GetActorRotation(), Params);
	}
}

void ATdfKiller_Lucki::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATdfKiller_Lucki, bInCar);
	DOREPLIFETIME(ATdfKiller_Lucki, Hunger);
}

void ATdfKiller_Lucki::ToggleVehicle()
{
	if (IsLocallyControlled())
	{
		ServerSetCar(!bInCar);
	}
}

void ATdfKiller_Lucki::ServerSetCar_Implementation(bool bNewInCar)
{
	if (bIsDown || bIsDead)
	{
		return;
	}

	if (bNewInCar)
	{
		// The Prius is a real car, not a genie: you have to walk to it. And unlock it.
		if (!ParkedCar.IsValid()
			|| FVector::Dist(ParkedCar->GetActorLocation(), GetActorLocation()) > CarEnterRange)
		{
			if (GEngine && IsLocallyControlled())
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Silver, TEXT("The Prius isn't here..."));
			}
			return;
		}
		if (ParkedCar->bLocked)
		{
			if (GEngine && IsLocallyControlled())
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Silver, TEXT("It's locked (RMB to unlock)"));
			}
			return;
		}
		ParkedCar->Driver = this;
		ParkedCar->SetActorHiddenInGame(true);
		ParkedCar->SetActorEnableCollision(false);
	}
	else if (ParkedCar.IsValid())
	{
		// Park it where he stops.
		ParkedCar->Driver = nullptr;
		ParkedCar->SetActorLocation(GetActorLocation() + GetActorForwardVector() * 250.f);
		ParkedCar->SetActorRotation(GetActorRotation());
		ParkedCar->SetActorHiddenInGame(false);
		ParkedCar->SetActorEnableCollision(true);

		// Bailing from a MOVING car: you eat the tarmac.
		if (GetVelocity().Size2D() > 500.f)
		{
			if (AttributeSet)
			{
				AttributeSet->SetHealth(FMath::Max(5.f, AttributeSet->GetHealth() - 20.f));
			}
			ApplyStun(1.2f); // down he goes, rolling
			bBailSlowed = true;
			GetWorldTimerManager().SetTimer(BailTimerHandle, this, &ATdfKiller_Lucki::EndBailSlow, 4.f, false);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, TEXT("LUCKI HIT THE TARMAC - rolling..."));
			}
		}
	}

	bInCar = bNewInCar;
	ApplyCarState();
}

void ATdfKiller_Lucki::EndBailSlow()
{
	bBailSlowed = false;
	if (!bIsDead)
	{
		ApplyCarState();
	}
}

void ATdfKiller_Lucki::HandleCarInteract()
{
	if (!HasAuthority())
	{
		return;
	}
	if (bInCar)
	{
		// E inside the car: keys in / keys out.
		if (ParkedCar.IsValid())
		{
			ParkedCar->ToggleKeys();
		}
		return;
	}
	// E outside: sit in if it's next to him.
	if (ParkedCar.IsValid()
		&& FVector::Dist(ParkedCar->GetActorLocation(), GetActorLocation()) <= CarEnterRange)
	{
		ServerSetCar_Implementation(true);
	}
}

void ATdfKiller_Lucki::OnSprintPressed()
{
	if (bInCar)
	{
		if (IsLocallyControlled())
		{
			ServerSetCar(false); // Shift = get out
		}
		return;
	}
	Super::OnSprintPressed();
}

void ATdfKiller_Lucki::OnJumpPressed()
{
	if (bInCar)
	{
		if (IsLocallyControlled())
		{
			ServerToggleHeadlights(); // SPACE = headlights while driving
		}
		return;
	}
	Super::OnJumpPressed();
}

void ATdfKiller_Lucki::ServerToggleHeadlights_Implementation()
{
	if (bInCar && ParkedCar.IsValid())
	{
		ParkedCar->ToggleHeadlights();
	}
}

void ATdfKiller_Lucki::OnSecondaryAction()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (bInCar)
	{
		ServerCarSecondary(); // engine start/stop
		return;
	}

	// Looking at the parked Prius? RMB locks/unlocks it.
	if (GetWorld())
	{
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		const FVector Start = GetPawnViewLocation();
		const FVector End = Start + GetControlRotation().Vector() * 700.f;
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params)
			&& Cast<ATdfPriusProp>(Hit.GetActor()))
		{
			ServerCarSecondary();
			return;
		}
	}

	ServerToggleDJMode();
}

void ATdfKiller_Lucki::ServerCarSecondary_Implementation()
{
	if (!ParkedCar.IsValid())
	{
		return;
	}
	if (bInCar)
	{
		ParkedCar->RequestEngineToggle();
	}
	else if (FVector::Dist(ParkedCar->GetActorLocation(), GetActorLocation()) <= 800.f)
	{
		ParkedCar->ToggleLock();
	}
}

void ATdfKiller_Lucki::ServerToggleDJMode_Implementation()
{
	for (TActorIterator<ATdfDJDog> It(GetWorld()); It; ++It)
	{
		if (ATdfDJDog* DJ = *It)
		{
			DJ->DJMode = (DJ->DJMode + 1) % 3;
			if (GEngine)
			{
				const TCHAR* ModeName = (DJ->DJMode == 0) ? TEXT("SCOUT - find them, boy")
					: (DJ->DJMode == 1) ? TEXT("ATTACK - GET 'EM") : TEXT("DEFEND - stay close");
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange,
					FString::Printf(TEXT("*whistle* DJ: %s"), ModeName));
			}
			return;
		}
	}
	if (GEngine && IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Silver, TEXT("DJ isn't out (R to release him)"));
	}
}

void ATdfKiller_Lucki::ApplyCarState()
{
	if (AttributeSet)
	{
		const bool bStarving = Hunger < StarvingThreshold;
		// Starving is painful but playable now (was 5 — "unplayable", correct).
		float FootSpeed = bStarving ? 35.f : Stats.Speed;
		if (bBailSlowed)
		{
			FootSpeed *= 0.35f; // limping after eating the tarmac
		}

		float DriveSpeed = 2.f; // engine off: the Prius is furniture
		if (ParkedCar.IsValid() && ParkedCar->bEngineOn)
		{
			// Petrol = full speed. Ctrl = ECO mode (silent, battery-only). No petrol = eco pace anyway.
			const bool bEco = bWantsToWalk || ParkedCar->Petrol <= 0.f;
			DriveSpeed = bEco ? CarSpeed * 0.45f : CarSpeed;
		}
		AttributeSet->SetMoveSpeed(bInCar ? DriveSpeed : FootSpeed);
	}
}

void ATdfKiller_Lucki::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CarMesh)
	{
		CarMesh->SetVisibility(bInCar);
	}

	if (!HasAuthority() || bIsDown || bIsDead)
	{
		return;
	}

	// Hunger drains on foot only — sitting in the Prius costs nothing. Feeding on a downed runner refills.
	float HungerGain = bInCar ? 0.f : -HungerDrainPerSecond * DeltaSeconds;
	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		const ATdfRunnerCharacter* Runner = *It;
		if (Runner && Runner->bIsDown && !Runner->IsDead() && !Runner->HasEscaped()
			&& FVector::Dist(Runner->GetActorLocation(), GetActorLocation()) < 250.f)
		{
			HungerGain = 12.f * DeltaSeconds; // om nom nom
			break;
		}
	}
	const float OldHunger = Hunger;
	Hunger = FMath::Clamp(Hunger + HungerGain, 0.f, 100.f);
	if ((OldHunger >= StarvingThreshold) != (Hunger >= StarvingThreshold) || bInCar)
	{
		ApplyCarState();
	}
	if (Hunger < StarvingThreshold && OldHunger >= StarvingThreshold && GEngine && IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange, TEXT("STARVING - feed on a downed runner!"));
	}

	// Ram: driving fast into runners flattens them (once the hunt is on).
	if (bInCar && GetVelocity().Size2D() > RamSpeedThreshold && ATdfGameState::KillersUnleashed(GetWorld()))
	{
		for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
		{
			ATdfRunnerCharacter* Runner = *It;
			if (!Runner || Runner->bIsDown || Runner->IsDead() || Runner->HasEscaped())
			{
				continue;
			}
			if (FVector::Dist(Runner->GetActorLocation(), GetActorLocation()) <= RamRange)
			{
				if (Runner->bDiesInstantly)
				{
					Runner->Die();
				}
				else if (UTdfAttributeSet* Attr = Runner->GetTdfAttributeSet())
				{
					Attr->SetHealth(0.f);
					Runner->OnHealthDepleted();
				}
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("RUN OVER BY THE PRIUS"));
				}
			}
		}
	}
}

// --- Tung Tung (Combatant) ---

ATdfKiller_TungTung::ATdfKiller_TungTung()
{
	PrimaryActorTick.bCanEverTick = true;

	KillerType = ETdfKillerType::Combatant_TungTung;
	Stats.Stamina = 150.f;
	Stats.Speed = 120.f;
	Stats.Kealth = 200.f;
	Stats.Damage = 60.f;

	DefaultAbilities[1] = UTdfAbility_GroundSmash::StaticClass();    // Q
	DefaultAbilities[3] = UTdfAbility_RootOfFlesh::StaticClass();    // R (unlocks after 3 min)
	DefaultAbilities[4] = UTdfAbility_TreeForm::StaticClass();       // T (E is interact now)
	DefaultAbilities[5] = UTdfAbility_BlowDart::StaticClass();       // RMB

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));

	TreeTrunkMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TreeTrunkMesh"));
	TreeTrunkMesh->SetupAttachment(GetCapsuleComponent());
	TreeTrunkMesh->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	TreeTrunkMesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 4.2f));
	TreeTrunkMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TreeTrunkMesh->SetVisibility(false);

	TreeCanopyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TreeCanopyMesh"));
	TreeCanopyMesh->SetupAttachment(GetCapsuleComponent());
	TreeCanopyMesh->SetRelativeLocation(FVector(0.f, 0.f, 330.f));
	TreeCanopyMesh->SetRelativeScale3D(FVector(3.4f, 3.4f, 3.2f));
	TreeCanopyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TreeCanopyMesh->SetVisibility(false);

	RedLeafMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RedLeafMesh"));
	RedLeafMesh->SetupAttachment(TreeCanopyMesh);
	RedLeafMesh->SetRelativeLocation(FVector(0.f, 30.f, 55.f));
	RedLeafMesh->SetRelativeScale3D(FVector(0.09f, 0.03f, 0.09f));
	RedLeafMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RedLeafMesh->SetVisibility(false);

	if (CylinderAsset.Succeeded())
	{
		TreeTrunkMesh->SetStaticMesh(CylinderAsset.Object);
		TreeCanopyMesh->SetStaticMesh(CylinderAsset.Object);
	}
	if (CubeAsset.Succeeded())
	{
		RedLeafMesh->SetStaticMesh(CubeAsset.Object);
	}
}

void ATdfKiller_TungTung::BeginPlay()
{
	Super::BeginPlay();

	// Tint the disguise: brown trunk, green canopy... and the one red leaf.
	if (UMaterialInterface* Base = Cast<UMaterialInterface>(
		StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"))))
	{
		auto Tint = [&](UStaticMeshComponent* Comp, const FLinearColor& Color)
		{
			if (Comp)
			{
				UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this);
				MID->SetVectorParameterValue(TEXT("Color"), Color);
				Comp->SetMaterial(0, MID);
			}
		};
		Tint(TreeTrunkMesh, FLinearColor(0.28f, 0.18f, 0.10f));
		Tint(TreeCanopyMesh, FLinearColor(0.10f, 0.30f, 0.12f));
		Tint(RedLeafMesh, FLinearColor(0.95f, 0.05f, 0.05f));
	}
}

void ATdfKiller_TungTung::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATdfKiller_TungTung, bRootOfFleshActive);
	DOREPLIFETIME(ATdfKiller_TungTung, bTreeForm);
}

void ATdfKiller_TungTung::SetTreeForm(bool bNewTreeForm)
{
	if (!HasAuthority() || bTreeForm == bNewTreeForm)
	{
		return;
	}
	bTreeForm = bNewTreeForm;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		if (bTreeForm)
		{
			Move->DisableMovement(); // trees do not jog
		}
		else if (!bIsDown && !bIsStunned)
		{
			Move->SetMovementMode(MOVE_Walking);
		}
	}
}

void ATdfKiller_TungTung::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Disguise visibility runs on every machine off the replicated flag.
	if (TreeTrunkMesh) { TreeTrunkMesh->SetVisibility(bTreeForm); }
	if (TreeCanopyMesh) { TreeCanopyMesh->SetVisibility(bTreeForm); }
	if (RedLeafMesh) { RedLeafMesh->SetVisibility(bTreeForm); }
	if (!IsLocallyControlled())
	{
		SetBodyVisibleLocal(!bTreeForm);
	}

	// Getting stunned or downed makes him drop whoever's on his shoulder (and blows the disguise).
	if (HasAuthority())
	{
		if (CarriedRunner.IsValid() && (bIsStunned || bIsDown || bIsDead))
		{
			DropCarried();
		}
		if (bTreeForm && (bIsStunned || bIsDown || bIsDead))
		{
			SetTreeForm(false);
		}
	}
}

void ATdfKiller_TungTung::TryCarryOrDeliver()
{
	if (!HasAuthority() || bIsDown || bIsDead)
	{
		return;
	}

	if (ATdfRunnerCharacter* Carried = CarriedRunner.Get())
	{
		for (TActorIterator<ATdfShrine> It(GetWorld()); It; ++It)
		{
			ATdfShrine* Shrine = *It;
			if (Shrine && FVector::Dist(Shrine->GetActorLocation(), GetActorLocation()) <= Shrine->DeliverRadius)
			{
				TieToShrine(Carried, Shrine);
				return;
			}
		}
		DropCarried();
		return;
	}

	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		ATdfRunnerCharacter* Runner = *It;
		if (Runner && !Runner->IsDead() && !Runner->HasEscaped() && !Runner->bTiedToShrine && !Runner->bBeingCarried
			&& (Runner->bIsStunned || Runner->bIsDown)
			&& FVector::Dist(Runner->GetActorLocation(), GetActorLocation()) <= 260.f)
		{
			PickUp(Runner);
			return;
		}
	}
}

void ATdfKiller_TungTung::PickUp(ATdfRunnerCharacter* Runner)
{
	SetTreeForm(false);
	CarriedRunner = Runner;
	Runner->bBeingCarried = true;
	Runner->SetActorEnableCollision(false);
	if (UCharacterMovementComponent* Move = Runner->GetCharacterMovement())
	{
		Move->DisableMovement();
	}
	Runner->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Runner->SetActorRelativeLocation(FVector(-40.f, -70.f, 40.f));

	if (AttributeSet)
	{
		AttributeSet->SetMoveSpeed(Stats.Speed * CarrySpeedFraction);
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Purple, TEXT("Tung Tung has taken someone..."));
	}
}

void ATdfKiller_TungTung::DropCarried()
{
	ATdfRunnerCharacter* Runner = CarriedRunner.Get();
	CarriedRunner = nullptr;

	if (AttributeSet && !bRootOfFleshActive)
	{
		AttributeSet->SetMoveSpeed(Stats.Speed);
	}
	if (!Runner)
	{
		return;
	}
	Runner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Runner->SetActorEnableCollision(true);
	Runner->bBeingCarried = false;
	if (!Runner->bIsDown && !Runner->bIsStunned)
	{
		if (UCharacterMovementComponent* Move = Runner->GetCharacterMovement())
		{
			Move->SetMovementMode(MOVE_Walking);
		}
	}
}

void ATdfKiller_TungTung::TieToShrine(ATdfRunnerCharacter* Runner, ATdfShrine* Shrine)
{
	CarriedRunner = nullptr;
	if (AttributeSet && !bRootOfFleshActive)
	{
		AttributeSet->SetMoveSpeed(Stats.Speed);
	}

	Runner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Runner->SetActorEnableCollision(true);
	Runner->bBeingCarried = false;
	Runner->bTiedToShrine = true;
	Runner->bIsDown = true;
	if (UCharacterMovementComponent* Move = Runner->GetCharacterMovement())
	{
		Move->DisableMovement();
	}

	int32 TiedIndex = 0;
	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		if (*It && (*It)->bTiedToShrine && *It != Runner)
		{
			++TiedIndex;
		}
	}
	const float Angle = FMath::DegreesToRadians(TiedIndex * 60.f);
	Runner->SetActorLocation(Shrine->GetActorLocation()
		+ FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * 260.f + FVector(0.f, 0.f, 120.f));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple, TEXT("A runner has been TIED TO THE SHRINE - free them!"));
	}
	if (ATdfGameMode* GameMode = GetWorld()->GetAuthGameMode<ATdfGameMode>())
	{
		GameMode->CheckWinConditions();
	}
}

void ATdfKiller_TungTung::StartRootOfFlesh(float Duration)
{
	if (!HasAuthority() || bRootOfFleshActive)
	{
		return;
	}
	SetTreeForm(false);
	bRootOfFleshActive = true;

	if (AttributeSet)
	{
		AttributeSet->SetMoveSpeed(Stats.Speed * RootSpeedMultiplier);
	}
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->JumpZVelocity = 1200.f;
	}
	GetWorldTimerManager().SetTimer(RootTimerHandle, this, &ATdfKiller_TungTung::EndRootOfFlesh, Duration, false);
}

void ATdfKiller_TungTung::EndRootOfFlesh()
{
	bRootOfFleshActive = false;
	if (AttributeSet)
	{
		AttributeSet->SetMoveSpeed(Stats.Speed);
	}
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->JumpZVelocity = 420.f;
	}
}
