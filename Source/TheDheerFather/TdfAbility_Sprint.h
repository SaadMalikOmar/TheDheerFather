#pragma once

#include "CoreMinimal.h"
#include "TdfGameplayAbility.h"
#include "TdfAbility_Sprint.generated.h"

class UGameplayEffect;

/** Applies a temporary speed-boost gameplay effect to the user. Template for diet-coke / sprint powers. */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_Sprint : public UTdfGameplayAbility
{
	GENERATED_BODY()

public:
	UTdfAbility_Sprint();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	/** The effect applied on activation. Defaults to UTdfGE_SpeedBoost. */
	UPROPERTY(EditDefaultsOnly, Category = "Tdf")
	TSubclassOf<UGameplayEffect> SpeedBoostEffect;
};
