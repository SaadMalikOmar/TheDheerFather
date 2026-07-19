#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TdfTypes.generated.h"

/**
 * Shared enums and stat structs for The Dheer Father.
 * These mirror the design docs (runners.md / killers.md / difficulties.md) so that
 * tuning values live in DataTables and can be edited without recompiling.
 */

/** Difficulty mode. Drives selection rules, killer lethality and ability limits. See difficulties.md. */
UENUM(BlueprintType)
enum class ETdfDifficulty : uint8
{
	Casual    UMETA(DisplayName = "Casual"),
	Normal    UMETA(DisplayName = "Normal"),
	Hardcore  UMETA(DisplayName = "Hardcore")
};

/** The six base runner roles. See runners.md. */
UENUM(BlueprintType)
enum class ETdfRunnerRole : uint8
{
	Scout      UMETA(DisplayName = "Scout (Dheer)"),
	Tank       UMETA(DisplayName = "Tank (Troos)"),
	Engineer   UMETA(DisplayName = "Engineer (Julius)"),
	Trickster  UMETA(DisplayName = "Trickster (Musa)"),
	Stealth    UMETA(DisplayName = "Stealth (Shaun)"),
	Medic      UMETA(DisplayName = "Medic (Emma)")
};

/** The killers. See killers.md. */
UENUM(BlueprintType)
enum class ETdfKillerType : uint8
{
	Mimic_SkinnyBear    UMETA(DisplayName = "Mimic (Skinny Bear)"),
	Stalker_Lucki       UMETA(DisplayName = "Stalker (Lucki)"),
	Combatant_TungTung  UMETA(DisplayName = "Combatant (Tung Tung)")
};

/** Runner weapons (see weapons.md). Found as pickups; scarce. */
UENUM(BlueprintType)
enum class ETdfWeaponType : uint8
{
	None       UMETA(DisplayName = "Fists"),
	Axon       UMETA(DisplayName = "The Axon (axe)"),
	TripleTBat UMETA(DisplayName = "Triple T Bat"),
	Taser      UMETA(DisplayName = "Lip Stick Tazer")
};

/** Match flow: lobby -> fix the internet -> blackout, repair generators -> escape -> a side wins. */
UENUM(BlueprintType)
enum class ETdfMatchPhase : uint8
{
	Lobby        UMETA(DisplayName = "Lobby"),
	CheckRouter  UMETA(DisplayName = "Stage 1a: Check the Router"),
	FixInternet  UMETA(DisplayName = "Stage 1b: Repair the Fibre"),
	ReturnHome   UMETA(DisplayName = "Stage 1c: Back to your PC"),
	Repair       UMETA(DisplayName = "Stage 2: Repair Generators"),
	Escape       UMETA(DisplayName = "Escape"),
	RunnersWin   UMETA(DisplayName = "Runners Win"),
	KillerWins   UMETA(DisplayName = "Killer Wins")
};

/**
 * Per-runner tuning values. Speed uses the normalised design scale (100 = average human jog);
 * it is converted to cm/s by ATdfCharacterBase. See README.md "Normalized speed scale".
 */
USTRUCT(BlueprintType)
struct FTdfRunnerStats : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Stamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Speed = 60.f;

	/** 0..100 cowardice. Drives fear distortion, breathing and detectability. See design-pillars.md (PUD). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0", ClampMax = "100"))
	float PudLevel = 50.f;
};

/** Per-killer tuning values. Kealth = knockout health (not death health). See killers.md / design-pillars.md. */
USTRUCT(BlueprintType)
struct FTdfKillerStats : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Stamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Speed = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Kealth = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Damage = 50.f;
};
