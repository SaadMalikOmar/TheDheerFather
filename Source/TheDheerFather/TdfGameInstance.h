// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "TdfTypes.h"
#include "TdfGameInstance.generated.h"

/**
 * Persists lobby selections across level travel and runs the multiplayer session layer.
 * Currently the NULL online subsystem (LAN / VPN play); swaps to EOS for internet
 * matchmaking once the Epic dev credentials exist — same IOnlineSession API.
 */
UCLASS()
class THEDHEERFATHER_API UTdfGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Tdf|Session")
	ETdfDifficulty SelectedDifficulty = ETdfDifficulty::Normal;

	/** Party-lobby choices, carried across level travel (the server's instance is authoritative). */
	UPROPERTY(BlueprintReadWrite, Category = "Tdf|Session")
	FString SelectedMapName = TEXT("whitetreeforest");

	/** 0 = use the map's default generator count. */
	UPROPERTY(BlueprintReadWrite, Category = "Tdf|Session")
	int32 GeneratorCountOverride = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Tdf|Session")
	bool bKillerEnabled = true;

	/** Host a joinable LAN game on the current map (F1). */
	UFUNCTION(BlueprintCallable, Category = "Tdf|Session")
	void HostLanGame();

	/** Search the LAN for a game and join the first one found (F2). */
	UFUNCTION(BlueprintCallable, Category = "Tdf|Session")
	void JoinLanGame();

protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	IOnlineSessionPtr GetSessionInterface() const;

	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	FDelegateHandle CreateSessionHandle;
	FDelegateHandle FindSessionsHandle;
	FDelegateHandle JoinSessionHandle;
};
