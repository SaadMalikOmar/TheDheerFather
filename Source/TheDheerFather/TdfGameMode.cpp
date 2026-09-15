// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#include "TdfGameMode.h"
#include "TdfRunnerCharacter.h"
#include "TdfKillerCharacter.h"
#include "TdfRunners.h"
#include "TdfKillers.h"
#include "TdfShrine.h"
#include "TdfPlayerController.h"
#include "TdfHUD.h"
#include "TdfGameInstance.h"
#include "TdfGameState.h"
#include "TdfGenerator.h"
#include "TdfFibreBox.h"
#include "TdfHouseActors.h"
#include "TdfMapBuilder.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ATdfGameMode::ATdfGameMode()
{
	// The full roster (swap entries for Blueprint characters once models are hooked up).
	SelectableCharacters = {
		ATdfRunner_Dheer::StaticClass(),      // 0
		ATdfRunner_Troos::StaticClass(),      // 1
		ATdfRunner_Julius::StaticClass(),     // 2
		ATdfRunner_Musa::StaticClass(),       // 3
		ATdfRunner_Shaun::StaticClass(),      // 4
		ATdfRunner_Emma::StaticClass(),       // 5
		ATdfKiller_SkinnyBear::StaticClass(), // 6
		ATdfKiller_Lucki::StaticClass(),      // 7
		ATdfKiller_TungTung::StaticClass()    // 8
	};

	RunnerClass = ATdfRunner_Dheer::StaticClass();
	KillerClass = ATdfKiller_SkinnyBear::StaticClass();

	DefaultPawnClass = ATdfRunner_Dheer::StaticClass();
	PlayerControllerClass = ATdfPlayerController::StaticClass();
	HUDClass = ATdfHUD::StaticClass();
	GameStateClass = ATdfGameState::StaticClass();
}

TSubclassOf<ATdfCharacterBase> ATdfGameMode::GetCharacterClass(int32 Index) const
{
	return SelectableCharacters.IsValidIndex(Index) ? SelectableCharacters[Index] : nullptr;
}

void ATdfGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Party-lobby choices survive level travel inside the (server's) GameInstance.
	const UTdfGameInstance* GI = GetGameInstance<UTdfGameInstance>();
	if (GI)
	{
		Difficulty = GI->SelectedDifficulty;
	}

	if (ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>())
	{
		TdfState->Difficulty = Difficulty;
		if (GI)
		{
			TdfState->SelectedMapName = GI->SelectedMapName;
			TdfState->bKillerEnabled = GI->bKillerEnabled;
			TdfState->LobbyGeneratorCount = (GI->GeneratorCountOverride > 0) ? GI->GeneratorCountOverride : 3;
		}
	}
	RefreshObjectiveCount();
}

bool ATdfGameMode::IsHost(const ATdfPlayerController* PC) const
{
	return PC && PC == GetWorld()->GetFirstPlayerController();
}

void ATdfGameMode::TravelToMap(const FString& MapName)
{
	// ServerTravel is a no-op in offline/standalone PIE — that was "H doesn't start the game".
	if (GetWorld()->GetNetMode() == NM_Standalone)
	{
		UGameplayStatics::OpenLevel(GetWorld(), FName(*MapName));
	}
	else
	{
		GetWorld()->ServerTravel(MapName + TEXT("?listen"));
	}
}

void ATdfGameMode::CycleSelectedMap(ATdfPlayerController* Requester)
{
	if (!IsHost(Requester))
	{
		return;
	}
	// (BearCampus rejoins the list once it has a TdfMapBuilder — it's a bare landscape for now.)
	static const TArray<FString> Maps = { TEXT("WhiteTreeForest"), TEXT("WonderPond"), TEXT("Whitewood") };
	UTdfGameInstance* GI = GetGameInstance<UTdfGameInstance>();
	ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>();
	if (!GI || !TdfState)
	{
		return;
	}
	int32 Idx = Maps.IndexOfByKey(GI->SelectedMapName);
	Idx = (Idx + 1) % Maps.Num();
	GI->SelectedMapName = Maps[Idx];
	TdfState->SelectedMapName = Maps[Idx];
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, FString::Printf(TEXT("Map: %s"), *Maps[Idx]));
	}
}

void ATdfGameMode::CycleGeneratorCount(ATdfPlayerController* Requester)
{
	if (!IsHost(Requester))
	{
		return;
	}
	UTdfGameInstance* GI = GetGameInstance<UTdfGameInstance>();
	ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>();
	if (!GI || !TdfState)
	{
		return;
	}
	const int32 Current = (GI->GeneratorCountOverride > 0) ? GI->GeneratorCountOverride : 3;
	const int32 Next = (Current % 6) + 1; // 1..6
	GI->GeneratorCountOverride = Next;
	TdfState->LobbyGeneratorCount = Next;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, FString::Printf(TEXT("Generators: %d"), Next));
	}
}

