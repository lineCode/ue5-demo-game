// Copyright Epic Games, Inc. All Rights Reserved.

#include "MessageProcessors/EndgameProcessor.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "NativeGameplayTags.h"
#include "Messages/LyraVerbMessage.h"
#include "Messages/LyraVerbMessageHelpers.h"
#include "GameFramework/PlayerState.h"
#include "Player/LyraPlayerState.h"
#include "CommonUserSubsystem.h"
#include "CommonSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_Elimination_Message, "Lyra.Elimination.Message");

///////////////////////////////////////////////////////////////////////////////

void UEndgameProcessor::StartListening()
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	AddListenerHandle(MessageSubsystem.RegisterListener(TAG_Lyra_Elimination_Message, this, &ThisClass::OnEliminationMessage));

	m_consoleCommandShowToken = IConsoleManager::Get().RegisterConsoleCommand(TEXT("endgame.ShowKillTokenOnOpenSea"), TEXT("Show kill token on open sea"), FConsoleCommandWithArgsDelegate::CreateLambda(
	[&](const TArray< FString >& Args)
	{
		FPlatformProcess::LaunchURL(TEXT("https://testnets.opensea.io/assets/mumbai/0x9c4698d03d6993dbb1bd51fe46d4ce9799f62ded/1"), NULL, NULL);
	}));

	m_consoleCommandShowWallet = IConsoleManager::Get().RegisterConsoleCommand(TEXT("endgame.ShowWalletOnOpenSea"), TEXT("Show wallet on OpenSea"), FConsoleCommandWithArgsDelegate::CreateLambda(
	[&](const TArray< FString >& Args)
	{
		FString openseaWalletUrl = FString("https://testnets.opensea.io/") + m_walletAddress;
		FPlatformProcess::LaunchURL(*openseaWalletUrl, NULL, NULL);
	}));

	m_consoleCommandAwardKillToken = IConsoleManager::Get().RegisterConsoleCommand(TEXT("endgame.AwardKillToken"), TEXT("Award a kill token"), FConsoleCommandWithArgsDelegate::CreateLambda(
	[&](const TArray< FString >& Args)
	{
		AwardToken(FGuid("fe56684f-578a-4916-bfd7-a83336353822"));
	}));

	InitEndgamePlayer();
}

///////////////////////////////////////////////////////////////////////////////

void UEndgameProcessor::StopListening()
{
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("endgame.ShowKillTokenOnOpenSea"),false);
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("endgame.ShowWalletOnOpenSea"),false);
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("endgame.AwardKillToken"),false);
	m_consoleCommandShowToken = nullptr;
	m_consoleCommandShowWallet = nullptr;
	m_consoleCommandAwardKillToken = nullptr;
}

///////////////////////////////////////////////////////////////////////////////

void UEndgameProcessor::InitEndgamePlayer()
{
#if !UE_EDITOR
	UGameInstance const* GameInstance = UGameplayStatics::GetGameInstance(this);
	UCommonUserSubsystem const* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();

	auto const* userInfo = UserSubsystem->GetUserInfoForLocalPlayerIndex(0);

	if( userInfo->InitializationState == ECommonUserInitializationState::LoggedInOnline || userInfo->InitializationState == ECommonUserInitializationState::LoggedInLocalOnly )
	{
		if( auto* endgameModule = FModuleManager::GetModulePtr<FAccelByteEndgameModule>("AccelByteEndgame") )
		{
			FString nickname = userInfo->GetNickname();

			auto getPlayerHandler = endgame::CreateHandler([this, nickname](endgame::HandlerResult const& result)
			{
				UE_LOG(LogTemp, Display, TEXT("Endgame Player Created"));

				if (result.error == endgame::ErrorType::None && result.responseObjects.Num() == 1)
				{
					endgame::Player player = *static_cast<endgame::Player*>(result.responseObjects[0].Get());
					m_playerId = player.m_objectId;
					m_loggedInPlayerUniqueId = nickname;
					GetPlayerWalletAddress();
				}
			});

			endgameModule->GetOrCreatePlayer(nickname, getPlayerHandler);

			m_endgameHandlers.Add(getPlayerHandler);
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////

void UEndgameProcessor::GetPlayerWalletAddress()
{
#if !UE_EDITOR
	UGameInstance const* GameInstance = UGameplayStatics::GetGameInstance(this);
	UCommonUserSubsystem const* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();

	auto const* userInfo = UserSubsystem->GetUserInfoForLocalPlayerIndex(0);

	if (userInfo->InitializationState == ECommonUserInitializationState::LoggedInOnline || userInfo->InitializationState == ECommonUserInitializationState::LoggedInLocalOnly)
	{
		if (auto* endgameModule = FModuleManager::GetModulePtr<FAccelByteEndgameModule>("AccelByteEndgame"))
		{
			auto getWalletAddressHandler = endgame::CreateHandler([this](endgame::HandlerResult const& result)
			{
				UE_LOG(LogTemp, Display, TEXT("Endgame Player Created"));

				if (result.error == endgame::ErrorType::None && result.responseObjects.Num() == 1)
				{
					endgame::Account account = *static_cast<endgame::Account*>(result.responseObjects[0].Get());
					m_walletAddress = account.m_address;
				}

			});

			endgameModule->GetPlayerWalletAddress(m_playerId, FGuid("ed50aa96-6afd-486e-b248-413339e81bd2"), getWalletAddressHandler);

			m_endgameHandlers.Add(getWalletAddressHandler);
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////

void UEndgameProcessor::OnEliminationMessage
(
	FGameplayTag Channel,
	const FLyraVerbMessage& Payload
)
{
#if !UE_EDITOR
	if (Payload.Instigator != Payload.Target)
	{
		if (ALyraPlayerState* instigator = Cast<ALyraPlayerState>(Payload.Instigator))
		{
			if (instigator->GetPlayerConnectionType() == ELyraPlayerConnectionType::Player)
			{
				if (!m_loggedInPlayerUniqueId.IsEmpty() && instigator->GetPlayerName() == m_loggedInPlayerUniqueId)
				{
					AwardToken(FGuid("fe56684f-578a-4916-bfd7-a83336353822"));
				}
			}
		}
	}
#endif
}


///////////////////////////////////////////////////////////////////////////////

void UEndgameProcessor::AwardToken
(
	FGuid const& itemId
)
{
#if !UE_EDITOR
	if (auto* endgameModule = FModuleManager::GetModulePtr<FAccelByteEndgameModule>("AccelByteEndgame"))
	{
		auto awardTokenHandler = endgame::CreateHandler([this](endgame::HandlerResult const& result)
		{
			if (result.error == endgame::ErrorType::None)
			{
				UE_LOG(LogTemp, Display, TEXT("https://testnets.opensea.io/assets/mumbai/0x9c4698d03d6993dbb1bd51fe46d4ce9799f62ded/1"));
			}
		});

		endgameModule->AwardToken(m_loggedInPlayerUniqueId, itemId, awardTokenHandler);

		m_endgameHandlers.Add(awardTokenHandler);
	}
#endif
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
