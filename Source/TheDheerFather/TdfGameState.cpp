// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#include "TdfGameState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

void ATdfGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATdfGameState, Phase);
	DOREPLIFETIME(ATdfGameState, Difficulty);
	DOREPLIFETIME(ATdfGameState, GeneratorsTotal);
	DOREPLIFETIME(ATdfGameState, GeneratorsRepaired);
	DOREPLIFETIME(ATdfGameState, FibreTotal);
	DOREPLIFETIME(ATdfGameState, FibreRepaired);
	DOREPLIFETIME(ATdfGameState, RoutersTotal);
	DOREPLIFETIME(ATdfGameState, RoutersChecked);
	DOREPLIFETIME(ATdfGameState, bMonitorsDone);
	DOREPLIFETIME(ATdfGameState, MapRoundSeed);
	DOREPLIFETIME(ATdfGameState, bFrontEndLobby);
	DOREPLIFETIME(ATdfGameState, SelectedMapName);
	DOREPLIFETIME(ATdfGameState, LobbyGeneratorCount);
	DOREPLIFETIME(ATdfGameState, bKillerEnabled);
}

bool ATdfGameState::KillersUnleashed(const UWorld* World)
{
	const ATdfGameState* GameState = World ? World->GetGameState<ATdfGameState>() : nullptr;
	return GameState && (GameState->Phase == ETdfMatchPhase::Repair || GameState->Phase == ETdfMatchPhase::Escape);
}