void ATdfGameMode::ToggleKillerEnabled(ATdfPlayerController* Requester)
{
	if (!IsHost(Requester))
	{
		return;
	}
	UTdfGameInstance* GI = GetGameInstance<UTdfGameInstance>();
	ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>();
	if (!GI || !TdfState)
	{
		return;
	}
	GI->bKillerEnabled = !GI->bKillerEnabled;
	TdfState->bKillerEnabled = GI->bKillerEnabled;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
			GI->bKillerEnabled ? TEXT("Killer: ON") : TEXT("Killer: OFF (practice)"));
	}
}

void ATdfGameMode::RefreshObjectiveCount()
{
	ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>();
	if (!TdfState || TdfState->Phase == ETdfMatchPhase::RunnersWin || TdfState->Phase == ETdfMatchPhase::KillerWins)
	{
		return;
	}

	int32 Total = 0;
	for (TActorIterator<ATdfGenerator> It(GetWorld()); It; ++It)
	{
		++Total;
	}
	TdfState->GeneratorsTotal = Total;

	int32 Fibre = 0;
	for (TActorIterator<ATdfFibreBox> It(GetWorld()); It; ++It)
	{
		++Fibre;
	}
	TdfState->FibreTotal = Fibre;

	int32 Routers = 0;
	for (TActorIterator<ATdfRouter> It(GetWorld()); It; ++It)
	{
		++Routers;
	}
	TdfState->RoutersTotal = Routers;

	int32 MonitorSeats = 0;
	for (TActorIterator<ATdfSeat> It(GetWorld()); It; ++It)
	{
		if ((*It)->bIsMonitorSeat)
		{
			++MonitorSeats;
		}
	}
	if (MonitorSeats == 0)
	{
		TdfState->bMonitorsDone = true; // maps without starter houses skip the PC stage
	}

	// Phase only advances past the lobby once the host starts the match.
	if (bMatchStarted)
	{
		if (TdfState->RoutersChecked < TdfState->RoutersTotal)
		{
			TdfState->Phase = ETdfMatchPhase::CheckRouter;
		}
		else if (TdfState->FibreRepaired < TdfState->FibreTotal)
		{
			TdfState->Phase = ETdfMatchPhase::FixInternet;
		}
		else if (!TdfState->bMonitorsDone)
		{
			TdfState->Phase = ETdfMatchPhase::ReturnHome;
		}
		else
		{
			TdfState->Phase = (Total == 0 || TdfState->GeneratorsRepaired >= Total) ? ETdfMatchPhase::Escape : ETdfMatchPhase::Repair;
		}
	}
}

void ATdfGameMode::NotifyRouterChecked()
{
	ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>();
	if (!TdfState || TdfState->Phase != ETdfMatchPhase::CheckRouter)
	{
		return;
	}
	++TdfState->RoutersChecked;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
			FString::Printf(TEXT("Routers checked %d/%d"), TdfState->RoutersChecked, TdfState->RoutersTotal));
	}
	if (TdfState->RoutersChecked >= TdfState->RoutersTotal)
	{
		RefreshObjectiveCount();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Cyan, TEXT("It's the STREET's internet. Go find the broken fibre boxes!"));
		}
	}
}

void ATdfGameMode::RecheckMonitorSeating()
{
	ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>();
	if (!TdfState || TdfState->Phase != ETdfMatchPhase::ReturnHome || TdfState->bMonitorsDone)
	{
		return;
	}

	int32 Alive = 0, AtMonitors = 0;
	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		const ATdfRunnerCharacter* Runner = *It;
		if (!Runner || Runner->IsDead() || Runner->HasEscaped())
		{
			continue;
		}
		++Alive;
		if (Runner->bSeated && Runner->SeatedOn.IsValid() && Runner->SeatedOn->bIsMonitorSeat)
		{
			++AtMonitors;
		}
	}

	if (Alive > 0 && AtMonitors >= Alive)
	{
		TdfState->bMonitorsDone = true;
		RefreshObjectiveCount();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Red,
				TEXT("\"wait... guys...\" ...THE POWER JUST DIED. LOAD SHEDDING. THE HUNT BEGINS."));
		}
	}
}

