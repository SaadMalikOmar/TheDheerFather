// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#include "TdfAnimInstance.h"
#include "TdfCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

void UTdfAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwningCharacter = Cast<ATdfCharacterBase>(TryGetPawnOwner());
}

void UTdfAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter)
	{
		OwningCharacter = Cast<ATdfCharacterBase>(TryGetPawnOwner());
		if (!OwningCharacter)
		{
			return;
		}
	}

	GroundSpeed = OwningCharacter->GetVelocity().Size2D();
	SpeedMultiplier = OwningCharacter->GetSpeedMultiplier();
	bIsSprinting = SpeedMultiplier > 1.5f;
	bIsStunned = OwningCharacter->bIsStunned;
	bIsDown = OwningCharacter->IsDownState();
	bIsAsleep = OwningCharacter->IsAsleep();

	if (const UCharacterMovementComponent* Move = OwningCharacter->GetCharacterMovement())
	{
		bIsFalling = Move->IsFalling();
	}
}
