// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "TdfAttributeSet.generated.h"

// Standard GAS accessor boilerplate: generates Get/Set/Init + attribute getter for each attribute.
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * All gameplay attributes for the game. Runners use Health/Stamina/Pud; killers use Kealth.
 * Maps to the stat blocks in runners.md / killers.md.
 */
UCLASS()
class THEDHEERFATHER_API UTdfAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UTdfAttributeSet();

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Clamp current-value changes (duration effects, etc.) before they land. */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	/** Clamp base-value changes (instant effects like damage) so Health/Kealth never go negative. */
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UTdfAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UTdfAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Stamina", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UTdfAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "Stamina", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UTdfAttributeSet, MaxStamina)

	/** Design-scale move speed (100 = jog). */
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UTdfAttributeSet, MoveSpeed)

	/** Runner cowardice 0..100. Raised by scare tactics. See design-pillars.md. */
	UPROPERTY(BlueprintReadOnly, Category = "PUD", ReplicatedUsing = OnRep_Pud)
	FGameplayAttributeData Pud;
	ATTRIBUTE_ACCESSORS(UTdfAttributeSet, Pud)

	/** Killer knockout health. */
	UPROPERTY(BlueprintReadOnly, Category = "Kealth", ReplicatedUsing = OnRep_Kealth)
	FGameplayAttributeData Kealth;
	ATTRIBUTE_ACCESSORS(UTdfAttributeSet, Kealth)

	UPROPERTY(BlueprintReadOnly, Category = "Kealth", ReplicatedUsing = OnRep_MaxKealth)
	FGameplayAttributeData MaxKealth;
	ATTRIBUTE_ACCESSORS(UTdfAttributeSet, MaxKealth)

protected:
	UFUNCTION() virtual void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION() virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	UFUNCTION() virtual void OnRep_Stamina(const FGameplayAttributeData& OldValue);
	UFUNCTION() virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);
	UFUNCTION() virtual void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);
	UFUNCTION() virtual void OnRep_Pud(const FGameplayAttributeData& OldValue);
	UFUNCTION() virtual void OnRep_Kealth(const FGameplayAttributeData& OldValue);
	UFUNCTION() virtual void OnRep_MaxKealth(const FGameplayAttributeData& OldValue);

private:
	/** Shared clamp logic used by both Pre*Change hooks. */
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
