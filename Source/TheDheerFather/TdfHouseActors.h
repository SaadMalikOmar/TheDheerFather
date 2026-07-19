#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TdfHouseActors.generated.h"

class UStaticMeshComponent;

/**
 * A seat (desk chair, sofa...). Look at it and press E to sit, E again to stand.
 * Monitor seats (the desk chairs in the starter houses) count for Load Shedding's
 * "everyone back at their PC" stage.
 */
UCLASS()
class THEDHEERFATHER_API ATdfSeat : public AActor
{
	GENERATED_BODY()

public:
	ATdfSeat();

	virtual void Tick(float DeltaSeconds) override;

	/** Counts toward the ReturnHome stage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tdf")
	bool bIsMonitorSeat = false;

	/** Hide spots (wardrobes) tuck you fully inside — invisible to anyone outside. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tdf")
	bool bHideSpot = false;

	/** Who's on it (server). */
	TWeakObjectPtr<class ATdfCharacterBase> Occupant;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Tdf")
	UStaticMeshComponent* SeatMesh;

	/** Beacon shown over free monitor seats during the ReturnHome stage. */
	UPROPERTY(VisibleAnywhere, Category = "Tdf")
	UStaticMeshComponent* BeaconMesh;
};

/** The home router. Stage 1, task 1: look at it, press E — "yep, internet's dead". */
UCLASS()
class THEDHEERFATHER_API ATdfRouter : public AActor
{
	GENERATED_BODY()

public:
	ATdfRouter();

	virtual void Tick(float DeltaSeconds) override;

	/** Server: a runner checked this router. */
	void CheckRouter(class ATdfCharacterBase* Checker);

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf")
	bool bChecked = false;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Tdf")
	UStaticMeshComponent* RouterMesh;

	UPROPERTY(VisibleAnywhere, Category = "Tdf")
	UStaticMeshComponent* BeaconMesh;
};
