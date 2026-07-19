#include "TdfPlayerController.h"
#include "TdfCharacterBase.h"
#include "TdfKillerCharacter.h"
#include "TdfGameInstance.h"
#include "TdfGameMode.h"
#include "TdfMapBuilder.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"

void ATdfPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Pick-your-character screen greets you at spawn.
	bCharacterMenuOpen = true;
}

void ATdfPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindAction("ToggleCharacterMenu", IE_Pressed, this, &ATdfPlayerController::ToggleCharacterMenu);

		DECLARE_DELEGATE_OneParam(FSelectCharDelegate, int32);
		InputComponent->BindAction<FSelectCharDelegate>("SelectChar1", IE_Pressed, this, &ATdfPlayerController::SelectCharacterIndex, 0);
		InputComponent->BindAction<FSelectCharDelegate>("SelectChar2", IE_Pressed, this, &ATdfPlayerController::SelectCharacterIndex, 1);
		InputComponent->BindAction<FSelectCharDelegate>("SelectChar3", IE_Pressed, this, &ATdfPlayerController::SelectCharacterIndex, 2);
		InputComponent->BindAction<FSelectCharDelegate>("SelectChar4", IE_Pressed, this, &ATdfPlayerController::SelectCharacterIndex, 3);
		InputComponent->BindAction<FSelectCharDelegate>("SelectChar5", IE_Pressed, this, &ATdfPlayerController::SelectCharacterIndex, 4);
		InputComponent->BindAction<FSelectCharDelegate>("SelectChar6", IE_Pressed, this, &ATdfPlayerController::SelectCharacterIndex, 5);
		InputComponent->BindAction<FSelectCharDelegate>("SelectChar7", IE_Pressed, this, &ATdfPlayerController::SelectCharacterIndex, 6);
		InputComponent->BindAction<FSelectCharDelegate>("SelectChar8", IE_Pressed, this, &ATdfPlayerController::SelectCharacterIndex, 7);
		InputComponent->BindAction<FSelectCharDelegate>("SelectChar9", IE_Pressed, this, &ATdfPlayerController::SelectCharacterIndex, 8);

		InputComponent->BindAction("DifficultyCasual", IE_Pressed, this, &ATdfPlayerController::SelectDifficultyCasual);
		InputComponent->BindAction("DifficultyNormal", IE_Pressed, this, &ATdfPlayerController::SelectDifficultyNormal);
		InputComponent->BindAction("DifficultyHardcore", IE_Pressed, this, &ATdfPlayerController::SelectDifficultyHardcore);
		InputComponent->BindAction("StartMatch", IE_Pressed, this, &ATdfPlayerController::OnStartMatchPressed);
		InputComponent->BindAction("HostLan", IE_Pressed, this, &ATdfPlayerController::OnHostLanPressed);
		InputComponent->BindAction("JoinLan", IE_Pressed, this, &ATdfPlayerController::OnJoinLanPressed);
		InputComponent->BindAction("CycleMap", IE_Pressed, this, &ATdfPlayerController::OnCycleMapPressed);
		InputComponent->BindAction("CycleGens", IE_Pressed, this, &ATdfPlayerController::OnCycleGensPressed);
		InputComponent->BindAction("ToggleKiller", IE_Pressed, this, &ATdfPlayerController::OnToggleKillerPressed);
	}
}

void ATdfPlayerController::OnCycleMapPressed() { ServerCycleMap(); }
void ATdfPlayerController::OnCycleGensPressed() { ServerCycleGens(); }
void ATdfPlayerController::OnToggleKillerPressed() { ServerToggleKiller(); }

void ATdfPlayerController::ServerCycleMap_Implementation()
{
	if (ATdfGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATdfGameMode>() : nullptr)
	{
		GameMode->CycleSelectedMap(this);
	}
}

void ATdfPlayerController::ServerCycleGens_Implementation()
{
	if (ATdfGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATdfGameMode>() : nullptr)
	{
		GameMode->CycleGeneratorCount(this);
	}
}

void ATdfPlayerController::ServerToggleKiller_Implementation()
{
	if (ATdfGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATdfGameMode>() : nullptr)
	{
		GameMode->ToggleKillerEnabled(this);
	}
}

void ATdfPlayerController::OnHostLanPressed()
{
	if (UTdfGameInstance* GI = GetGameInstance<UTdfGameInstance>())
	{
		GI->HostLanGame();
	}
}

void ATdfPlayerController::OnJoinLanPressed()
{
	if (UTdfGameInstance* GI = GetGameInstance<UTdfGameInstance>())
	{
		GI->JoinLanGame();
	}
}

