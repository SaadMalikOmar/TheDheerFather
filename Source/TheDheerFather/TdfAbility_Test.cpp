// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#include "TdfAbility_Test.h"
#include "Engine/Engine.h"

void UTdfAbility_Test::ActivateAbility(
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

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("GAS ability activated! (template)"));
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
