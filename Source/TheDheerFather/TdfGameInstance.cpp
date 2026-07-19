#include "TdfGameInstance.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

namespace
{
	void SessionMsg(const FString& Msg, FColor Color = FColor::Cyan)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.f, Color, Msg);
		}
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
		SessionMsg(TEXT("No online subsystem available"), FColor::Red);
		return;
	}

	// Tear down any stale session first.
	if (Sessions->GetNamedSession(NAME_GameSession))
	{
		Sessions->DestroySession(NAME_GameSession);
	}

	FOnlineSessionSettings Settings;
	Settings.bIsLANMatch = true;
	Settings.NumPublicConnections = 8;
	Settings.bShouldAdvertise = true;
	Settings.bAllowJoinInProgress = true;
	Settings.bUsesPresence = false;
	Settings.bIsDedicated = false;

	CreateSessionHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UTdfGameInstance::OnCreateSessionComplete));

	SessionMsg(TEXT("Hosting LAN game..."));
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
		SessionMsg(TEXT("Failed to host the LAN game"), FColor::Red);
		return;
	}

	// Relaunch the current map as a listen server; friends on the LAN can now find it.
	if (UWorld* World = GetWorld())
	{
		const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		SessionMsg(FString::Printf(TEXT("LAN game up - hosting %s"), *MapName), FColor::Green);
		World->ServerTravel(MapName + TEXT("?listen"));
	}
}

void UTdfGameInstance::JoinLanGame()
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		SessionMsg(TEXT("No online subsystem available"), FColor::Red);
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 20;

	FindSessionsHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UTdfGameInstance::OnFindSessionsComplete));

	SessionMsg(TEXT("Searching for LAN games..."));
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
		SessionMsg(TEXT("No LAN games found"), FColor::Orange);
		return;
	}

	SessionMsg(FString::Printf(TEXT("Found %d game(s) - joining..."), SessionSearch->SearchResults.Num()));
	JoinSessionHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UTdfGameInstance::OnJoinSessionComplete));
	Sessions->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[0]);
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
