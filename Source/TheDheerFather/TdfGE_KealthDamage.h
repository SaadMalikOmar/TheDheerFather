// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "TdfGE_KealthDamage.generated.h"

/** Instant Kealth damage (runner weapons vs killers). Magnitude via SetByCaller Data.Damage. */
UCLASS()
class THEDHEERFATHER_API UTdfGE_KealthDamage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UTdfGE_KealthDamage();
};