void ATdfGameMode::NotifyFibreRepaired()
{
	ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>();
	if (!TdfState || TdfState->Phase != ETdfMatchPhase::FixInternet)
	{
		return;
	}

	++TdfState->FibreRepaired;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan,
			FString::Printf(TEXT("Fibre box fixed (%d/%d)"), TdfState->FibreRepaired, TdfState->FibreTotal));
	}

	if (TdfState->FibreRepaired >= TdfState->FibreTotal)
	{
		RefreshObjectiveCount(); // -> ReturnHome (or straight to the blackout on maps without PCs)
		if (GEngine)
		{
			if (TdfState->Phase == ETdfMatchPhase::ReturnHome)
			{
				GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green,
					TEXT("INTERNET'S BACK! Everyone home, lock the door, get on your PC."));
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Red,
					TEXT("...THE POWER JUST DIED. LOAD SHEDDING. THE HUNT BEGINS."));
			}
		}
	}
}

void ATdfGameMode::StartMatch(ATdfPlayerController* Requester)
{
	if (!IsHost(Requester))
	{
		return;
	}

	// After a win/loss, the host's H sends the whole party back to the lobby for a rematch.
	if (bMatchStarted)
	{
		if (const ATdfGameState* EndState = GetWorld()->GetGameState<ATdfGameState>())
		{
			if (EndState->Phase == ETdfMatchPhase::RunnersWin || EndState->Phase == ETdfMatchPhase::KillerWins)
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("Back to the lobby..."));
				}
				TravelToMap(TEXT("PartyLobby"));
			}
		}
		return;
	}

	// On the party-lobby island, H launches the selected map instead of starting in place.
	if (const ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>())
	{
		if (TdfState->bFrontEndLobby)
		{
			if (const UTdfGameInstance* GI = GetGameInstance<UTdfGameInstance>())
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
						FString::Printf(TEXT("LAUNCHING %s..."), *GI->SelectedMapName));
				}
				TravelToMap(GI->SelectedMapName);
			}
			return;
		}
	}

	// Normal/Hardcore: characters are randomly allocated, one of each (see difficulties.md).
	if (Difficulty != ETdfDifficulty::Casual)
	{
		TArray<int32> RunnerIndices = { 0, 1, 2, 3, 4, 5 };
		for (int32 i = RunnerIndices.Num() - 1; i > 0; --i)
		{
			RunnerIndices.Swap(i, FMath::RandRange(0, i));
		}

		int32 NextRunner = 0;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			ATdfPlayerController* PC = Cast<ATdfPlayerController>(It->Get());
			if (!PC)
			{
				continue;
			}
			const bool bIsKillerPlayer = Cast<ATdfKillerCharacter>(PC->GetPawn()) != nullptr;
			int32 Index;
			if (bIsKillerPlayer)
			{
				Index = FMath::RandRange(6, 8); // random killer
			}
			else
			{
				Index = RunnerIndices[NextRunner % RunnerIndices.Num()];
				++NextRunner;
			}
			if (TSubclassOf<ATdfCharacterBase> Chosen = GetCharacterClass(Index))
			{
				PC->SpawnAndPossess(Chosen);
			}
			PC->ClientCloseMenu();
		}
	}

	bMatchStarted = true;
	RefreshObjectiveCount();

	// THE OPENING: every runner is at their setup, mid-game, when the connection drops.
	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		ATdfRunnerCharacter* Runner = *It;
		if (!Runner || Runner->bSeated || Runner->IsDead())
		{
			continue;
		}
		ATdfSeat* NearestSeat = nullptr;
		float BestDist = 1e12f;
		for (TActorIterator<ATdfSeat> SeatIt(GetWorld()); SeatIt; ++SeatIt)
		{
			ATdfSeat* Seat = *SeatIt;
			if (Seat && Seat->bIsMonitorSeat && !Seat->Occupant.IsValid())
			{
				const float Dist = FVector::DistSquared(Seat->GetActorLocation(), Runner->GetActorLocation());
				if (Dist < BestDist)
				{
					BestDist = Dist;
					NearestSeat = Seat;
				}
			}
		}
		if (NearestSeat)
		{
			Runner->SitOn(NearestSeat);
		}
	}

	// If the killer is Tung Tung, his shrine rises near his spawn.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (const APlayerController* PC = It->Get())
		{
			if (const ATdfKiller_TungTung* TungTung = Cast<ATdfKiller_TungTung>(PC->GetPawn()))
			{
				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
				GetWorld()->SpawnActor<ATdfShrine>(ATdfShrine::StaticClass(),
					TungTung->GetActorLocation() + TungTung->GetActorForwardVector() * 500.f,
					FRotator::ZeroRotator, Params);
				break;
			}
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green, TEXT("MATCH STARTED"));
	}
}

