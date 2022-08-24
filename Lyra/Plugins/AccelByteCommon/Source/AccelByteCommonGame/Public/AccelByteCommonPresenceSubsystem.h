// Copyright (c) 2018 AccelByte, inc. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AccelByteCommonPresenceSubsystem.generated.h"

#pragma region Blueprintable Structs and Enums

UENUM(BlueprintType)
enum class EABActivityState : uint8
{
	Available,
	InMatch
};

UENUM(BlueprintType)
enum class EABOnlinePresenceState : uint8
{
	Online = 0,
	Offline,
	Away,
	ExtendedAway,
	DoNotDisturb,
	Chat
};

USTRUCT(BlueprintType)
struct FABOnlineUserPresence
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	EABOnlinePresenceState State;

	UPROPERTY(BlueprintReadOnly)
	FString LoggedInPlatform;

	UPROPERTY(BlueprintReadOnly)
	EABActivityState ActivityState;

	FABOnlineUserPresence() :
	State(EABOnlinePresenceState::Offline),
	LoggedInPlatform(""),
	ActivityState(EABActivityState::Available){}

	FABOnlineUserPresence(EABOnlinePresenceState State, FString LoggedInPlatform, EABActivityState ActivityState) :
	State(State), LoggedInPlatform(LoggedInPlatform), ActivityState(ActivityState){}
};

#pragma endregion

DECLARE_LOG_CATEGORY_CLASS(LogAccelByteCommonPresence, Log, All);

/**
 * 
 */
UCLASS()
class ACCELBYTECOMMONGAME_API UAccelByteCommonPresenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:

	/**
	 * Set Local User Presence. This will also set the currently logged in platform as the StatusStr
	 *
	 * @param State Shown state
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Presence")
	void SetPresence(const EABOnlinePresenceState State, const int32 LocalPlayerIndex = 0);

	/**
	 * Query a specific user's presence status. Will call endpoint. Get the presence with GetCachedUserPresence.
	 *
	 * @param TargetUser User to be queried
	 * @param OnComplete Delegate to be executed upon async task completion
	 */
	void QueryUserPresence(FUniqueNetIdPtr TargetUser, TDelegate<void(bool)> OnComplete);

	/**
	 * Return the cached user's presence
	 *
	 * @param TargetUser Target user
	 *
	 * @return Presence state
	 */
	FABOnlineUserPresence GetCachedUserPresence(FUniqueNetIdPtr TargetUser) const;

#pragma region Project Specific Functions and Variables

	static inline const FString Activity_Available = "Available";
	static inline const FString Activity_InMatch = "InMatch";

	UPROPERTY(BlueprintReadOnly)
	TMap<EABActivityState, FString> ActivityStateMap = {
		{EABActivityState::Available, Activity_Available},
		{EABActivityState::InMatch, Activity_InMatch}
	};

	/**
	 * Update ONLY the activity of current presence. Use SetPresence to change the presence state.
	 *
	 * @param ActivityState Desired activity state
	 * @param LocalPlayerIndex Local Player Index
	 */
	void UpdateActivity(const EABActivityState ActivityState, const int32 LocalPlayerIndex);

#pragma endregion

protected:

	void PresenceInSessionDelegateSetup();

	EOnlinePresenceState::Type DesiredPresence;

	void SetPresence_Internal(
		const FString& PresenceString,
		const int32 LocalPlayerIndex = 0);

private:

#pragma region Project Specific Functions and Variables

	static inline const FString PresenceStatusStr_LoggedInPlatformKey = "LoggedInPlatform";
	static inline const FString PresenceStatusStr_Activity = "Activity";

	FString ParsePresenceToString(const FString& ActivityState, const int32 LocalPlayerIndex = 0) const;

	static void ParsePresenceFromString(
		FString& OutLoggedInPlatform,
		FString& OutActivityState,
		const FString& ActivityString);

	static FString ParsePresenceFromString_Helper(
		TArray<FString>& ArrayStrings,
		const FString& Key,
		const bool bRemoveFoundFromArray = true);

#pragma endregion

	static FABOnlineUserPresence BlueprintableOnlineUserPresence(const FOnlineUserPresenceStatus UserPresence);

	IOnlineSubsystem* OSS;
};
