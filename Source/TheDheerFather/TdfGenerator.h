#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TdfGenerator.generated.h"

class UStaticMeshComponent;

/**
 * Repair objective. A runner stands within RepairRadius and holds Interact (F) to
 * add progress. Engineers repair 3x faster (see runners.md). Server-authoritative.
 */
UCLASS()
class THEDHEERFATHER_API ATdfGenerator : public AActor
{
	GENERATED_BODY()

public:
	ATdfGenerator();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Generator")
	float Progress = 0.f;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Generator")
	bool bRepaired = false;

	/** Effective repair-seconds needed (1 runner at normal speed = this many seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tdf|Generator")
	float RequiredRepairSeconds = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tdf|Generator")
	float RepairRadius = 300.f;

protected:
	/** Should the sky-beacon be visible to the LOCAL player? (difficulty + role + finder device). */
	bool ShouldShowBeaconLocally() const;

	UPROPERTY(VisibleAnywhere, Category = "Tdf|Generator")
	UStaticMeshComponent* GeneratorMesh;

	/** Tall pillar of light so generators can be found across the map (per-viewer visibility). */
	UPROPERTY(VisibleAnywhere, Category = "Tdf|Generator")
	UStaticMeshComponent* BeaconMesh;
};
