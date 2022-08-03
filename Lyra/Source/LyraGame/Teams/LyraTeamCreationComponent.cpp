// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraTeamCreationComponent.h"

#include "Net/UnrealNetwork.h"
#include "GameModes/LyraExperienceDefinition.h"
#include "GameModes/LyraExperienceManagerComponent.h"
#include "LyraTeamPublicInfo.h"
#include "LyraTeamPrivateInfo.h"
#include "LyraTeamSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Player/LyraPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "GameModes/LyraGameMode.h"

ULyraTeamCreationComponent::ULyraTeamCreationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PublicTeamInfoClass = ALyraTeamPublicInfo::StaticClass();
	PrivateTeamInfoClass = ALyraTeamPrivateInfo::StaticClass();
}

#if WITH_EDITOR
EDataValidationResult ULyraTeamCreationComponent::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = Super::IsDataValid(ValidationErrors);

	//@TODO: TEAMS: Validate that all display assets have the same properties set!

	return Result;
}
#endif

void ULyraTeamCreationComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for the experience load to complete
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	ULyraExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded_HighPriority(FOnLyraExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void ULyraTeamCreationComponent::OnExperienceLoaded(const ULyraExperienceDefinition* Experience)
{
#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		ServerCreateTeams();
		ServerAssignPlayersToTeams();
	}
#endif
}

#if WITH_SERVER_CODE

void ULyraTeamCreationComponent::ServerCreateTeams()
{
	for (const auto& KVP : TeamsToCreate)
	{
		const int32 TeamId = KVP.Key;
		ServerCreateTeam(TeamId, KVP.Value);
	}
}

void ULyraTeamCreationComponent::ServerAssignPlayersToTeams()
{
	// Assign players that already exist to teams
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS))
		{
			ServerChooseTeamForPlayer(LyraPS);
		}
	}

	// Listen for new players logging in
	ALyraGameMode* GameMode = Cast<ALyraGameMode>(GameState->AuthorityGameMode);
	check(GameMode);

	GameMode->OnGameModeCombinedPostLogin().AddUObject(this, &ThisClass::OnPostLogin);
}

void ULyraTeamCreationComponent::ServerChooseTeamForPlayer(ALyraPlayerState* PS)
{
	// #START @AccelByte Implementation: Pre Assigned bots
	// assign pre-assigned team to bots
	if (PS->IsABot())
	{
		ALyraGameMode* LyraGameMode = GetGameMode<ALyraGameMode>();
		FGenericTeamId TeamID;

		if (LyraGameMode)
		{
			if (LyraGameMode->CurrentBotsNum_Team1 < LyraGameMode->PreAssignedBotsNum_Team1)
			{
				TeamID = IntegerToGenericTeamId(1);
				PS->SetGenericTeamId(TeamID);
				LyraGameMode->CurrentBotsNum_Team1++;
			}
			else
			{
				TeamID = IntegerToGenericTeamId(2);
				PS->SetGenericTeamId(TeamID);
			}
		}

		return;
	}

	// Destroy bot when new player joins mid-match
	// Will only be "used" on custom session, since matchmaking have less player limit and no bots
	int32 MaxPlayerInMatch = 0;
	GConfig->GetInt(TEXT("AccelByteSocial"), TEXT("MaxPartyMembers_CustomSession"), MaxPlayerInMatch, GEngineIni);
	const int32 CurrentControllerNum = GetWorld()->GetNumControllers();


	// kick first bots if match full
	if (CurrentControllerNum > MaxPlayerInMatch)
	{
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			const TWeakObjectPtr<AController> Controller = *It;
			if (Controller->PlayerState->IsABot() && HasAuthority())
			{
				APawn* FormerPawn = Controller->GetPawn();
				Controller->UnPossess();
				Controller->Destroy(true);
				FormerPawn->Destroy(true);
				UE_LOG(LogTemp, Log, TEXT("Removing bot due to match is already filled with bots"))
				break;
			}
		}
	}
	// #END

	if (PS->IsOnlyASpectator())
	{
		PS->SetGenericTeamId(FGenericTeamId::NoTeam);
	}
	// #START @AccelByte Implementation: Pre Assigned team
	else if (PS->PreAssignedTeamId != 0)
	{
		const FGenericTeamId TeamID = IntegerToGenericTeamId(PS->PreAssignedTeamId);
		PS->SetGenericTeamId(TeamID);
	}
	// #END
	else
	{
		const FGenericTeamId TeamID = IntegerToGenericTeamId(GetLeastPopulatedTeamID());
		PS->SetGenericTeamId(TeamID);
	}
}

