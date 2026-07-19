#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TdfTypes.h"
#include "TdfPlayerController.generated.h"

class ATdfCharacterBase;

/**
 * Handles the character-select screen (auto-opens at start, reopen with B, pick with 1-9)
 * and testing hotkeys. Spawning is server-authoritative.
 */
UCLASS()
class THEDHEERFATHER_API ATdfPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** True while the select menu is showing — the HUD draws it, digit keys pick from it. */
	UPROPERTY(BlueprintReadOnly, Category = "Tdf")
	bool bCharacterMenuOpen = false;

	/** Server-side: replace the current pawn with a new character. */
	void SpawnAndPossess(TSubclassOf<ATdfCharacterBase> PawnClass);

	/** Server -> owning client: show a message on their screen. */
	UFUNCTION(Client, Reliable)
	void ClientNotify(const FString& Message);

	/** Server -> owning client: close the character menu (match started with random characters). */
	UFUNCTION(Client, Reliable)
	void ClientCloseMenu();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	void ToggleCharacterMenu();

	/** Digit pressed while the menu is open -> spawn as roster index (0-8). */
	void SelectCharacterIndex(int32 Index);

	UFUNCTION(Server, Reliable)
	void ServerSpawnCharacterByIndex(int32 Index);

	/** Testing hotkeys 8/9/0 (only when the menu is CLOSED): switch difficulty. */
	void SelectDifficultyCasual();
	void SelectDifficultyNormal();
	void SelectDifficultyHardcore();
	UFUNCTION(Server, Reliable) void ServerSetDifficulty(ETdfDifficulty NewDifficulty);

	/** H: host starts the match from the lobby. */
	void OnStartMatchPressed();
	UFUNCTION(Server, Reliable) void ServerStartMatch();

	/** F1 / F2: host or join a LAN game (Null online subsystem; EOS later, same flow). */
	void OnHostLanPressed();
	void OnJoinLanPressed();

	/** Party-lobby leader keys: M = map, G = generators, K = killer on/off. */
	void OnCycleMapPressed();
	void OnCycleGensPressed();
	void OnToggleKillerPressed();
	UFUNCTION(Server, Reliable) void ServerCycleMap();
	UFUNCTION(Server, Reliable) void ServerCycleGens();
	UFUNCTION(Server, Reliable) void ServerToggleKiller();
};
