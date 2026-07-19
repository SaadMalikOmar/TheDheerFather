#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TdfEscapeZone.generated.h"

class UStaticMeshComponent;

/**
 * THE ROADWORKS — the closed end of the street (50/50 top or bottom of the hill).
 * Escape phase: every runner still alive must gather here TOGETHER; when the whole
 * crew is inside, they flip the electricity back on and everyone wins at once.
 */
UCLASS()
class THEDHEERFATHER_API ATdfEscapeZone : public AActor
{
	GENERATED_BODY()

public:
	ATdfEscapeZone();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tdf|Escape")
	float EscapeRadius = 600.f;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Tdf|Escape") UStaticMeshComponent* ZoneMesh;
	UPROPERTY(VisibleAnywhere, Category = "Tdf|Escape") UStaticMeshComponent* BarrierA;
	UPROPERTY(VisibleAnywhere, Category = "Tdf|Escape") UStaticMeshComponent* BarrierB;
	UPROPERTY(VisibleAnywhere, Category = "Tdf|Escape") UStaticMeshComponent* BarrierC;
	UPROPERTY(VisibleAnywhere, Category = "Tdf|Escape") UStaticMeshComponent* SwitchBox;

	/** Last "X/Y at the roadworks" headcount we announced (server). */
	int32 LastAnnouncedPresent = -1;
};
