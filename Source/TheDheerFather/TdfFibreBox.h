// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TdfFibreBox.generated.h"

class UStaticMeshComponent;

/**
 * Stage 1 objective: a broken fibre-optic box. Runners hold Interact (F) nearby to fix it.
 * When every box is fixed, the internet comes back... and then all the power dies.
 */
UCLASS()
class THEDHEERFATHER_API ATdfFibreBox : public AActor
{
	GENERATED_BODY()

public:
	ATdfFibreBox();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Fibre")
	float Progress = 0.f;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Fibre")
	bool bRepaired = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tdf|Fibre")
	float RequiredRepairSeconds = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tdf|Fibre")
	float RepairRadius = 250.f;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Tdf|Fibre")
	UStaticMeshComponent* BoxMesh;

	/** Thin beacon so Stage 1 boxes are findable (everyone sees them — the hunt hasn't begun). */
	UPROPERTY(VisibleAnywhere, Category = "Tdf|Fibre")
	UStaticMeshComponent* BeaconMesh;
};
