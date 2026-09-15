// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "TdfTypes.h"
#include "TdfCharacterBase.generated.h"

class UCameraComponent;
class UStaticMeshComponent;
class UTdfAbilitySystemComponent;
class UTdfAttributeSet;
class UTdfGameplayAbility;
struct FOnAttributeChangeData;

/**
 * Shared base for runners and killers. Owns the design->UE speed conversion, the
 * stamina-driven sprint/jump/breath systems, and the Gameplay Ability System wiring.
 */
UCLASS(Abstract)
class THEDHEERFATHER_API ATdfCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATdfCharacterBase();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// --- HUD accessors ---
	UFUNCTION(BlueprintPure, Category = "Tdf|HUD") float GetHealth() const;
	UFUNCTION(BlueprintPure, Category = "Tdf|HUD") float GetMaxHealth() const;
	UFUNCTION(BlueprintPure, Category = "Tdf|HUD") float GetStamina() const;
	UFUNCTION(BlueprintPure, Category = "Tdf|HUD") float GetMaxStamina() const;
	UFUNCTION(BlueprintPure, Category = "Tdf|HUD") float GetSpeedMultiplier() const { return LastSpeedMultiplier; }
	UFUNCTION(BlueprintPure, Category = "Tdf|HUD") bool IsWinded() const { return bSprintExhausted; }
	UFUNCTION(BlueprintPure, Category = "Tdf|HUD") bool IsHoldingBreath() const { return bHoldingBreath && bAllowHoldBreath; }
	UFUNCTION(BlueprintPure, Category = "Tdf|HUD") bool IsAsleep() const { return bIsAsleep; }
	UFUNCTION(BlueprintPure, Category = "Tdf|HUD") bool IsDownState() const { return bIsDown; }
	UFUNCTION(BlueprintPure, Category = "Tdf|HUD") float GetSleepTimeRemaining() const { return SleepTimeRemaining; }
	UFUNCTION(BlueprintPure, Category = "Tdf|HUD") int32 GetSuperRunCharges() const { return SuperRunCharges; }
	UFUNCTION(BlueprintPure, Category = "Tdf|HUD") bool IsSuperRunActive() const { return bSuperRunActive; }

	/** Top bar shows Health for runners, Kealth for killers (overridden). */
	virtual float GetHealthBarValue() const;
	virtual float GetHealthBarMax() const;
	virtual FString GetHealthBarLabel() const;

	/** Sets the design-scale WALK speed; Tick applies it with sprint/uphill multipliers. */
	UFUNCTION(BlueprintCallable, Category = "Tdf|Movement")
	void ApplyMoveSpeed(float NewDesignSpeed);

	/** Called when Health hits 0 (runner down / dummy death). Override for behaviour. */
	virtual void OnHealthDepleted();

	/** Called when Kealth hits 0 (killer knockout). Override for behaviour. */
	virtual void OnKealthDepleted();

	/** Stand back up from a knockout: clears bIsDown and restores movement. */
	virtual void Recover();

	/** Stun: disables movement for Duration, then auto-removes. Override for visuals. */
	virtual void ApplyStun(float Duration);
	virtual void RemoveStun();

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|State")
	bool bIsDown = false;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|State")
	bool bIsStunned = false;

	/** Seconds before a knocked-out character auto-recovers (the "briefly knocked out" window). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|State")
	float KnockoutDuration = 5.f;

	/** Permanently eliminated (finisher on a downed runner, suffocation, Casual killer death). */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|State")
	bool bIsDead = false;

	/** Permanent elimination. Server-side. */
	virtual void Die();

	UFUNCTION(BlueprintPure, Category = "Tdf|HUD")
	bool IsDead() const { return bIsDead; }

	/** True while the interact key (F) is held — drives generator repair server-side. */
	bool IsInteracting() const { return bWantsToInteract; }

	/** Attribute set access for abilities/auras (heal, stink, etc.). */
	UTdfAttributeSet* GetTdfAttributeSet() const { return AttributeSet; }

	/** LOCAL-ONLY visibility toggle (stealth / Skinny Bear stillness vision). Not replicated. */
	void SetBodyVisibleLocal(bool bVisible);

	/** HUD: read-only view of the kit slots (E/Q/LMB/R/T/RMB). */
	const TArray<TSubclassOf<class UTdfGameplayAbility>>& GetKitSlots() const { return DefaultAbilities; }

	/** Sitting / hiding (E on a seat or wardrobe). */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|State")
	bool bSeated = false;

	void SitOn(class ATdfSeat* Seat);
	void StandUp();
	TWeakObjectPtr<class ATdfSeat> SeatedOn;

	/** What the camera is pointing at within Range (for E prompts + interactions). */
	AActor* GetLookAtActor(float Range) const;

	/** Reads the current match difficulty from the game mode (defaults to Normal). */
	ETdfDifficulty GetMatchDifficulty() const;

