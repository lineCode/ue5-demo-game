// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Messages/GameplayMessageProcessor.h"
#include "GameplayTagContainer.h"
#include "AccelByteEndgame.h"
#include "EndgameProcessor.generated.h"

struct FLyraVerbMessage;
class APlayerState;

UCLASS()
class UEndgameProcessor : public UGameplayMessageProcessor
{
	GENERATED_BODY()

public:
	virtual void StartListening() override;
	virtual void StopListening() override;
	
private:
	void OnEliminationMessage(FGameplayTag Channel, const FLyraVerbMessage& Payload);
	void InitEndgamePlayer();
	void GetPlayerWalletAddress();
	void AwardToken(FGuid const& itemId);

private:
	TArray<endgame::HandlerPtr> m_endgameHandlers;
	FString m_loggedInPlayerUniqueId;
	FGuid m_playerId;
	FString m_walletAddress;

	IConsoleCommand* m_consoleCommandShowToken = nullptr;
	IConsoleCommand* m_consoleCommandShowWallet = nullptr;
	IConsoleCommand* m_consoleCommandAwardKillToken = nullptr;
};
