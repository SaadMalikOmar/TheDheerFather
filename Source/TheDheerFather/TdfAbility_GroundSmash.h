#pragma once

#include "CoreMinimal.h"
#include "TdfGameplayAbility.h"
#include "TdfAbility_GroundSmash.generated.h"

/**
 * Tung Tung's tactical: slam the ground, stunning every character within Radius.
 * Has a simple cooldown so it can't be spammed (foundation for the Hardcore use-limit system).
 */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_GroundSmash : public UTdfGameplayAbility
{
	GENERATED_BODY()

public:
	UTdfAbility_GroundSmash();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Tdf")
	float Radius = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf")
	float StunDuration = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf")
	float Cooldown = 6.f;

private:
	/** World time at which this ability comes off cooldown. */
	float CooldownEndTime = 0.f;
};
