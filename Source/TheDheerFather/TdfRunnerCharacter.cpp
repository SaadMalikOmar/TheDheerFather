// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#include "TdfRunnerCharacter.h"
#include "TdfAttributeSet.h"
#include "TdfAbility_Melee.h"
#include "TdfDeployables.h"
#include "TdfGameMode.h"
#include "TdfKillerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

ATdfRunnerCharacter::ATdfRunnerCharacter()
{
	// Survivors can hold their breath (E).
	bAllowHoldBreath = true;

	// Kit slots: [1] = Q tactical (per character), [2] = LMB weapon swing, [3] = R special.
	DefaultAbilities.SetNum(6);
	DefaultAbilities[2] = UTdfAbility_Melee::StaticClass();
}

void ATdfRunnerCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = Stats.Health;
	CurrentPud = Stats.PudLevel;
	ApplyMoveSpeed(Stats.Speed);
}

void ATdfRunnerCharacter::InitializeAttributes()
{
	if (AttributeSet)
	{
		AttributeSet->SetMaxHealth(Stats.Health);
		AttributeSet->SetHealth(Stats.Health);
		AttributeSet->SetMaxStamina(Stats.Stamina);
		AttributeSet->SetStamina(Stats.Stamina);
		AttributeSet->SetMoveSpeed(Stats.Speed);
		AttributeSet->SetPud(Stats.PudLevel);
	}
}

void ATdfRunnerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATdfRunnerCharacter, bHasEscaped);
	DOREPLIFETIME(ATdfRunnerCharacter, bHasGeneratorFinder);
	DOREPLIFETIME(ATdfRunnerCharacter, bHasRepairKit);
	DOREPLIFETIME(ATdfRunnerCharacter, bDraggedByDJ);
	DOREPLIFETIME(ATdfRunnerCharacter, CurrentWeapon);
	DOREPLIFETIME(ATdfRunnerCharacter, WeaponDurability);
	DOREPLIFETIME(ATdfRunnerCharacter, bBeingCarried);
	DOREPLIFETIME(ATdfRunnerCharacter, bTiedToShrine);
}

void ATdfRunnerCharacter::Recover()
{
	Super::Recover();

	// If we actually got back up (not dead), we're no longer bound to anything.
	if (!IsDead() && !bIsDown)
	{
		bTiedToShrine = false;
		bBeingCarried = false;
		SetActorEnableCollision(true);
	}
}

void ATdfRunnerCharacter::ConsumeWeaponHit()
{
	if (!HasAuthority())
	{
		return;
	}
	--WeaponDurability;
	if (WeaponDurability <= 0)
	{
		CurrentWeapon = ETdfWeaponType::None;
		WeaponDurability = 0;
		if (GEngine && IsLocallyControlled())
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Your weapon broke!"));
		}
	}
}

void ATdfRunnerCharacter::Die()
{
	const bool bWasDead = IsDead();
	Super::Die();

	// Drop the generator-finder gadget for teammates (the hardcore mechanic).
	if (!bWasDead && HasAuthority() && GetWorld()
		&& (RunnerRole == ETdfRunnerRole::Scout || RunnerRole == ETdfRunnerRole::Engineer || RunnerRole == ETdfRunnerRole::Medic))
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<ATdfFinderDevice>(ATdfFinderDevice::StaticClass(),
			GetActorLocation() + FVector(60.f, 0.f, -60.f), FRotator::ZeroRotator, Params);
	}
}

void ATdfRunnerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SensorTimeRemaining = FMath::Max(0.f, SensorTimeRemaining - DeltaSeconds);

	// Hardcore: panicking runners crash into each other and trip.
	if (HasAuthority())
	{
		TripCooldownRemaining = FMath::Max(0.f, TripCooldownRemaining - DeltaSeconds);
		if (TripCooldownRemaining <= 0.f && !bIsDown && !IsDead() && !bIsStunned
			&& GetMatchDifficulty() == ETdfDifficulty::Hardcore
			&& GetVelocity().Size2D() > MoveSpeed * 6.f * 1.2f)
		{
			for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
			{
				ATdfRunnerCharacter* Other = *It;
				if (Other && Other != this && !Other->bIsDown && !Other->IsDead()
					&& Other->TripCooldownRemaining <= 0.f
					&& FVector::Dist(Other->GetActorLocation(), GetActorLocation()) < 130.f)
				{
					TripCooldownRemaining = 4.f;
					Other->TripCooldownRemaining = 4.f;
					ApplyStun(0.8f);
					Other->ApplyStun(0.8f);
					break;
				}
			}
		}
	}

	// Fear is a local presentation effect — only the player looking through this camera needs it.
	if (IsLocallyControlled())
	{
		UpdateFear(DeltaSeconds);
	}
}

void ATdfRunnerCharacter::UpdateFear(float DeltaSeconds)
{
	// Nearest killer within FearRange.
	float NearestDist = FearRange;
	for (TActorIterator<ATdfKillerCharacter> It(GetWorld()); It; ++It)
	{
		if (const ATdfKillerCharacter* Killer = *It)
		{
			NearestDist = FMath::Min(NearestDist, (float)FVector::Dist(Killer->GetActorLocation(), GetActorLocation()));
		}
	}

	const float Proximity = 1.f - (NearestDist / FearRange);           // 0 far .. 1 touching
	const float Cowardice = AttributeSet ? (AttributeSet->GetPud() / 100.f) : 0.5f;
	const float TargetFear = FMath::Clamp(Proximity, 0.f, 1.f) * Cowardice;

	// Fear rises fast, fades slowly — dread lingers.
	const float InterpSpeed = (TargetFear > FearLevel) ? 3.f : 0.8f;
	FearLevel = FMath::FInterpTo(FearLevel, TargetFear, DeltaSeconds, InterpSpeed);

	// Screen distortion scaled by fear: tunnel-vision vignette + colour fringing.
	if (FirstPersonCamera)
	{
		FPostProcessSettings& PP = FirstPersonCamera->PostProcessSettings;
		PP.bOverride_VignetteIntensity = true;
		PP.VignetteIntensity = BaseVignette + FearLevel * 1.3f;
		PP.bOverride_SceneFringeIntensity = true;
		PP.SceneFringeIntensity = FearLevel * 5.f;
	}
}

void ATdfRunnerCharacter::Escape()
{
	if (!HasAuthority() || bHasEscaped || bIsDead)
	{
		return;
	}
	bHasEscaped = true;

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
	}

	if (GEngine && IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("YOU ESCAPED!"));
	}

	if (ATdfGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATdfGameMode>() : nullptr)
	{
		GameMode->CheckWinConditions();
	}
}
