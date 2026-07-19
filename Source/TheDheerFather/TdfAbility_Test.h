#pragma once

#include "CoreMinimal.h"
#include "TdfGameplayAbility.h"
#include "TdfAbility_Test.generated.h"

/** Template/demo ability: prints to screen on activation to prove the GAS pipeline works. */
UCLASS()
class THEDHEERFATHER_API UTdfAbility_Test : public UTdfGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
