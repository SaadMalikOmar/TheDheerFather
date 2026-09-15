// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "TdfCharacterBase.h"
#include "TdfTypes.h"
#include "TdfRunnerCharacter.generated.h"

/** A survivor. One of each role per match (see difficulties.md). */
UCLASS()
class THEDHEERFATHER_API ATdfRunnerCharacter : public ATdfCharacterBase
{
	GENERATED_BODY()

public:
	ATdfRunnerCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Server: mark escaped, hide the pawn, count toward the runners' win. */
	void Escape();

	UFUNCTION(BlueprintPure, Category = "Tdf|HUD")
	bool HasEscaped() const { return bHasEscaped; }

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Runner")
	bool bHasEscaped = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tdf|Runner")
	ETdfRunnerRole RunnerRole = ETdfRunnerRole::Scout;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tdf|Runner")
	FTdfRunnerStats Stats;

	/** Live cowardice value, seeded from Stats.PudLevel; rises under scare tactics. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Runner")
	float CurrentPud = 0.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Runner")
	float CurrentHealth = 100.f;

	/** 0..1 fear right now (PUD x killer proximity). Drives screen distortion; HUD reads it too. */
	UFUNCTION(BlueprintPure, Category = "Tdf|HUD")
	float GetFearLevel() const { return FearLevel; }

	/** Dheer's heartbeat sensor: while > 0 the HUD shows killer distance. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Runner")
	float SensorTimeRemaining = 0.f;

	/** Shaun: a killer's hit kills outright — no down state. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Runner")
	bool bDiesInstantly = false;

	/** Hardcore: grants generator beacons (picked up from a dead Dheer/Julius/Mahnam's gadget). */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Runner")
	bool bHasGeneratorFinder = false;

	/** Stage 2: generators can only be repaired holding one of these (found in your house). */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Runner")
	bool bHasRepairKit = false;

	/** DJ has you by the leg — you can't move. SPAM E: 1% chance per press to break free. */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Runner")
	bool bDraggedByDJ = false;

	/** Held weapon (picked up in the world; see weapons.md). */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Runner")
	ETdfWeaponType CurrentWeapon = ETdfWeaponType::None;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Runner")
	int32 WeaponDurability = 0;

	/** Server: one durability hit; weapon breaks at 0. */
	void ConsumeWeaponHit();

	/** Hardcore: runners crashing into each other at speed trip and stumble. */
	float TripCooldownRemaining = 0.f;

	/** Dead Dheer/Julius/Mahnam drop their finder gadget where they fell. */
	virtual void Die() override;

	/** Tung Tung's kidnap loop: slung over his shoulder / bound at the shrine. */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Runner")
	bool bBeingCarried = false;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Runner")
	bool bTiedToShrine = false;

	/** Reviving a tied runner frees them from the shrine. */
	virtual void Recover() override;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void InitializeAttributes() override;

	/** PUD fear: proximity to the killer scaled by this character's cowardice. */
	void UpdateFear(float DeltaSeconds);

	/** Distance at which the killer starts affecting you at all. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Fear")
	float FearRange = 2500.f;

	/** Vignette floor — Julius (bad eyes) raises this. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Fear")
	float BaseVignette = 0.4f;

	float FearLevel = 0.f;
};