void ATdfPlayerController::OnStartMatchPressed()
{
	ServerStartMatch();
}

void ATdfPlayerController::ServerStartMatch_Implementation()
{
	if (ATdfGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATdfGameMode>() : nullptr)
	{
		GameMode->StartMatch(this);
	}
}

void ATdfPlayerController::ClientNotify_Implementation(const FString& Message)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, Message);
	}
}

void ATdfPlayerController::ClientCloseMenu_Implementation()
{
	bCharacterMenuOpen = false;
}

void ATdfPlayerController::ToggleCharacterMenu()
{
	bCharacterMenuOpen = !bCharacterMenuOpen;
}

void ATdfPlayerController::SelectCharacterIndex(int32 Index)
{
	if (!bCharacterMenuOpen)
	{
		return;
	}
	bCharacterMenuOpen = false;
	ServerSpawnCharacterByIndex(Index);
}

void ATdfPlayerController::ServerSpawnCharacterByIndex_Implementation(int32 Index)
{
	const ATdfGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATdfGameMode>() : nullptr;
	if (!GameMode)
	{
		return;
	}
	TSubclassOf<ATdfCharacterBase> ChosenClass = GameMode->GetCharacterClass(Index);
	if (!ChosenClass)
	{
		return;
	}

	// One of each: no two players on the same character; only one killer player.
	const bool bChoseKiller = ChosenClass->IsChildOf(ATdfKillerCharacter::StaticClass());
	if (bChoseKiller)
	{
		if (const UTdfGameInstance* GI = GetGameInstance<UTdfGameInstance>())
		{
			if (!GI->bKillerEnabled)
			{
				ClientNotify(TEXT("The killer is disabled for this match."));
				return;
			}
		}
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* Other = It->Get();
		if (!Other || Other == this || !Other->GetPawn())
		{
			continue;
		}
		if (Other->GetPawn()->GetClass() == *ChosenClass)
		{
			ClientNotify(TEXT("That character is already taken."));
			return;
		}
		if (bChoseKiller && Cast<ATdfKillerCharacter>(Other->GetPawn()))
		{
			ClientNotify(TEXT("There is already a killer in this match."));
			return;
		}
	}

	SpawnAndPossess(ChosenClass);
}

// Difficulty hotkeys share digit keys with the menu, so they only fire while it's closed.
void ATdfPlayerController::SelectDifficultyCasual() { if (!bCharacterMenuOpen) { ServerSetDifficulty(ETdfDifficulty::Casual); } }
void ATdfPlayerController::SelectDifficultyNormal() { if (!bCharacterMenuOpen) { ServerSetDifficulty(ETdfDifficulty::Normal); } }
void ATdfPlayerController::SelectDifficultyHardcore() { if (!bCharacterMenuOpen) { ServerSetDifficulty(ETdfDifficulty::Hardcore); } }

void ATdfPlayerController::ServerSetDifficulty_Implementation(ETdfDifficulty NewDifficulty)
{
	if (ATdfGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATdfGameMode>() : nullptr)
	{
		GameMode->SetDifficulty(NewDifficulty);
	}
}

void ATdfPlayerController::SpawnAndPossess(TSubclassOf<ATdfCharacterBase> PawnClass)
{
	UWorld* World = GetWorld();
	if (!World || !PawnClass)
	{
		return;
	}

	FVector Location(0.f, 0.f, 300.f);
	const FRotator Rotation = GetControlRotation();
	if (APawn* Old = GetPawn())
	{
		Location = Old->GetActorLocation() + FVector(0.f, 0.f, 60.f);
	}

	// Killers spawn at the killer house when the map has one.
	if (PawnClass->IsChildOf(ATdfKillerCharacter::StaticClass()))
	{
		for (TActorIterator<ATdfKillerStart> It(World); It; ++It)
		{
			Location = (*It)->GetActorLocation();
			break;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Coming back from spectating (testing respawn via the menu): return to the playing state first.
	if (PlayerState && PlayerState->IsSpectator())
	{
		PlayerState->SetIsSpectator(false);
		ChangeState(NAME_Playing);
		ClientGotoState(NAME_Playing);
	}

	APawn* PreviousPawn = GetPawn();
	if (APawn* NewPawn = World->SpawnActor<APawn>(PawnClass, Location, Rotation, SpawnParams))
	{
		UnPossess();
		if (PreviousPawn)
		{
			PreviousPawn->Destroy();
		}
		Possess(NewPawn);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
				FString::Printf(TEXT("Spawned as %s"), *PawnClass->GetName()));
		}
	}
}
