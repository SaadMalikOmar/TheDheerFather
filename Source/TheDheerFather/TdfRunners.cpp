// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#include "TdfRunners.h"
#include "TdfAttributeSet.h"
#include "TdfCharacterAbilities.h"
#include "TdfKillers.h"
#include "TdfDeployables.h"
#include "Camera/PlayerCameraManager.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"

// --- Dheer (Scout) ---

ATdfRunner_Dheer::ATdfRunner_Dheer()
{
	PrimaryActorTick.bCanEverTick = true;
	RunnerRole = ETdfRunnerRole::Scout;
	Stats.Health = 100.f; Stats.Stamina = 100.f; Stats.Speed = 60.f; Stats.PudLevel = 90.f;
	// Q = the watch (hold), handled below — no GAS ability in the slot.
}

void ATdfRunner_Dheer::OnAbility2Pressed()
{
	if (!IsLocallyControlled())
	{
		return;
	}
	bWatchHeld = true;

	// Remember exactly where he was looking, then lock his head down onto the wrist.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PreWatchControlRotation = PC->GetControlRotation();
		bHasPreWatchRotation = true;
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->ViewPitchMin = -52.f;
			PC->PlayerCameraManager->ViewPitchMax = -34.f;
		}
	}
}

void ATdfRunner_Dheer::OnAbility2Released()
{
	if (!IsLocallyControlled())
	{
		return;
	}
	bWatchHeld = false;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->ViewPitchMin = -89.9f;
			PC->PlayerCameraManager->ViewPitchMax = 89.9f;
		}
		// Eyes snap back to exactly where they were before the watch check.
		if (bHasPreWatchRotation)
		{
			PC->SetControlRotation(PreWatchControlRotation);
			bHasPreWatchRotation = false;
		}
	}
}

void ATdfRunner_Dheer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsLocallyControlled())
	{
		return;
	}

	// Battery drains only while he's looking at it...
	if (bWatchHeld && WatchBattery > 0.f)
	{
		WatchBattery = FMath::Max(0.f, WatchBattery - DeltaSeconds);
	}
	// ...and trickle-charges kinetically — ONLY while actually running (~3 min for a full bar).
	else if (!bWatchHeld && WatchBattery < 20.f
		&& bWantsToSprint && GetVelocity().Size2D() > 400.f)
	{
		WatchBattery = FMath::Min(20.f, WatchBattery + WatchRechargePerSecond * DeltaSeconds);
	}
}

// --- Troos (Tank) ---

ATdfRunner_Troos::ATdfRunner_Troos()
{
	RunnerRole = ETdfRunnerRole::Tank;
	Stats.Health = 200.f; Stats.Stamina = 30.f; Stats.Speed = 55.f; Stats.PudLevel = 30.f;
	// No active ability — his kit is being a fridge. Trap immunity lives in ATdfNaniTrap.
}

// --- Julius (Engineer) ---

ATdfRunner_Julius::ATdfRunner_Julius()
{
	RunnerRole = ETdfRunnerRole::Engineer;
	Stats.Health = 75.f; Stats.Stamina = 75.f; Stats.Speed = 70.f; Stats.PudLevel = 0.f;
	// 3x repair speed lives in ATdfGenerator. Poor vision: permanently narrowed view.
	BaseVignette = 0.7f;
}

// --- Musa (Trickster) ---

ATdfRunner_Musa::ATdfRunner_Musa()
{
	RunnerRole = ETdfRunnerRole::Trickster;
	Stats.Health = 100.f; Stats.Stamina = 50.f; Stats.Speed = 50.f; Stats.PudLevel = 100.f;
	DefaultAbilities[1] = UTdfAbility_PlaceDecoy::StaticClass(); // Q: decoy body-double
	// RMB is handled directly (hold to record, release to plant).
}

void ATdfRunner_Musa::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATdfRunner_Musa, SpeakersLeft);
}

void ATdfRunner_Musa::OnSecondaryAction()
{
	if (!IsLocallyControlled() || bRecording)
	{
		return;
	}
	if (SpeakersLeft <= 0)
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Silver, TEXT("Out of speakers")); }
		return;
	}
	bRecording = true;
	RecordStartTime = GetWorld()->GetTimeSeconds();
	if (GEngine)
	{
		// TODO(audio): start actual mic capture here (VOIP) — records the proximity chat around Musa.
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("REC ● recording voice note... release RMB to plant the speaker"));
	}
}

void ATdfRunner_Musa::OnSecondaryReleased()
{
	if (!IsLocallyControlled() || !bRecording)
	{
		return;
	}
	bRecording = false;
	const float Seconds = FMath::Clamp(GetWorld()->GetTimeSeconds() - RecordStartTime, 0.5f, 10.f);
	ServerPlaceSpeaker(Seconds);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
			FString::Printf(TEXT("Voice note saved (%.1fs) - speaker hidden here"), Seconds));
	}
}

void ATdfRunner_Musa::ServerPlaceSpeaker_Implementation(float RecordedSeconds)
{
	if (SpeakersLeft <= 0 || bIsDown || IsDead())
	{
		return;
	}
	--SpeakersLeft;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (ATdfSpeaker* Speaker = GetWorld()->SpawnActor<ATdfSpeaker>(ATdfSpeaker::StaticClass(),
		GetActorLocation(), GetActorRotation(), Params))
	{
		Speaker->RecordedSeconds = RecordedSeconds;
	}
}

void ATdfRunner_Musa::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Phone addiction: every PhoneInterval seconds he MUST scroll reels for PhoneFreezeDuration.
	if (HasAuthority() && !bIsDown && !IsDead() && !bIsStunned && !HasEscaped())
	{
		PhoneTimer += DeltaSeconds;
		if (PhoneTimer >= PhoneInterval)
		{
			PhoneTimer = 0.f;
			ApplyStun(PhoneFreezeDuration);
		}
	}
}

// --- Shaun (Stealth) ---

ATdfRunner_Shaun::ATdfRunner_Shaun()
{
	RunnerRole = ETdfRunnerRole::Stealth;
	Stats.Health = 60.f; Stats.Stamina = 100.f; Stats.Speed = 100.f; Stats.PudLevel = 25.f;
	bDiesInstantly = true; // caught = dead, no down state
}

void ATdfRunner_Shaun::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Server decides stealth: holding breath + standing still.
	if (HasAuthority())
	{
		bStealthed = bHoldingBreath && GetVelocity().Size2D() < 20.f && !bIsDown && !IsDead();
	}

	// Every machine applies it visually — except when the local player is Skinny Bear,
	// whose own stillness-vision logic owns runner visibility on that screen.
	if (!IsLocallyControlled())
	{
		const APlayerController* LocalPC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
		const bool bViewerIsSkinnyBear = LocalPC && Cast<ATdfKiller_SkinnyBear>(LocalPC->GetPawn());
		if (!bViewerIsSkinnyBear)
		{
			SetBodyVisibleLocal(!bStealthed);
		}
	}
}

void ATdfRunner_Shaun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATdfRunner_Shaun, bStealthed);
}

// --- Emma (Medic) ---

ATdfRunner_Emma::ATdfRunner_Emma()
{
	RunnerRole = ETdfRunnerRole::Medic;
	Stats.Health = 75.f; Stats.Stamina = 75.f; Stats.Speed = 70.f; Stats.PudLevel = 58.f;
	DefaultAbilities[1] = UTdfAbility_Heal::StaticClass(); // Q: the rave stash — heals + drops PUD to 0
}
