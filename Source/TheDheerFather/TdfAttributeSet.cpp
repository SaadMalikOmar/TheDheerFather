#include "TdfAttributeSet.h"
#include "TdfCharacterBase.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UTdfAttributeSet::UTdfAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitStamina(100.f);
	InitMaxStamina(100.f);
	InitMoveSpeed(60.f);
	InitPud(50.f);
	InitKealth(200.f);
	InitMaxKealth(200.f);
}

void UTdfAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetKealthAttribute())
	{
		SetKealth(FMath::Clamp(GetKealth(), 0.f, GetMaxKealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetPudAttribute())
	{
		SetPud(FMath::Clamp(GetPud(), 0.f, 100.f));
	}

	// Depletion hooks: notify the owning character when Health/Kealth bottoms out.
	if (Data.EvaluatedData.Attribute == GetHealthAttribute() && GetHealth() <= 0.f)
	{
		if (ATdfCharacterBase* Character = Cast<ATdfCharacterBase>(GetOwningActor()))
		{
			Character->OnHealthDepleted();
		}
	}
	else if (Data.EvaluatedData.Attribute == GetKealthAttribute() && GetKealth() <= 0.f)
	{
		if (ATdfCharacterBase* Character = Cast<ATdfCharacterBase>(GetOwningActor()))
		{
			Character->OnKealthDepleted();
		}
	}
}

void UTdfAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UTdfAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UTdfAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetKealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxKealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
	else if (Attribute == GetPudAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 100.f);
	}
}

void UTdfAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UTdfAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTdfAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTdfAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTdfAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTdfAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTdfAttributeSet, Pud, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTdfAttributeSet, Kealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTdfAttributeSet, MaxKealth, COND_None, REPNOTIFY_Always);
}

void UTdfAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue) { GAMEPLAYATTRIBUTE_REPNOTIFY(UTdfAttributeSet, Health, OldValue); }
void UTdfAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue) { GAMEPLAYATTRIBUTE_REPNOTIFY(UTdfAttributeSet, MaxHealth, OldValue); }
void UTdfAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue) { GAMEPLAYATTRIBUTE_REPNOTIFY(UTdfAttributeSet, Stamina, OldValue); }
void UTdfAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue) { GAMEPLAYATTRIBUTE_REPNOTIFY(UTdfAttributeSet, MaxStamina, OldValue); }
void UTdfAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue) { GAMEPLAYATTRIBUTE_REPNOTIFY(UTdfAttributeSet, MoveSpeed, OldValue); }
void UTdfAttributeSet::OnRep_Pud(const FGameplayAttributeData& OldValue) { GAMEPLAYATTRIBUTE_REPNOTIFY(UTdfAttributeSet, Pud, OldValue); }
void UTdfAttributeSet::OnRep_Kealth(const FGameplayAttributeData& OldValue) { GAMEPLAYATTRIBUTE_REPNOTIFY(UTdfAttributeSet, Kealth, OldValue); }
void UTdfAttributeSet::OnRep_MaxKealth(const FGameplayAttributeData& OldValue) { GAMEPLAYATTRIBUTE_REPNOTIFY(UTdfAttributeSet, MaxKealth, OldValue); }