protected:
	virtual void BeginPlay() override;

	FTimerHandle RecoveryTimerHandle;
	FTimerHandle StunTimerHandle;

	/** Seeds GAS attribute values; runners/killers override to pull from their Stats struct. */
	virtual void InitializeAttributes();

	/** Server-side: grants the abilities listed in DefaultAbilities. */
	void GrantDefaultAbilities();

	void OnAbility1Pressed();
	virtual void OnAbility2Pressed();
	virtual void OnAbility2Released() {}
	void OnAbility3Pressed();
	void OnAbility4Pressed();
	void OnAbility5Pressed();

	/** Right mouse button. Default: activate DefaultAbilities[5]. Lucki overrides (DJ whistle). */
	virtual void OnSecondaryAction();

	/** Right mouse released (Musa's voice notes record while held). */
	virtual void OnSecondaryReleased() {}

	/** Input wrappers: track RMB held state (drives survivor PEEKING) then dispatch the virtuals. */
	void OnSecondaryPressedInternal();
	void OnSecondaryReleasedInternal();

	/** PEEKING: RMB held + A/D leans the camera around corners (survivors). */
	bool bSecondaryHeld = false;
	float PeekInput = 0.f;
	float CurrentPeek = 0.f;
	FVector CameraBaseRelLoc = FVector::ZeroVector;

	/** Peeking costs stamina per lean: 5 still, 20 moving, 30 sprinting. */
	bool bPeekCostCharged = false;
	UFUNCTION(Server, Reliable) void ServerSpendStamina(float Amount);
	void SpendStaminaLocal(float Amount);

	/** While climbing, actor yaw is decoupled so you can look around (±135° on the trunk). */
	bool bClimbLookDecoupled = false;

	/** Tree climbing (everyone but Troos). Hold Jump against a trunk to shimmy up. */
	virtual bool CanClimbTrees() const { return true; }
	void OnJumpReleased();
	bool IsFacingClimbableTree() const;
	UFUNCTION(Server, Reliable) void ServerSetClimb(bool bNewValue);

	/** C key. Base does nothing; Lucki overrides to hop in/out of the Prius. */
	virtual void ToggleVehicle() {}

	/** Subclasses can veto sprinting (e.g. while driving). */
	virtual bool CanSprint() const { return true; }

	/** Caches the MoveSpeed attribute value (Tick applies it). */
	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);

	/** WASD movement, relative to where the camera is looking. */
	void MoveForward(float Value);
	void MoveRight(float Value);

	/** Hold-Shift sprint, hold-Ctrl walk. (Virtual: Lucki's Shift exits the Prius.) */
	virtual void OnSprintPressed();
	void OnSprintReleased();
	void OnWalkPressed();
	void OnWalkReleased();

	/** Hold-E breath hold (survivors only). */
	void OnHoldBreathPressed();
	void OnHoldBreathReleased();

	/** Replicate movement intent to the server so it computes the same speed (no rubber-banding). */
	UFUNCTION(Server, Reliable) void ServerSetSprint(bool bNewValue);
	UFUNCTION(Server, Reliable) void ServerSetWalk(bool bNewValue);
	UFUNCTION(Server, Reliable) void ServerSetHoldBreath(bool bNewValue);
	UFUNCTION(Server, Reliable) void ServerSetInteract(bool bNewValue);

	/** Interact (F): hold near a generator to repair; press near a downed teammate to revive. */
	void OnInteractPressed();
	void OnInteractReleased();

	/** Server: route the interact key — Tung Tung carries/delivers, runners revive. */
	void HandleServerInteract();

	/** Server: look for a downed teammate in range and start the 3s revive. */
	void TryReviveNearby();
	void FinishRevive();
	void CancelRevive();

	bool bWantsToInteract = false;
	FTimerHandle ReviveTimerHandle;
	TWeakObjectPtr<ATdfCharacterBase> ReviveTarget;

	/** Seconds a revive takes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|State")
	float ReviveDuration = 3.f;

	/** Range for reviving and for staying latched to a revive. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|State")
	float ReviveRange = 250.f;

	/** Jump that costs stamina (1/3 of max); blocked when too tired. (Lucki: headlights in the Prius.) */
	virtual void OnJumpPressed();

	/** Per-frame stamina drain/regen, sprint gradient, uphill, breath. */
	void UpdateStaminaAndMovement(float DeltaSeconds);

	/** Suffocation damage tick when holding breath at empty stamina. */
	void ApplyBreathDamage();

	/** First suffocation death rolls 80% die / 20% pass out; these handle the sleep state. */
	void EnterSleep();
	void WakeUp();
	void UpdateSleep(float DeltaSeconds);

	/** Slow->fast->slow rate scalar (0..1) for a given stamina fraction. */
	float StaminaRateCurve(float Fraction) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tdf|Abilities")
	UTdfAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	UTdfAttributeSet* AttributeSet;

	/** Abilities granted on possession. Slot order = E / Q / LMB / R. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Abilities")
	TArray<TSubclassOf<UTdfGameplayAbility>> DefaultAbilities;

	/** First-person camera at eye height. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tdf|Camera")
	UCameraComponent* FirstPersonCamera;

	/** Placeholder visible body: hidden from the owning first-person view, seen by other players. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tdf|Visual")
	UStaticMeshComponent* BodyMesh;

	/** Design-scale WALK speed (100 = jog). Comes from each character's Stats. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Movement")
	float MoveSpeed = 60.f;

	/** design speed -> cm/s. 6.0 means 100 (jog) = 600 cm/s. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Movement")
	float SpeedToCmPerSec = 6.f;

	// --- Stamina / sprint tuning ---

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	bool bWantsToSprint = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	bool bWantsToWalk = false;

	/** True while holding Jump against a tree trunk. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	bool bWantsToClimb = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	float ClimbSpeed = 280.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	float ClimbStaminaPerSecond = 4.f;

	/** Stamina per second while hanging onto the trunk (Ctrl while climbing). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	float HangStaminaPerSecond = 0.8f;

	/** Set when stamina empties mid-sprint; blocks sprint until recovered ("winded"). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	bool bSprintExhausted = false;

	// --- SUPER RUN: keep the bar full for a whole minute -> bank one double-length sprint ---

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	int32 SuperRunCharges = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	bool bSuperRunActive = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	float SuperRunEarnSeconds = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	int32 SuperRunMaxCharges = 3;

	float FullStaminaTime = 0.f;

	/** True while holding breath (survivors). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	bool bHoldingBreath = false;

	/** Whether this character can hold breath (runners = true). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	bool bAllowHoldBreath = false;

	/** Locked to full sprint speed because this sprint began from a full bar. */
	bool bFullSprintLock = false;

	/** Was sprinting last frame (used to detect sprint start). */
	bool bWasSprinting = false;

	/** Last computed speed multiplier (for the HUD readout). */
	float LastSpeedMultiplier = 1.f;

	/** Speed multiplier at full sprint (gradient tops out here). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	float SprintFullMultiplier = 3.0f;

	/** Speed multiplier at the bottom of the sprint gradient. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	float SprintPartialMultiplier = 1.5f;

	/** Top of the sprint GRADIENT (used when you did NOT start from a full bar). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	float SprintGradientMax = 2.5f;

	/** Begin a sprint at/above this fraction to lock full 3x for the whole run. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina", meta = (ClampMin = "0", ClampMax = "1"))
	float FullStartThreshold = 0.95f;

	/** Base stamina spent per second while sprinting (scaled by the curve). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	float SprintDrainPerSecond = 25.f;

	/** Base stamina regained per second while idle (scaled by the curve). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	float StaminaRegenPerSecond = 18.f;

	/** Minimum rate scalar so the curve never fully stalls at the extremes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina", meta = (ClampMin = "0", ClampMax = "1"))
	float CurveFloor = 0.2f;

	/** While winded, walk at this multiplier until stamina recovers to SprintRecoverFraction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	float WindedWalkMultiplier = 0.7f;

	/** Winded clears (and 0.7x penalty lifts) once stamina recovers to this fraction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina", meta = (ClampMin = "0", ClampMax = "1"))
	float SprintRecoverFraction = 0.45f;

	/** Slow-walk speed multiplier (hold Ctrl). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	float WalkSpeedMultiplier = 0.4f;

	/** Jump costs this fraction of MAX stamina. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina", meta = (ClampMin = "0", ClampMax = "1"))
	float JumpStaminaCostFraction = 0.3334f;

	/** Extra sprint drain when going uphill (scaled by slope 0..1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	float UphillDrainBonus = 1.0f;

	/** Speed lost when going uphill (fraction at max slope). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina", meta = (ClampMin = "0", ClampMax = "1"))
	float UphillSpeedPenalty = 0.4f;

	// --- Hold breath ---

	/** Stamina lost per second while holding breath (and not sprinting). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	float HoldBreathDrainPerSecond = 6.f;

	/** Sprint drain is multiplied by this while holding breath (sprint capped to 1.5x). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	float HoldBreathSprintDrainMultiplier = 5.f;

	/** Health lost per suffocation tick (every 2s once stamina has been empty 2s). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|Stamina")
	float BreathDamagePerTick = 10.f;

	/** Seconds stamina has been empty while holding breath. */
	float BreathZeroTime = 0.f;

	/** Next BreathZeroTime threshold at which to apply suffocation damage. */
	float NextBreathDamageAt = 2.f;

	// --- Suffocation death / sleep ---

	/** Set after the first suffocation death (the 80/20 roll only happens once). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|State")
	bool bHasBreathDiedBefore = false;

	/** True while knocked out asleep (regenerates health, but can still be killed). */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|State")
	bool bIsAsleep = false;

	float SleepTimeRemaining = 0.f;

	/** Chance the first suffocation death is fatal (otherwise you pass out). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|State", meta = (ClampMin = "0", ClampMax = "1"))
	float BreathDeathChance = 0.8f;

	/** Seconds you stay asleep before waking. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|State")
	float SleepDuration = 30.f;

	/** Health regained per second while asleep. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tdf|State")
	float SleepHealthRegenPerSecond = 3.f;
};
