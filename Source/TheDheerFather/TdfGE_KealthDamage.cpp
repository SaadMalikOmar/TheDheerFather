// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#include "TdfGE_KealthDamage.h"
#include "TdfAttributeSet.h"

UTdfGE_KealthDamage::UTdfGE_KealthDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UTdfAttributeSet::GetKealthAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(Modifier);
}
