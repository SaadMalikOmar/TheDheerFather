#include "TdfGameInstance.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

namespace
{
	void SessionMsg(const FString& Msg, FColor Color = FColor::Cyan)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.f, Color, Msg);
		}
	}

	/** Our tag on the shared Steam test AppID — so we only ever see OUR sessions. */
	const FString TdfSearchKeyword = TEXT("TheDheerFatherV1");

	bool IsOnlineService()
	{
		const IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
		return OSS && OSS->GetSubsystemName() != TEXT("NULL");
	}
}

IOnlineSessionPtr UTdfGameInstance::GetSessionInterface() const
{
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	return OSS ? OSS->GetSessionInterface() : nullptr;
}

void UTdfGameInstance::HostLanGame()
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		SessionMsg(TEXT("No online service (is Steam running?)"), FColor::Red);
		return;
	}

	// Tear down any stale session first.
	if (Sessions->GetNamedSession(NAME_GameSession))
	{
		Sessions->DestroySession(NAME_GameSession);
	}

	const bool bOnline = IsOnlineService();

	FOnlineSessionSettings Settings;
	Settings.bIsLANMatch = !bOnline;
	Settings.NumPublicConnections = 8;
	Settings.bShouldAdvertise = true;
	Settings.bAllowJoinInProgress = true;
	Settings.bIsDedicated = false;
	// Online (Steam): presence lobbies punch through routers — this is the "actually online" part.
	Settings.bUsesPresence = bOnline;
	Settings.bUseLobbiesIfAvailable = bOnline;
	Settings.bAllowJoinViaPresence = bOnline;
	Settings.Set(SEARCH_KEYWORDS, TdfSearchKeyword, EOnlineDataAdvertisementType::ViaOnlineService);

	CreateSessionHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UTdfGameInstance::OnCreateSessionComplete));

	SessionMsg(bOnline ? TEXT("Hosting ONLINE game (Steam)...") : TEXT("Hosting LAN game..."));
	Sessions->CreateSession(0, NAME_GameSession, Settings);
}

void UTdfGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (IOnlineSessionPtr Sessions = GetSessionInterface())
	{
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionHandle);
	}
	if (!bWasSuccessful)
	{
		SessionMsg(TEXT("Failed to host the game"), FColor::Red);
		return;
	}

	// Relaunch the current map as a listen server; friends can now find and join it.
	if (UWorld* World = GetWorld())
	{
		const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		SessionMsg(FString::Printf(TEXT("Game is UP - hosting %s. Friends: press F2!"), *MapName), FColor::Green);
		World->ServerTravel(MapName + TEXT("?listen"));
	}
}

void UTdfGameInstance::JoinLanGame()
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		SessionMsg(TEXT("No online service (is Steam running?)"), FColor::Red);
		return;
	}

	const bool bOnline = IsOnlineService();

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = !bOnline;
	SessionSearch->MaxSearchResults = 100;
	if (bOnline)
	{
		SessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, TdfSearchKeyword, EOnlineComparisonOp::Equals);
		SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	}

	FindSessionsHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UTdfGameInstance::OnFindSessionsComplete));

	SessionMsg(bOnline ? TEXT("Searching ONLINE (Steam)...") : TEXT("Searching for LAN games..."));
	Sessions->FindSessions(0, SessionSearch.ToSharedRef());
}

void UTdfGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsHandle);
	}
	if (!bWasSuccessful || !SessionSearch.IsValid() || SessionSearch->SearchResults.Num() == 0)
	{
		SessionMsg(TEXT("No games found (host pressed F1 first?)"), FColor::Orange);
		return;
	}

	// Only join OUR game — the shared Steam test AppID has other people's sessions on it too.
	int32 PickedIndex = INDEX_NONE;
	for (int32 i = 0; i < SessionSearch->SearchResults.Num(); ++i)
	{
		FString Keyword;
		if (SessionSearch->SearchResults[i].Session.SessionSettings.Get(SEARCH_KEYWORDS, Keyword)
			&& Keyword == TdfSearchKeyword)
		{
			PickedIndex = i;
			break;
		}
	}
	if (PickedIndex == INDEX_NONE)
	{
		if (!IsOnlineService())
		{
			PickedIndex = 0; // LAN broadcast: trust it, it's our own network
		}
		else
		{
			SessionMsg(TEXT("No Dheer Father games found online"), FColor::Orange);
			return;
		}
	}

	SessionMsg(FString::Printf(TEXT("Found the game - joining...")));
	JoinSessionHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UTdfGameInstance::OnJoinSessionComplete));
	Sessions->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[PickedIndex]);
}

void UTdfGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionHandle);
	}
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		SessionMsg(TEXT("Failed to join the game"), FColor::Red);
		return;
	}

	FString ConnectUrl;
	if (Sessions.IsValid() && Sessions->GetResolvedConnectString(NAME_GameSession, ConnectUrl))
	{
		if (APlayerController* PC = GetFirstLocalPlayerController())
		{
			SessionMsg(FString::Printf(TEXT("Connecting to %s"), *ConnectUrl), FColor::Green);
			PC->ClientTravel(ConnectUrl, TRAVEL_Absolute);
		}
	}
}
