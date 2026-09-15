// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "TdfCharacterBase.h"
#include "TdfTargetDummy.generated.h"

struct FOnAttributeChangeData;

/**
 * A stationary, unpossessed punching bag for testing abilities. Has the full ASC + attributes,
 * a visible mesh, and prints its health whenever it changes.
 */
UCLASS()
class THEDHEERFATHER_API ATdfTargetDummy : public ATdfCharacterBase
{
	GENERATED_BODY()

public:
	ATdfTargetDummy();

protected:
	virtual void BeginPlay() override;
	virtual void OnHealthDepleted() override;
	virtual void Recover() override;
	virtual void ApplyStun(float Duration) override;
	virtual void RemoveStun() override;

	void OnHealthChanged(const FOnAttributeChangeData& Data);

	UPROPERTY(EditAnywhere, Category = "Tdf|Dummy")
	float StartingHealth = 300.f;
};
