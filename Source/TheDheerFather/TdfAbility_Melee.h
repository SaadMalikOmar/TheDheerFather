#pragma once

#include "CoreMinimal.h"
#include "TdfGameplayAbility.h"
#include "TdfAbility_Melee.generated.h"

class UGameplayEffect;

/**
 * Melee swing: line-trace forward, apply damage on the SERVER.
 * Rules (see design docs):
 *  - Killer hitting a runner: uses the killer's Stats.Damage. A downed runner is finished (killed).
 *  - Runner hitting the killer: damages Kealth — except Hardcore, where killers can't be hurt.
 *  - Anything else (dummy): flat Health damage.
 */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_Melee : public UTdfGameplayAbility
{
	GENERATED_BODY()

public:
	UTdfAbility_Melee();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	/** Runner weapon damage (weapons will vary this later). */
	UPROPERTY(EditDefaultsOnly, Category = "Tdf")
	float RunnerWeaponDamage = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf")
	TSubclassOf<UGameplayEffect> HealthDamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf")
	TSubclassOf<UGameplayEffect> KealthDamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf")
	float TraceRange = 250.f;

	/** Seconds between swings. */
	UPROPERTY(EditDefaultsOnly, Category = "Tdf")
	float SwingCooldown = 1.f;

private:
	float CooldownEndTime = 0.f;

protected:

	void ApplyDamageTo(class UAbilitySystemComponent* SourceASC, class ATdfCharacterBase* Target,
		TSubclassOf<UGameplayEffect> Effect, float Damage) const;
};
