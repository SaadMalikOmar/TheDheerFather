#include "TdfAbility_Sprint.h"
#include "TdfGE_SpeedBoost.h"
#include "AbilitySystemComponent.h"
#include "Engine/Engine.h"

UTdfAbility_Sprint::UTdfAbility_Sprint()
{
	SpeedBoostEffect = UTdfGE_SpeedBoost::StaticClass();
}

void UTdfAbility_Sprint::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ActorInfo && SpeedBoostEffect)
	{
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SpeedBoostEffect, GetAbilityLevel(), Context);
			if (SpecHandle.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Sprint! (+speed for 5s)"));
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
