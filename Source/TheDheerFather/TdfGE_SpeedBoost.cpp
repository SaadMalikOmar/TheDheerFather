// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#include "TdfGE_SpeedBoost.h"
#include "TdfAttributeSet.h"

UTdfGE_SpeedBoost::UTdfGE_SpeedBoost()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(5.0f));

	// One stack max: re-applying refreshes the 5s timer instead of stacking +90 repeatedly.
	StackingType = EGameplayEffectStackingType::AggregateByTarget;
	StackLimitCount = 1;
	StackDurationRefreshPolicy = EGameplayEffectStackingDurationPolicy::RefreshOnSuccessfulApplication;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UTdfAttributeSet::GetMoveSpeedAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(90.0f));
	Modifiers.Add(Modifier);
}
