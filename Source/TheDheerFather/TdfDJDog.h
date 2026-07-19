#pragma once

#include "CoreMinimal.h"
#include "TdfCharacterBase.h"
#include "TdfDJDog.generated.h"

/**
 * DJ — Lucki's rabid dog (his ultimate). Chases the nearest runner, bites to injure and
 * drag them down. Built on the character base so runners can fight him: ~4 weapon hits
 * put him down for good (the design wants two people on the job).
 */
UCLASS()
class THEDHEERFATHER_API ATdfDJDog : public ATdfCharacterBase
{
	GENERATED_BODY()

public:
	ATdfDJDog();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnHealthDepleted() override;

	UPROPERTY(EditAnywhere, Category = "Tdf|DJ") float BiteRange = 200.f;
	UPROPERTY(EditAnywhere, Category = "Tdf|DJ") float BiteDamage = 30.f;
	UPROPERTY(EditAnywhere, Category = "Tdf|DJ") float BiteStun = 1.5f;
	UPROPERTY(EditAnywhere, Category = "Tdf|DJ") float BiteCooldown = 4.f;
	UPROPERTY(EditAnywhere, Category = "Tdf|DJ") float AggroRange = 6000.f;

	/**
	 * SCOUT: tracks the nearest runner and pings their position to Lucki (no biting).
	 * ATTACK: lone wolf — hunts and bites on his own (can be baited and killed).
	 * DEFEND: sticks to Lucki, only attacks runners that come near his master.
	 * Lucki's whistle (RMB) cycles the modes.
	 */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|DJ")
	uint8 DJMode = 1; // 0=Scout, 1=Attack, 2=Defend

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_MarkTarget(FVector Location);

	float MarkCooldownRemaining = 0.f;

public:

protected:
	virtual void InitializeAttributes() override;

	float BiteCooldownRemaining = 0.f;
};
