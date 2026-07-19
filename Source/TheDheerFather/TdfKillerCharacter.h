#pragma once

#include "CoreMinimal.h"
#include "TdfCharacterBase.h"
#include "TdfTypes.h"
#include "TdfKillerCharacter.generated.h"

/** The hunter. One per match. Ability behaviour is built on GAS. */
UCLASS()
class THEDHEERFATHER_API ATdfKillerCharacter : public ATdfCharacterBase
{
	GENERATED_BODY()

public:
	ATdfKillerCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tdf|Killer")
	ETdfKillerType KillerType = ETdfKillerType::Mimic_SkinnyBear;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tdf|Killer")
	FTdfKillerStats Stats;

	/** Knockout health. In Hardcore the killer cannot be downed (design: distort only). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Killer")
	float CurrentKealth = 200.f;

	/** Killer's top HUD bar shows Kealth instead of Health. */
	virtual float GetHealthBarValue() const override;
	virtual float GetHealthBarMax() const override;
	virtual FString GetHealthBarLabel() const override;

	/** Kealth at 0: Casual = killed (runners win), Normal = brief knockout, Hardcore = can't happen. */
	virtual void OnKealthDepleted() override;
	virtual void Recover() override;

	/** Server: Axon bleed — Kealth damage over time. */
	void StartBleed(int32 Ticks, float DamagePerTick);

protected:
	void BleedTick();
	FTimerHandle BleedTimerHandle;
	int32 BleedTicksLeft = 0;
	float BleedDamagePerTick = 0.f;

public:

protected:
	virtual void BeginPlay() override;
	virtual void InitializeAttributes() override;
};
