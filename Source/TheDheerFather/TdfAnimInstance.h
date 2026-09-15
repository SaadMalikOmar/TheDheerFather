// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TdfAnimInstance.generated.h"

class ATdfCharacterBase;

/**
 * Base AnimInstance: caches movement/state values each frame so the Anim Blueprint
 * graph can just read variables (GroundSpeed for a walk/run blendspace, etc.).
 * Set your character's Anim Blueprint parent class to this.
 */
UCLASS()
class THEDHEERFATHER_API UTdfAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Tdf|Anim")
	ATdfCharacterBase* OwningCharacter = nullptr;

	/** Horizontal speed in cm/s — drive a locomotion blendspace with this. */
	UPROPERTY(BlueprintReadOnly, Category = "Tdf|Anim")
	float GroundSpeed = 0.f;

	/** Current speed multiplier (1 walk, up to 3 sprint). */
	UPROPERTY(BlueprintReadOnly, Category = "Tdf|Anim")
	float SpeedMultiplier = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Tdf|Anim")
	bool bIsFalling = false;

	UPROPERTY(BlueprintReadOnly, Category = "Tdf|Anim")
	bool bIsSprinting = false;

	UPROPERTY(BlueprintReadOnly, Category = "Tdf|Anim")
	bool bIsStunned = false;

	UPROPERTY(BlueprintReadOnly, Category = "Tdf|Anim")
	bool bIsDown = false;

	UPROPERTY(BlueprintReadOnly, Category = "Tdf|Anim")
	bool bIsAsleep = false;
};
