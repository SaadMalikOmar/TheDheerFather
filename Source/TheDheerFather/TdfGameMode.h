// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TdfTypes.h"
#include "TdfGameMode.generated.h"

/** Base match rules. Difficulty selects character allocation, killer lethality and ability limits. */
UCLASS()
class THEDHEERFATHER_API ATdfGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATdfGameMode();

	/** In a network game the first player becomes the killer, everyone else runners. */
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	/** Killers spawn at the killer house (ATdfKillerStart) when the map has one. */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual void BeginPlay() override;

	/** Called by generators when they finish; flips to Escape when all are done. */
	void NotifyGeneratorRepaired();

	/** Stage 1: called by fibre boxes; when all are fixed, the power dies and the hunt begins. */
	void NotifyFibreRepaired();

	/** Stage 1a: a home router was checked. */
	void NotifyRouterChecked();

	/** Stage 1c: re-evaluate whether every living runner is back at a monitor seat. */
	void RecheckMonitorSeating();

	/** Evaluates runner deaths/escapes and ends the match when a side has won. */
	void CheckWinConditions();

	/** Force the match result (e.g. Casual killer death -> runners win). */
	void EndMatch(bool bRunnersWin);

	/** Change difficulty at runtime (testing hotkeys 8/9/0) and mirror it into the GameState. */
	void SetDifficulty(ETdfDifficulty NewDifficulty);

	/** Party-lobby leader controls (only the host may call these). */
	void CycleSelectedMap(class ATdfPlayerController* Requester);
	void CycleGeneratorCount(class ATdfPlayerController* Requester);
	void ToggleKillerEnabled(class ATdfPlayerController* Requester);

	bool IsHost(const class ATdfPlayerController* PC) const;

	/** Offline/standalone-safe map change (OpenLevel solo, ServerTravel when hosting). */
	void TravelToMap(const FString& MapName);

	/** Recount generators in the level (map builder spawns them at BeginPlay, order isn't guaranteed). */
	void RefreshObjectiveCount();

	/** Host starts the match from the lobby. Normal/Hardcore randomise character allocation first. */
	void StartMatch(class ATdfPlayerController* Requester);

	bool HasMatchStarted() const { return bMatchStarted; }

private:
	bool bMatchStarted = false;

	/** Each player claims one spawn house for the whole match. */
	TMap<TWeakObjectPtr<AController>, TWeakObjectPtr<AActor>> ClaimedHomeStarts;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tdf|Match")
	ETdfDifficulty Difficulty = ETdfDifficulty::Normal;

	/** Default pawn classes for auto-spawn (runners) and network killer assignment. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tdf|Match")
	TSubclassOf<class ATdfCharacterBase> RunnerClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tdf|Match")
	TSubclassOf<class ATdfCharacterBase> KillerClass;

	/** Selection-screen roster: 0-5 runners (Dheer..Mahnam), 6-8 killers (SkinnyBear, Lucki, TungTung). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tdf|Match")
	TArray<TSubclassOf<class ATdfCharacterBase>> SelectableCharacters;

	TSubclassOf<class ATdfCharacterBase> GetCharacterClass(int32 Index) const;

private:
	bool bKillerAssigned = false;
};
