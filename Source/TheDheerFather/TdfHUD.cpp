// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#include "TdfHUD.h"
#include "TdfCharacterBase.h"
#include "TdfRunnerCharacter.h"
#include "TdfRunners.h"
#include "TdfKillerCharacter.h"
#include "TdfKillers.h"
#include "TdfDeployables.h"
#include "TdfHouseActors.h"
#include "TdfFibreBox.h"
#include "TdfPlayerController.h"
#include "TdfGameState.h"
#include "TdfGenerator.h"
#include "TdfTypes.h"
#include "TdfGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "EngineUtils.h"

void ATdfHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	// --- Character select screen (overrides everything while open) ---
	if (const ATdfPlayerController* TdfPC = Cast<ATdfPlayerController>(GetOwningPlayerController()))
	{
		if (TdfPC->bCharacterMenuOpen)
		{
			const float PanelX = Canvas->SizeX * 0.5f - 380.f;
			float RowY = Canvas->SizeY * 0.5f - 280.f;

			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.85f), PanelX - 20.f, RowY - 20.f, 800.f, 560.f);

			DrawText(TEXT("CHOOSE YOUR CHARACTER"), FLinearColor::White, PanelX, RowY, nullptr, 1.6f);
			RowY += 40.f;
			DrawText(TEXT("ALL: WASD move | Shift sprint | Ctrl walk | Space jump / hold at a tree to CLIMB | E interact | LMB weapon"), FLinearColor(0.7f, 0.7f, 0.7f), PanelX, RowY, nullptr, 0.95f);
			RowY += 34.f;

			DrawText(TEXT("--- RUNNERS ---  (F = hold breath)"), FLinearColor(0.4f, 0.8f, 1.f), PanelX, RowY, nullptr, 1.1f); RowY += 28.f;
			DrawText(TEXT("[1] DHEER    Scout      HP100 SPD60  | hold Q: smart watch (heartbeat + generator sense) | slow repairs"), FLinearColor::White, PanelX, RowY, nullptr, 1.f); RowY += 25.f;
			DrawText(TEXT("[2] TROOS    Tank       HP200 SPD55  | trap-proof fridge | can't climb trees"), FLinearColor::White, PanelX, RowY, nullptr, 1.f); RowY += 25.f;
			DrawText(TEXT("[3] JULIUS   Engineer   HP75  SPD70  | repairs 3x faster | bad eyes, zero fear"), FLinearColor::White, PanelX, RowY, nullptr, 1.f); RowY += 25.f;
			DrawText(TEXT("[4] MUSA     Trickster  HP100 SPD50  | Q: decoy | RMB: hidden voice speaker x3 | doomscrolls every 30s"), FLinearColor::White, PanelX, RowY, nullptr, 1.f); RowY += 25.f;
			DrawText(TEXT("[5] SHAUN    Stealth    HP60  SPD100 | still + holding breath = invisible | dies in ONE hit"), FLinearColor::White, PanelX, RowY, nullptr, 1.f); RowY += 25.f;
			DrawText(TEXT("[6] EMMA     Medic      HP75  SPD70  | Q: rave stash - heal + PUD to 0"), FLinearColor::White, PanelX, RowY, nullptr, 1.f); RowY += 32.f;

			DrawText(TEXT("--- KILLERS ---"), FLinearColor(1.f, 0.35f, 0.35f), PanelX, RowY, nullptr, 1.1f); RowY += 28.f;
			DrawText(TEXT("[7] SKINNY BEAR  3RD PERSON, one-shot, SPD190, blind to still prey | RMB: sniff (reveal thru walls) | R: Echo Scream"), FLinearColor(1.f, 0.6f, 0.6f), PanelX, RowY, nullptr, 1.f); RowY += 25.f;
			DrawText(TEXT("[8] LUCKI        SPD70 + the Prius: E sit/keys, RMB lock or start engine, Shift exit | Q: Nani | R: DJ (RMB whistle) | T: Coke"), FLinearColor(1.f, 0.6f, 0.6f), PanelX, RowY, nullptr, 1.f); RowY += 25.f;
			DrawText(TEXT("[9] TUNG TUNG    SPD120, E carries downed prey to his shrine | Q: smash | RMB: dart | T: tree form | R(3min): Root of Flesh"), FLinearColor(1.f, 0.6f, 0.6f), PanelX, RowY, nullptr, 1.f); RowY += 36.f;

			DrawText(TEXT("Press the number to spawn.  [B] closes/reopens this menu."), FLinearColor(0.7f, 0.7f, 0.7f), PanelX, RowY, nullptr, 1.f);
			return;
		}
	}

	ATdfCharacterBase* Char = Cast<ATdfCharacterBase>(GetOwningPawn());
	if (!Char)
	{
		// Spectator (dead player flying around): keep them in the loop.
		if (const ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>())
		{
			FString Objective;
			switch (TdfState->Phase)
			{
			case ETdfMatchPhase::Lobby:
				Objective = TEXT("LOBBY");
				break;
			case ETdfMatchPhase::CheckRouter:
				Objective = FString::Printf(TEXT("ROUTERS  %d / %d"), TdfState->RoutersChecked, TdfState->RoutersTotal);
				break;
			case ETdfMatchPhase::FixInternet:
				Objective = FString::Printf(TEXT("FIBRE BOXES  %d / %d"), TdfState->FibreRepaired, TdfState->FibreTotal);
				break;
			case ETdfMatchPhase::ReturnHome:
				Objective = TEXT("EVERYONE BACK TO THEIR PC");
				break;
			case ETdfMatchPhase::Repair:
				Objective = FString::Printf(TEXT("GENERATORS  %d / %d"), TdfState->GeneratorsRepaired, TdfState->GeneratorsTotal);
				break;
			case ETdfMatchPhase::Escape:     Objective = TEXT("POWER RESTORED - THE HUNT IS ON"); break;
			case ETdfMatchPhase::RunnersWin: Objective = TEXT("RUNNERS WIN!"); break;
			case ETdfMatchPhase::KillerWins: Objective = TEXT("KILLER WINS!"); break;
			}
			DrawText(Objective, FLinearColor::White, Canvas->SizeX * 0.5f - 150.f, 40.f, nullptr, 1.4f);
		}
		DrawText(TEXT("SPECTATING - fly with WASD + mouse"), FLinearColor(0.7f, 0.7f, 0.7f),
			Canvas->SizeX * 0.5f - 160.f, Canvas->SizeY - 60.f, nullptr, 1.1f);
		return;
	}

	// --- Dheer's smart watch (hold Q): battery + killer heartbeat + generator sense ---
	if (const ATdfRunner_Dheer* Dheer = Cast<ATdfRunner_Dheer>(Char))
	{
		if (Dheer->bWatchHeld)
		{
			FString WatchText;
			if (Dheer->WatchBattery > 0.f)
			{
				float NearestDist = -1.f;
				for (TActorIterator<ATdfKillerCharacter> It(GetWorld()); It; ++It)
				{
					const float Dist = FVector::Dist((*It)->GetActorLocation(), Char->GetActorLocation());
					if (NearestDist < 0.f || Dist < NearestDist)
					{
						NearestDist = Dist;
					}
				}
				WatchText = (NearestDist < 0.f)
					? FString::Printf(TEXT("WATCH [batt %.0fs]  no heartbeat detected"), Dheer->WatchBattery)
					: FString::Printf(TEXT("WATCH [batt %.0fs]  KILLER %.0fm"), Dheer->WatchBattery, NearestDist / 100.f);
			}
			else
			{
				WatchText = TEXT("WATCH [battery dead]  generator sense only");
			}
			DrawText(WatchText, FLinearColor(0.4f, 1.f, 0.6f), Canvas->SizeX * 0.5f - 170.f, Canvas->SizeY * 0.62f, nullptr, 1.3f);
		}
	}

	// --- Objective / match phase (top-center), worded per role ---
	const bool bIsKiller = Cast<ATdfKillerCharacter>(Char) != nullptr;
	if (const ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>())
	{
		FString Objective;
		FLinearColor ObjectiveColor = FLinearColor::White;
		switch (TdfState->Phase)
		{
		case ETdfMatchPhase::Lobby:
			Objective = TdfState->bFrontEndLobby
				? TEXT("PARTY LOBBY - bring the boys in with [F1] host / [F2] join, leader presses [H] to LAUNCH")
				: TEXT("LOBBY - pick characters [1-9], host presses [H] to start the match");
			ObjectiveColor = FLinearColor(0.6f, 0.8f, 1.f);
			break;
		case ETdfMatchPhase::CheckRouter:
			Objective = bIsKiller
				? TEXT("They don't know yet. Wait.")
				: FString::Printf(TEXT("\"wth, the game froze\" - GO CHECK A ROUTER  %d / %d   (downstairs, press E on it)"),
					TdfState->RoutersChecked, TdfState->RoutersTotal);
			ObjectiveColor = bIsKiller ? FLinearColor(0.5f, 0.5f, 0.5f) : FLinearColor(0.4f, 0.8f, 1.f);
			break;
		case ETdfMatchPhase::FixInternet:
			Objective = bIsKiller
				? TEXT("The power is still on... wait. Your time comes with the dark.")
				: FString::Printf(TEXT("INTERNET'S DOWN - REPAIR THE FIBRE BOXES  %d / %d   (hold E)"),
					TdfState->FibreRepaired, TdfState->FibreTotal);
			ObjectiveColor = bIsKiller ? FLinearColor(0.5f, 0.5f, 0.5f) : FLinearColor(0.4f, 0.8f, 1.f);
			break;
		case ETdfMatchPhase::ReturnHome:
			Objective = bIsKiller
				? TEXT("They're heading home. Soon.")
				: TEXT("INTERNET'S BACK - GET HOME AND SIT AT YOUR PC (E on the desk chair)");
			ObjectiveColor = bIsKiller ? FLinearColor(0.5f, 0.5f, 0.5f) : FLinearColor(0.4f, 1.f, 0.6f);
			break;
		case ETdfMatchPhase::Repair:
			Objective = bIsKiller
				? FString::Printf(TEXT("HUNT THE RUNNERS - STOP THE REPAIRS  (%d / %d repaired)"),
					TdfState->GeneratorsRepaired, TdfState->GeneratorsTotal)
				: FString::Printf(TEXT("REPAIR THE GENERATORS  %d / %d   (hold E at a generator)"),
					TdfState->GeneratorsRepaired, TdfState->GeneratorsTotal);
			if (bIsKiller) { ObjectiveColor = FLinearColor(1.f, 0.55f, 0.2f); }
			break;
		case ETdfMatchPhase::Escape:
			Objective = bIsKiller
				? TEXT("THEY'RE HEADING FOR THE ROADWORKS - LET NO ONE LEAVE!")
				: TEXT("GENERATORS DONE - EVERYONE ALIVE to the ROADWORKS, flip the power TOGETHER!");
			ObjectiveColor = bIsKiller ? FLinearColor(1.f, 0.4f, 0.1f) : FLinearColor(0.2f, 1.f, 0.2f);
			break;
		case ETdfMatchPhase::RunnersWin:
			Objective = bIsKiller ? TEXT("THE RUNNERS ESCAPED - YOU LOSE   (host: [H] rematch)")
				: TEXT("RUNNERS WIN!   (host: [H] rematch)");
			ObjectiveColor = bIsKiller ? FLinearColor(1.f, 0.2f, 0.2f) : FLinearColor(0.2f, 1.f, 0.2f);
			break;
		case ETdfMatchPhase::KillerWins:
			Objective = bIsKiller ? TEXT("EVERYONE IS DEAD - YOU WIN   (host: [H] rematch)")
				: TEXT("KILLER WINS!   (host: [H] rematch)");
			ObjectiveColor = bIsKiller ? FLinearColor(0.2f, 1.f, 0.2f) : FLinearColor(1.f, 0.2f, 0.2f);
			break;
		}
		DrawText(Objective, ObjectiveColor, Canvas->SizeX * 0.5f - 220.f, 40.f, nullptr, 1.4f);

		// The opening beat: connection's gone, the call is still up.
		if (TdfState->Phase == ETdfMatchPhase::CheckRouter && !bIsKiller)
		{
			DrawText(TEXT("!! CONNECTION LOST !!   ...voice chat still up. \"guys? did your game just die too?\""),
				FLinearColor(1.f, 0.35f, 0.2f), Canvas->SizeX * 0.5f - 260.f, 68.f, nullptr, 1.05f);
		}

		// Party-lobby settings board (leader changes them; everyone sees them).
		if (TdfState->bFrontEndLobby && TdfState->Phase == ETdfMatchPhase::Lobby)
		{
			const TCHAR* DiffName = (TdfState->Difficulty == ETdfDifficulty::Casual) ? TEXT("CASUAL")
				: (TdfState->Difficulty == ETdfDifficulty::Hardcore) ? TEXT("HARDCORE") : TEXT("NORMAL");
			float SettingsY = 84.f;
			DrawText(FString::Printf(TEXT("[M] Map:         %s"), *TdfState->SelectedMapName), FLinearColor::White, Canvas->SizeX * 0.5f - 220.f, SettingsY, nullptr, 1.1f); SettingsY += 24.f;
			DrawText(FString::Printf(TEXT("[G] Generators:  %d"), TdfState->LobbyGeneratorCount), FLinearColor::White, Canvas->SizeX * 0.5f - 220.f, SettingsY, nullptr, 1.1f); SettingsY += 24.f;
			DrawText(FString::Printf(TEXT("[8/9/0] Difficulty: %s"), DiffName), FLinearColor::White, Canvas->SizeX * 0.5f - 220.f, SettingsY, nullptr, 1.1f); SettingsY += 24.f;
			DrawText(FString::Printf(TEXT("[K] Killer:      %s"), TdfState->bKillerEnabled ? TEXT("ON") : TEXT("OFF (practice)")), FLinearColor::White, Canvas->SizeX * 0.5f - 220.f, SettingsY, nullptr, 1.1f);
		}

		// Nearest unrepaired generator's progress while repairing phase is on.
		if (TdfState->Phase == ETdfMatchPhase::Repair)
		{
			float BestDist = 500.f;
			const ATdfGenerator* Nearest = nullptr;
			for (TActorIterator<ATdfGenerator> It(GetWorld()); It; ++It)
			{
				const ATdfGenerator* Gen = *It;
				if (Gen && !Gen->bRepaired)
				{
					const float Dist = FVector::Dist(Gen->GetActorLocation(), Char->GetActorLocation());
					if (Dist < BestDist)
					{
						BestDist = Dist;
						Nearest = Gen;
					}
				}
			}
			if (Nearest)
			{
				DrawText(FString::Printf(TEXT("Generator: %.0f%%"), 100.f * Nearest->Progress / Nearest->RequiredRepairSeconds),
					FLinearColor::Yellow, Canvas->SizeX * 0.5f - 60.f, 72.f, nullptr, 1.2f);
			}
		}
	}

	// --- Personal end states ---
	if (Char->IsDead())
	{
		DrawText(TEXT("YOU DIED"), FLinearColor(1.f, 0.1f, 0.1f), Canvas->SizeX * 0.5f - 60.f, Canvas->SizeY * 0.4f, nullptr, 2.f);
	}
	else if (const ATdfRunnerCharacter* Runner = Cast<ATdfRunnerCharacter>(Char))
	{
		if (Runner->HasEscaped())
		{
			DrawText(TEXT("YOU ESCAPED!"), FLinearColor(0.2f, 1.f, 0.2f), Canvas->SizeX * 0.5f - 80.f, Canvas->SizeY * 0.4f, nullptr, 2.f);
		}
	}

	const float ScreenH = Canvas->SizeY;
	const float BarWidth = 320.f;
	const float BarHeight = 22.f;
	const float Margin = 40.f;
	const float Spacing = 8.f;

	// Bottom-left stack: stamina on the bottom, health/kealth above it.
	float Y = ScreenH - Margin - BarHeight;

	const float MaxStam = Char->GetMaxStamina();
	const float StamFrac = (MaxStam > 0.f) ? Char->GetStamina() / MaxStam : 0.f;
	FLinearColor StamColor = Char->IsWinded() ? FLinearColor(0.6f, 0.3f, 0.0f) : FLinearColor(1.0f, 0.85f, 0.1f);
	DrawBar(Margin, Y, BarWidth, BarHeight, StamFrac, StamColor,
		FString::Printf(TEXT("Stamina  %.0f / %.0f"), Char->GetStamina(), MaxStam));

	Y -= (BarHeight + Spacing);

	const float MaxHP = Char->GetHealthBarMax();
	const float HPFrac = (MaxHP > 0.f) ? Char->GetHealthBarValue() / MaxHP : 0.f;
	DrawBar(Margin, Y, BarWidth, BarHeight, HPFrac, FLinearColor(0.85f, 0.15f, 0.15f),
		FString::Printf(TEXT("%s  %.0f / %.0f"), *Char->GetHealthBarLabel(), Char->GetHealthBarValue(), MaxHP));

	// Status line above the bars.
	float StatusY = Y - 26.f;

	TArray<FString> Status;
	if (Char->IsAsleep())          { Status.Add(FString::Printf(TEXT("ASLEEP %.0fs"), Char->GetSleepTimeRemaining())); }
	else if (Char->IsDownState())  { Status.Add(TEXT("DOWNED")); }
	if (Char->bIsStunned)          { Status.Add(TEXT("STUNNED")); }
	if (const ATdfRunnerCharacter* StateRunner = Cast<ATdfRunnerCharacter>(Char))
	{
		if (StateRunner->bTiedToShrine)   { Status.Add(TEXT("TIED TO THE SHRINE - teammates can free you")); }
		else if (StateRunner->bBeingCarried) { Status.Add(TEXT("TUNG TUNG HAS YOU")); }
	}
	if (Char->IsWinded())          { Status.Add(TEXT("WINDED")); }
	if (Char->IsHoldingBreath())   { Status.Add(TEXT("HOLDING BREATH")); }

	if (Status.Num() > 0)
	{
		DrawText(FString::Join(Status, TEXT("   ")), FLinearColor(1.0f, 0.5f, 0.0f), Margin, StatusY, nullptr, 1.2f);
		StatusY -= 22.f;
	}

	DrawText(FString::Printf(TEXT("Speed x%.2f"), Char->GetSpeedMultiplier()),
		FLinearColor(0.7f, 0.9f, 1.0f), Margin, StatusY, nullptr, 1.0f);

	// Banked SUPER RUNs (earned by keeping the bar full for a minute).
	if (Char->GetSuperRunCharges() > 0 || Char->IsSuperRunActive())
	{
		DrawText(Char->IsSuperRunActive()
			? FString(TEXT(">>> SUPER RUN <<<"))
			: FString::Printf(TEXT("SUPER RUN ready x%d"), Char->GetSuperRunCharges()),
			FLinearColor(0.2f, 1.f, 1.f), Margin + 130.f, StatusY, nullptr, 1.0f);
	}

	// --- LOADOUT PANEL (right side): every ability slot with charge-up bar / uses left ---
	{
		static const TCHAR* SlotKeys[6] = { TEXT("E"), TEXT("Q"), TEXT("LMB"), TEXT("R"), TEXT("T"), TEXT("RMB") };
		const float PanelX = Canvas->SizeX - 320.f;
		float RowY = Canvas->SizeY * 0.32f;
		const float Now = GetWorld()->GetTimeSeconds();
		UAbilitySystemComponent* ASC = Char->GetAbilitySystemComponent();

		for (int32 Slot = 0; Slot < Char->GetKitSlots().Num() && Slot < 6; ++Slot)
		{
			TSubclassOf<UTdfGameplayAbility> AbilityClass = Char->GetKitSlots()[Slot];
			if (!AbilityClass || !ASC)
			{
				continue;
			}

			// Name: "TdfAbility_Sniff" -> "Sniff".
			FString Name = AbilityClass->GetName();
			Name.RemoveFromStart(TEXT("TdfAbility_"));
			Name.RemoveFromEnd(TEXT("_C"));

			// Live state off the per-actor ability instance.
			float CdRemaining = 0.f, CdDuration = 0.f;
			int32 Uses = -1;
			if (FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(AbilityClass))
			{
				const UTdfGameplayAbility* Inst = Cast<UTdfGameplayAbility>(
					Spec->GetPrimaryInstance() ? (UGameplayAbility*)Spec->GetPrimaryInstance() : Spec->Ability.Get());
				if (Inst)
				{
					CdRemaining = FMath::Max(0.f, Inst->HudCooldownEnd - Now);
					CdDuration = Inst->HudCooldownDuration;
					Uses = Inst->HudUsesLeft;
				}
			}

			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), PanelX - 8.f, RowY - 4.f, 300.f, 30.f);

			const bool bReady = CdRemaining <= 0.f && Uses != 0;
			const FLinearColor TextColor = (Uses == 0) ? FLinearColor(0.5f, 0.5f, 0.5f)
				: bReady ? FLinearColor::White : FLinearColor(0.7f, 0.7f, 0.7f);
			FString Label = FString::Printf(TEXT("[%s] %s"), SlotKeys[Slot], *Name);
			if (Uses == 0)
			{
				Label += TEXT("  - EMPTY");
			}
			else if (Uses > 0)
			{
				Label += FString::Printf(TEXT("  x%d"), Uses);
			}
			else if (CdRemaining > 0.f)
			{
				Label += FString::Printf(TEXT("  %.0fs"), CdRemaining);
			}
			DrawText(Label, TextColor, PanelX, RowY, nullptr, 1.05f);

			// Charge-up bar: fills as the cooldown recovers.
			if (CdDuration > 0.f)
			{
				const float Frac = 1.f - FMath::Clamp(CdRemaining / CdDuration, 0.f, 1.f);
				DrawRect(FLinearColor(0.1f, 0.1f, 0.1f, 0.8f), PanelX, RowY + 19.f, 284.f, 4.f);
				DrawRect(bReady ? FLinearColor(0.2f, 1.f, 0.8f) : FLinearColor(1.f, 0.7f, 0.1f),
					PanelX, RowY + 19.f, 284.f * Frac, 4.f);
			}
			RowY += 36.f;
		}

		// Non-GAS specials get their own rows so the loadout is complete.
		auto SpecialRow = [&](const FString& Label, float Frac, const FLinearColor& BarColor)
		{
			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), PanelX - 8.f, RowY - 4.f, 300.f, 30.f);
			DrawText(Label, FLinearColor::White, PanelX, RowY, nullptr, 1.05f);
			if (Frac >= 0.f)
			{
				DrawRect(FLinearColor(0.1f, 0.1f, 0.1f, 0.8f), PanelX, RowY + 19.f, 284.f, 4.f);
				DrawRect(BarColor, PanelX, RowY + 19.f, 284.f * FMath::Clamp(Frac, 0.f, 1.f), 4.f);
			}
			RowY += 36.f;
		};
		if (const ATdfRunner_Dheer* DheerChar = Cast<ATdfRunner_Dheer>(Char))
		{
			SpecialRow(FString::Printf(TEXT("[hold Q] Watch  batt %.0fs"), DheerChar->WatchBattery),
				DheerChar->WatchBattery / 20.f, FLinearColor(0.4f, 1.f, 0.6f));
		}
		if (const ATdfRunner_Musa* MusaChar = Cast<ATdfRunner_Musa>(Char))
		{
			SpecialRow(FString::Printf(TEXT("[hold RMB] Voice note  x%d"), MusaChar->SpeakersLeft),
				MusaChar->SpeakersLeft / 3.f, FLinearColor(0.1f, 0.8f, 0.9f));
		}
		if (Char->GetSuperRunCharges() > 0)
		{
			SpecialRow(FString::Printf(TEXT("[Shift] SUPER RUN  x%d"), Char->GetSuperRunCharges()),
				-1.f, FLinearColor::White);
		}
	}

	// Lucki's dashboard.
	if (const ATdfKiller_Lucki* Lucki = Cast<ATdfKiller_Lucki>(Char))
	{
		// Hunger bar — always visible (doesn't drain while driving).
		DrawText(FString::Printf(TEXT("Hunger  %.0f / 100"), Lucki->Hunger),
			FLinearColor(1.f, 0.6f, 0.2f), Margin, StatusY - 46.f, nullptr, 1.0f);
		DrawRect(FLinearColor(0.12f, 0.08f, 0.04f, 0.85f), Margin, StatusY - 28.f, 220.f, 10.f);
		DrawRect(FLinearColor(1.f, 0.55f, 0.1f), Margin, StatusY - 28.f, 220.f * FMath::Clamp(Lucki->Hunger / 100.f, 0.f, 1.f), 10.f);

		if (Lucki->bInCar)
		{
			if (const ATdfPriusProp* Car = Lucki->GetCar())
			{
				FString Dash;
				if (Car->EngineStartRemaining > 0.f)
				{
					Dash = FString::Printf(TEXT("PRIUS: cranking... %.1fs"), Car->EngineStartRemaining);
				}
				else
				{
					Dash = FString::Printf(TEXT("PRIUS: engine %s | keys %s | petrol %.0f%% | battery %.0f%%   [E keys] [RMB engine] [Shift out]"),
						Car->bEngineOn ? TEXT("ON") : TEXT("off"),
						Car->bKeysInserted ? TEXT("in") : TEXT("out"),
						Car->Petrol, Car->Battery);
				}
				DrawText(Dash, FLinearColor(0.4f, 0.8f, 1.f), Margin, StatusY - 74.f, nullptr, 1.05f);
			}
		}
	}

	// --- DJ has your leg: nothing else matters, SPAM E ---
	if (const ATdfRunnerCharacter* DraggedRunner = Cast<ATdfRunnerCharacter>(Char))
	{
		if (DraggedRunner->bDraggedByDJ)
		{
			DrawText(TEXT("DJ HAS YOUR LEG!  SPAM [E] TO BREAK FREE (1% per press)"),
				FLinearColor(1.f, 0.15f, 0.1f), Canvas->SizeX * 0.5f - 260.f, Canvas->SizeY * 0.5f, nullptr, 1.5f);
		}
	}

	// --- Look-at interaction prompt (centre of screen) ---
	{
		FString Prompt;
		if (Char->bSeated)
		{
			Prompt = TEXT("[E] Stand up");
		}
		else if (AActor* LookAt = Char->GetLookAtActor(420.f))
		{
			if (const ATdfSeat* Seat = Cast<ATdfSeat>(LookAt))
			{
				if (!Seat->Occupant.IsValid())
				{
					Prompt = Seat->bHideSpot ? TEXT("[E] Hide") : (Seat->bIsMonitorSeat ? TEXT("[E] Sit at your PC") : TEXT("[E] Sit"));
				}
			}
			else if (const ATdfRouter* Router = Cast<ATdfRouter>(LookAt))
			{
				Prompt = Router->bChecked ? TEXT("Router: no internet.") : TEXT("[E] Check the router");
			}
			else if (Cast<ATdfGenerator>(LookAt))
			{
				const ATdfRunnerCharacter* KitRunner = Cast<ATdfRunnerCharacter>(Char);
				Prompt = (KitRunner && !KitRunner->bHasRepairKit)
					? TEXT("You need a REPAIR KIT - it's in your house")
					: TEXT("[hold E] Repair  (a 2nd runner must stand lookout)");
			}
			else if (Cast<ATdfFibreBox>(LookAt))
			{
				Prompt = TEXT("[hold E] Fix the fibre box");
			}
			else if (const ATdfPriusProp* Prius = Cast<ATdfPriusProp>(LookAt))
			{
				if (Cast<ATdfKiller_Lucki>(Char))
				{
					Prompt = Prius->bLocked ? TEXT("Locked  [RMB] Unlock") : TEXT("[E] Get in  [RMB] Lock");
				}
				else
				{
					// Runners: an unlocked Prius with keys inside is BEGGING to be robbed.
					Prompt = Prius->bLocked ? TEXT("A blue Prius. Locked.")
						: Prius->bKeysInserted ? TEXT("[E] STEAL THE KEYS")
						: TEXT("A blue Prius. No keys inside.");
				}
			}
		}
		if (!Prompt.IsEmpty())
		{
			DrawText(Prompt, FLinearColor(1.f, 1.f, 0.75f), Canvas->SizeX * 0.5f - 70.f, Canvas->SizeY * 0.56f, nullptr, 1.25f);
		}
	}

	// Fear meter + weapon (runners only).
	if (const ATdfRunnerCharacter* FearRunner = Cast<ATdfRunnerCharacter>(Char))
	{
		const float Fear = FearRunner->GetFearLevel();
		if (Fear > 0.02f)
		{
			DrawBar(Margin, StatusY - 30.f, 200.f, 12.f, Fear, FLinearColor(0.6f, 0.1f, 0.9f), TEXT("FEAR"));
		}

		const TCHAR* WeaponName =
			(FearRunner->CurrentWeapon == ETdfWeaponType::Axon) ? TEXT("THE AXON") :
			(FearRunner->CurrentWeapon == ETdfWeaponType::TripleTBat) ? TEXT("TRIPLE T BAT") :
			(FearRunner->CurrentWeapon == ETdfWeaponType::Taser) ? TEXT("LIP STICK TAZER") : TEXT("fists");
		DrawText(FearRunner->CurrentWeapon == ETdfWeaponType::None
			? FString(TEXT("Weapon: fists (find one!)"))
			: FString::Printf(TEXT("Weapon: %s (%d)"), WeaponName, FearRunner->WeaponDurability),
			FLinearColor(0.9f, 0.9f, 0.6f), Margin, StatusY - 52.f, nullptr, 1.f);
	}

	// Bottom-right: difficulty + testing controls.
	if (const ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>())
	{
		const TCHAR* DiffName = (TdfState->Difficulty == ETdfDifficulty::Casual) ? TEXT("CASUAL")
			: (TdfState->Difficulty == ETdfDifficulty::Hardcore) ? TEXT("HARDCORE") : TEXT("NORMAL");
		DrawText(FString::Printf(TEXT("Difficulty: %s   [B] characters  [8/9/0] difficulty  [E] interact  [F] hold breath  [F1] host LAN  [F2] join LAN"), DiffName),
			FLinearColor(0.6f, 0.6f, 0.6f), Canvas->SizeX - 760.f, Canvas->SizeY - 30.f, nullptr, 0.9f);
	}
}

void ATdfHUD::DrawBar(float X, float Y, float Width, float Height, float Fraction, const FLinearColor& FillColor, const FString& Label)
{
	Fraction = FMath::Clamp(Fraction, 0.f, 1.f);

	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), X - 2.f, Y - 2.f, Width + 4.f, Height + 4.f);   // border
	DrawRect(FLinearColor(0.12f, 0.12f, 0.12f, 0.85f), X, Y, Width, Height);                      // track
	DrawRect(FillColor, X, Y, Width * Fraction, Height);                                          // fill

	DrawText(Label, FLinearColor::White, X + 6.f, Y + 2.f, nullptr, 1.0f);
}
