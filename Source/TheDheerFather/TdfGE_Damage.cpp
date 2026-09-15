// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#include "TdfGE_Damage.h"
#include "TdfAttributeSet.h"

UTdfGE_Damage::UTdfGE_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Magnitude supplied per-hit via SetByCaller (Data.Damage) — pass a NEGATIVE value to damage.
	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UTdfAttributeSet::GetHealthAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(Modifier);
}