void ULyraTeamCreationComponent::OnPostLogin(AGameModeBase* GameMode, AController* NewPlayer)
{
	check(NewPlayer);
	check(NewPlayer->PlayerState);
	if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(NewPlayer->PlayerState))
	{
		ServerChooseTeamForPlayer(LyraPS);
	}
}

void ULyraTeamCreationComponent::ServerCreateTeam(int32 TeamId, ULyraTeamDisplayAsset* DisplayAsset)
{
	check(HasAuthority());

	//@TODO: ensure the team doesn't already exist

	UWorld* World = GetWorld();
	check(World);

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ALyraTeamPublicInfo* NewTeamPublicInfo = World->SpawnActor<ALyraTeamPublicInfo>(PublicTeamInfoClass, SpawnInfo);
	checkf(NewTeamPublicInfo != nullptr, TEXT("Failed to create public team actor from class %s"), *GetPathNameSafe(*PublicTeamInfoClass));
	NewTeamPublicInfo->SetTeamId(TeamId);
	NewTeamPublicInfo->SetTeamDisplayAsset(DisplayAsset);

	ALyraTeamPrivateInfo* NewTeamPrivateInfo = World->SpawnActor<ALyraTeamPrivateInfo>(PrivateTeamInfoClass, SpawnInfo);
	checkf(NewTeamPrivateInfo != nullptr, TEXT("Failed to create private team actor from class %s"), *GetPathNameSafe(*PrivateTeamInfoClass));
	NewTeamPrivateInfo->SetTeamId(TeamId);
}

int32 ULyraTeamCreationComponent::GetLeastPopulatedTeamID() const
{
	const int32 NumTeams = TeamsToCreate.Num();
	if (NumTeams > 0)
	{
		TMap<int32, uint32> TeamMemberCounts;
		TeamMemberCounts.Reserve(NumTeams);

		for (const auto& KVP : TeamsToCreate)
		{
			const int32 TeamId = KVP.Key;
			TeamMemberCounts.Add(TeamId, 0);
		}

		AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
		for (APlayerState* PS : GameState->PlayerArray)
		{
			if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS))
			{
				const int32 PlayerTeamID = LyraPS->GetTeamId();

				if ((PlayerTeamID != INDEX_NONE) && !LyraPS->IsInactive())	// do not count unassigned or disconnected players
				{
					check(TeamMemberCounts.Contains(PlayerTeamID))
					TeamMemberCounts[PlayerTeamID] += 1;
				}
			}
		}

		// sort by lowest team population, then by team ID
		int32 BestTeamId = INDEX_NONE;
		uint32 BestPlayerCount = TNumericLimits<uint32>::Max();
		for (const auto& KVP : TeamMemberCounts)
		{
			const int32 TestTeamId = KVP.Key;
			const uint32 TestTeamPlayerCount = KVP.Value;

			if (TestTeamPlayerCount < BestPlayerCount)
			{
				BestTeamId = TestTeamId;
				BestPlayerCount = TestTeamPlayerCount;
			}
			else if (TestTeamPlayerCount == BestPlayerCount)
			{
				if ((TestTeamId < BestTeamId) || (BestTeamId == INDEX_NONE))
				{
					BestTeamId = TestTeamId;
					BestPlayerCount = TestTeamPlayerCount;
				}
			}
		}

		return BestTeamId;
	}

	return INDEX_NONE;
}
#endif	// WITH_SERVER_CODE
