// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TdfTypes.h"
#include "TdfGameState.generated.h"

/** Replicated match state so every client's HUD can show phase + objective progress. */
UCLASS()
class THEDHEERFATHER_API ATdfGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tdf|Match")
	ETdfMatchPhase Phase = ETdfMatchPhase::Lobby;

	/** Mirrored from the GameMode so every client's HUD can show it. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tdf|Match")
	ETdfDifficulty Difficulty = ETdfDifficulty::Normal;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tdf|Match")
	int32 GeneratorsTotal = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tdf|Match")
	int32 GeneratorsRepaired = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tdf|Match")
	int32 FibreTotal = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tdf|Match")
	int32 FibreRepaired = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tdf|Match")
	int32 RoutersTotal = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tdf|Match")
	int32 RoutersChecked = 0;

	/** Set once everyone made it back to a PC (or the map has no monitor seats). */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tdf|Match")
	bool bMonitorsDone = false;

	/** Per-round layout seed (spawn/loot/closed houses, trees). Server rolls it; clients build the identical street. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tdf|Match")
	int32 MapRoundSeed = 0;

	// --- Party-lobby settings (visible to everyone; only the leader can change them) ---

	/** True on the front-end lobby island: H launches the selected map instead of the match. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tdf|Lobby")
	bool bFrontEndLobby = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tdf|Lobby")
	FString SelectedMapName = TEXT("whitetreeforest");

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tdf|Lobby")
	int32 LobbyGeneratorCount = 3;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tdf|Lobby")
	bool bKillerEnabled = true;

	/** The killer may only hurt runners once the power is out (Stage 2 onward). */
	static bool KillersUnleashed(const UWorld* World);
};
