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

private:
	void OnEliminationMessage(FGameplayTag Channel, const FLyraVerbMessage& Payload);
	void InitEndgamePlayer();

private:
	TArray<endgame::HandlerPtr> m_endgameHandlers;
	FString m_loggedInPlayerUniqueId;
};
