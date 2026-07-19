#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "TdfGE_SpeedBoost.generated.h"

/**
 * Duration gameplay effect: +90 design-speed for 5 seconds, then auto-reverts.
 * Reused for Lucki's Diet Coke and Skinny Bear's sprint. Defined in C++ so no asset is needed yet.
 */
UCLASS()
class THEDHEERFATHER_API UTdfGE_SpeedBoost : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UTdfGE_SpeedBoost();
};
