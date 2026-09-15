// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "TdfGameplayAbility.h"
#include "TdfCharacterAbilities.generated.h"

class UGameplayEffect;

/** Dheer (Scout) Q: heartbeat sensor — shows killer distance on the HUD for 20s. 30s cooldown. */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_HeartbeatSensor : public UTdfGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float SensorDuration = 20.f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float Cooldown = 30.f;
private:
	float CooldownEndTime = 0.f;
};

/** Mahnam (Medic) Q: narcotics — heals self + nearby runners and drops their PUD to 0. */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_Heal : public UTdfGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float HealAmount = 40.f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float HealRadius = 400.f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float Cooldown = 15.f;
private:
	float CooldownEndTime = 0.f;
};

/** Musa (Trickster) Q: drops a decoy body-double behind him. */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_PlaceDecoy : public UTdfGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float Cooldown = 15.f;
private:
	float CooldownEndTime = 0.f;
};

/** Lucki T: Diet Coke — speed boost on a recharge (yes, even in Hardcore: he keeps a crate in the boot). */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_DietCoke : public UTdfGameplayAbility
{
	GENERATED_BODY()
public:
	UTdfAbility_DietCoke();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float Cooldown = 45.f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf") TSubclassOf<UGameplayEffect> SpeedBoostEffect;

private:
	float CooldownEndTime = 0.f;
};

/** Lucki R: Spirit of Nani — places a doorbell trap that alerts him when runners pass. 4 uses. */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_PlaceNaniTrap : public UTdfGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf") int32 UsesLeft = 4;
};

/** Skinny Bear R: Echo Scream — reveals every runner for 5s (they can't hide by standing still). */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_EchoScream : public UTdfGameplayAbility
{
	GENERATED_BODY()
public:
	UTdfAbility_EchoScream() { HardcoreUseLimit = 1; } // hardcore: one scream per match

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float RevealDuration = 5.f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float Cooldown = 30.f;
private:
	float CooldownEndTime = 0.f;
};

/** Lucki R (ultimate): release DJ, the rabid dog. One dog at a time. */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_ReleaseDJ : public UTdfGameplayAbility
{
	GENERATED_BODY()
public:
	UTdfAbility_ReleaseDJ() { HardcoreUseLimit = 1; } // hardcore: DJ doesn't come back

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float Cooldown = 45.f;
private:
	float CooldownEndTime = 0.f;
};

/** Tung Tung R (ultimate): Root of Flesh — 10s slender form, huge speed/jumps, one-touch downs. */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_RootOfFlesh : public UTdfGameplayAbility
{
	GENERATED_BODY()
public:
	UTdfAbility_RootOfFlesh() { HardcoreUseLimit = 2; }

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float Duration = 10.f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float Cooldown = 60.f;
private:
	float CooldownEndTime = 0.f;
};

/** Skinny Bear E/RMB: Sniff — halves his speed briefly, but reveals everyone within 50 m through walls. */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_Sniff : public UTdfGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float SniffRadius = 5000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float RevealDuration = 6.f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float SlowDuration = 5.f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float Cooldown = 20.f;
private:
	float CooldownEndTime = 0.f;
};

/** Tung Tung E: Tree Form — disguise as a tree (with one traitorous red leaf). 60s recharge. */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_TreeForm : public UTdfGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float Cooldown = 60.f;
private:
	float CooldownEndTime = 0.f;
};

/** Musa RMB: place an invisible speaker (3 max) that spooks the killer with phantom voices. */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_PlaceSpeaker : public UTdfGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf") int32 UsesLeft = 3;
};

/** Tung Tung Q: Blow Dart — long-range tranquilizer, stuns the runner it hits. */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_BlowDart : public UTdfGameplayAbility
{
	GENERATED_BODY()
public:
	UTdfAbility_BlowDart() { HardcoreUseLimit = 8; }

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float DartRange = 2500.f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float TranqDuration = 4.f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf") float Cooldown = 10.f;
private:
	float CooldownEndTime = 0.f;
};
