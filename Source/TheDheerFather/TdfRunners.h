// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "TdfRunnerCharacter.h"
#include "TdfRunners.generated.h"

/**
 * The six base runners (see runners.md). Stats + kits are set in the constructors;
 * Blueprint subclasses can layer character models/anims on top later.
 */

/**
 * Scout — Dheer. HOLD Q to check his Samsung smart watch: head locks down to his wrist,
 * shows killer heartbeat distance while the battery (20s total) lasts — and it senses
 * generators forever, even after the battery dies. Tasks take 3x longer; massive coward.
 */
UCLASS()
class THEDHEERFATHER_API ATdfRunner_Dheer : public ATdfRunnerCharacter
{
	GENERATED_BODY()
public:
	ATdfRunner_Dheer();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnAbility2Pressed() override;
	virtual void OnAbility2Released() override;

	/** True while he's looking at the watch (local: drives beacons, HUD and the head-lock). */
	bool bWatchHeld = false;

	/** Seconds of heartbeat-sensor battery left (generator sense works even at 0). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Dheer")
	float WatchBattery = 20.f;

	/** Kinetic charging: recharges EXTREMELY slowly, and only while he's running. */
	UPROPERTY(EditDefaultsOnly, Category = "Tdf|Dheer")
	float WatchRechargePerSecond = 0.12f;

private:
	/** Where he was looking before the watch pulled his eyes down (restored on release). */
	FRotator PreWatchControlRotation = FRotator::ZeroRotator;
	bool bHasPreWatchRotation = false;
};

/** Tank — Troos. 200 HP, immune to Lucki traps; slow, tires fast, and CANNOT climb trees. */
UCLASS()
class THEDHEERFATHER_API ATdfRunner_Troos : public ATdfRunnerCharacter
{
	GENERATED_BODY()
public:
	ATdfRunner_Troos();

	virtual bool CanClimbTrees() const override { return false; }
};

/** Engineer — Julius. Repairs 3x faster, fearless (PUD 0); poor vision. */
UCLASS()
class THEDHEERFATHER_API ATdfRunner_Julius : public ATdfRunnerCharacter
{
	GENERATED_BODY()
public:
	ATdfRunner_Julius();
};

/**
 * Trickster — Musa. Q places decoys; HOLD RMB records a voice note, release plants an
 * invisible speaker (3 max) that plays it back when the killer walks past. Freezes
 * every 30s to doomscroll IG reels.
 */
UCLASS()
class THEDHEERFATHER_API ATdfRunner_Musa : public ATdfRunnerCharacter
{
	GENERATED_BODY()
public:
	ATdfRunner_Musa();

	virtual void Tick(float DeltaSeconds) override;

	/** Hold RMB = record; release = plant the speaker here. */
	virtual void OnSecondaryAction() override;
	virtual void OnSecondaryReleased() override;

	UFUNCTION(Server, Reliable)
	void ServerPlaceSpeaker(float RecordedSeconds);

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Musa")
	int32 SpeakersLeft = 3;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Seconds between doomscroll freezes / how long each freeze lasts. */
	UPROPERTY(EditDefaultsOnly, Category = "Tdf|Musa") float PhoneInterval = 30.f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf|Musa") float PhoneFreezeDuration = 5.f;

private:
	float PhoneTimer = 0.f;
	bool bRecording = false;
	float RecordStartTime = 0.f;
};

/** Stealth — Shaun. Fastest runner; invisible while still + holding breath; dies in one hit. */
UCLASS()
class THEDHEERFATHER_API ATdfRunner_Shaun : public ATdfRunnerCharacter
{
	GENERATED_BODY()
public:
	ATdfRunner_Shaun();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Shaun")
	bool bStealthed = false;
};

/** Medic — Emma. White emo raver, walking pharmacy: heals + calms PUD with her stash. */
UCLASS()
class THEDHEERFATHER_API ATdfRunner_Emma : public ATdfRunnerCharacter
{
	GENERATED_BODY()
public:
	ATdfRunner_Emma();
};
