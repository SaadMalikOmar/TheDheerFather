// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TdfShrine.generated.h"

class UStaticMeshComponent;

/**
 * Tung Tung's shrine. He tranquilizes runners, carries them here (F to pick up / deliver)
 * and ties them to it. Teammates can free tied runners (F, like a revive). If every runner
 * still in the match ends up tied at once, Tung Tung becomes human — and wins.
 */
UCLASS()
class THEDHEERFATHER_API ATdfShrine : public AActor
{
	GENERATED_BODY()

public:
	ATdfShrine();

	/** How close Tung Tung must be to deliver a body. */
	UPROPERTY(EditAnywhere, Category = "Tdf")
	float DeliverRadius = 500.f;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Tdf") UStaticMeshComponent* BaseMesh;
	UPROPERTY(VisibleAnywhere, Category = "Tdf") UStaticMeshComponent* PillarMesh;
};
