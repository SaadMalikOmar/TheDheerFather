#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "TdfGE_Damage.generated.h"

/** Instant gameplay effect: -50 Health. Placeholder fixed damage for testing; real weapons will vary. */
UCLASS()
class THEDHEERFATHER_API UTdfGE_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UTdfGE_Damage();
};