void ATdfGameMode::SetDifficulty(ETdfDifficulty NewDifficulty)
{
	Difficulty = NewDifficulty;
	if (UTdfGameInstance* GI = GetGameInstance<UTdfGameInstance>())
	{
		GI->SelectedDifficulty = NewDifficulty; // survives travel to the match map
	}
	if (ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>())
	{
		TdfState->Difficulty = NewDifficulty;
	}
	if (GEngine)
	{
		const TCHAR* Name = (NewDifficulty == ETdfDifficulty::Casual) ? TEXT("CASUAL")
			: (NewDifficulty == ETdfDifficulty::Hardcore) ? TEXT("HARDCORE") : TEXT("NORMAL");
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Magenta, FString::Printf(TEXT("Difficulty: %s"), Name));
	}
}

void ATdfGameMode::NotifyGeneratorRepaired()
{
	ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>();
	if (!TdfState || TdfState->Phase != ETdfMatchPhase::Repair)
	{
		return;
	}

	++TdfState->GeneratorsRepaired;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
			FString::Printf(TEXT("Generator repaired (%d/%d)"), TdfState->GeneratorsRepaired, TdfState->GeneratorsTotal));
	}

	if (TdfState->GeneratorsRepaired >= TdfState->GeneratorsTotal)
	{
		TdfState->Phase = ETdfMatchPhase::Escape;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, TEXT("POWER RESTORED - GET TO THE EXIT!"));
		}
	}
}

void ATdfGameMode::CheckWinConditions()
{
	ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>();
	if (!TdfState || TdfState->Phase == ETdfMatchPhase::Lobby
		|| TdfState->Phase == ETdfMatchPhase::RunnersWin || TdfState->Phase == ETdfMatchPhase::KillerWins)
	{
		return;
	}

	int32 Total = 0, Escaped = 0, Dead = 0, Tied = 0;
	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		const ATdfRunnerCharacter* Runner = *It;
		if (!Runner)
		{
			continue;
		}
		++Total;
		if (Runner->HasEscaped())
		{
			++Escaped;
		}
		else if (Runner->IsDead())
		{
			++Dead;
		}
		else if (Runner->bTiedToShrine)
		{
			++Tied;
		}
	}

	// No runners around (e.g. mid pawn-swap in testing) -> nothing to decide.
	if (Total == 0)
	{
		return;
	}

	const int32 StillFree = Total - Escaped - Dead - Tied;
	if (StillFree == 0)
	{
		if (Tied > 0)
		{
			EndMatch(false); // everyone left is strung up on the shrine — Tung Tung becomes human
		}
		else
		{
			EndMatch(Escaped > 0);
		}
	}
}

void ATdfGameMode::EndMatch(bool bRunnersWin)
{
	if (ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>())
	{
		if (TdfState->Phase != ETdfMatchPhase::RunnersWin && TdfState->Phase != ETdfMatchPhase::KillerWins)
		{
			TdfState->Phase = bRunnersWin ? ETdfMatchPhase::RunnersWin : ETdfMatchPhase::KillerWins;
		}
	}
}

UClass* ATdfGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	const UTdfGameInstance* GI = GetGameInstance<UTdfGameInstance>();
	const bool bKillerAllowed = !GI || GI->bKillerEnabled;
	const ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>();
	const bool bOnLobbyIsland = TdfState && TdfState->bFrontEndLobby;

	// Standalone keeps the solo-test flow; the party-lobby island has no killer either.
	// In a network game on a real map, the first player to join becomes the killer.
	if (GetNetMode() != NM_Standalone && !bKillerAssigned && KillerClass && bKillerAllowed && !bOnLobbyIsland)
	{
		bKillerAssigned = true;
		return KillerClass;
	}
	return RunnerClass ? RunnerClass.Get() : ATdfRunnerCharacter::StaticClass();
}

AActor* ATdfGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// The first network player becomes the killer -> put them in the killer house if the map has one.
	if (GetNetMode() != NM_Standalone && !bKillerAssigned)
	{
		for (TActorIterator<ATdfKillerStart> It(GetWorld()); It; ++It)
		{
			return *It;
		}
	}

	// Runners: everyone claims their OWN house for the whole match.
	if (Player)
	{
		if (TWeakObjectPtr<AActor>* Claimed = ClaimedHomeStarts.Find(Player))
		{
			if (Claimed->IsValid())
			{
				return Claimed->Get();
			}
		}
		for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
		{
			APlayerStart* Start = *It;
			if (!Start || Start->IsA(ATdfKillerStart::StaticClass()))
			{
				continue;
			}
			bool bTaken = false;
			for (const TPair<TWeakObjectPtr<AController>, TWeakObjectPtr<AActor>>& Pair : ClaimedHomeStarts)
			{
				if (Pair.Value.Get() == Start && Pair.Key.IsValid() && Pair.Key.Get() != Player)
				{
					bTaken = true;
					break;
				}
			}
			if (!bTaken)
			{
				ClaimedHomeStarts.Add(Player, Start);
				return Start;
			}
		}
	}
	return Super::ChoosePlayerStart_Implementation(Player);
}
