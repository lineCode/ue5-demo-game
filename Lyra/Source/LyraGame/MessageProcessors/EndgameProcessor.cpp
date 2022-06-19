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

	InitEndgamePlayer();
}

///////////////////////////////////////////////////////////////////////////////

void UEndgameProcessor::InitEndgamePlayer()
{
	UGameInstance const* GameInstance = UGameplayStatics::GetGameInstance(this);
	UCommonUserSubsystem const* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();

	auto const* userInfo = UserSubsystem->GetUserInfoForLocalPlayerIndex(0);

	if( userInfo->InitializationState == ECommonUserInitializationState::LoggedInOnline )
	{
		if( auto* endgameModule = FModuleManager::GetModulePtr<FAccelByteEndgameModule>("AccelByteEndgame") )
		{
			FString nickname = userInfo->GetNickname();

			auto getPlayerHandler = endgame::CreateHandler([this, nickname](endgame::HandlerResult const& result)
			{
				UE_LOG(LogTemp, Display, TEXT("Endgame Player Created"));

				m_loggedInPlayerUniqueId = nickname;
			});

			endgameModule->GetOrCreatePlayer(nickname, getPlayerHandler);

			m_endgameHandlers.Add(getPlayerHandler);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void UEndgameProcessor::OnEliminationMessage
(
	FGameplayTag Channel, 
	const FLyraVerbMessage& Payload
)
{
#if !UE_EDITOR
	if( Payload.Instigator != Payload.Target )
	{
		if( ALyraPlayerState* instigator = Cast<ALyraPlayerState>(Payload.Instigator) )
		{
			if( instigator->GetPlayerConnectionType() == ELyraPlayerConnectionType::Player )
			{
				if( !m_loggedInPlayerUniqueId.IsEmpty() && instigator->GetPlayerName() == m_loggedInPlayerUniqueId )
				{
					if( auto* endgameModule = FModuleManager::GetModulePtr<FAccelByteEndgameModule>("AccelByteEndgame") )
					{
						endgame::HandlerPtr getPlayerHandler = endgameModule->AwardToken(m_loggedInPlayerUniqueId, FGuid("096ed1f8-bb04-4a4a-b660-0f4ae02d4cfd"));

						m_endgameHandlers.Add(getPlayerHandler);
					}
				}
			}
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
