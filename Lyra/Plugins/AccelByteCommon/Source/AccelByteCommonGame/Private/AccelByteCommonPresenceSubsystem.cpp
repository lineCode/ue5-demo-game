// Copyright (c) 2018 AccelByte, inc. All rights reserved.


#include "AccelByteCommonPresenceSubsystem.h"

#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemUtils.h"

void UAccelByteCommonPresenceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OSS = Online::GetSubsystem(GetWorld());
	check(OSS);

	PresenceInSessionDelegateSetup();

	// OnLobbyConnected sequence to set presence
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(OSS->GetIdentityInterface());
	check(IdentityInterface);
	IdentityInterface->AddOnConnectLobbyCompleteDelegate_Handle(0, FOnConnectLobbyCompleteDelegate::CreateWeakLambda(
		this, [this](int32 LocalUserNum, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const FString& /*Error*/)
		{
			// Set presence as ONLINE
			UAccelByteCommonPresenceSubsystem* PresenceSubsystem =
				GetGameInstance()->GetSubsystem<UAccelByteCommonPresenceSubsystem>();
			PresenceSubsystem->SetPresence(EABOnlinePresenceState::Online);
		}));
}

void UAccelByteCommonPresenceSubsystem::SetPresence(const EABOnlinePresenceState State, const int32 LocalPlayerIndex)
{
	// store Desired Online Presence State
	DesiredPresence = static_cast<EOnlinePresenceState::Type>(State);

	const FString PresenceString = ParsePresenceToString(Activity_Available, LocalPlayerIndex);

	// Set Presence
	SetPresence_Internal(PresenceString, LocalPlayerIndex);
}

void UAccelByteCommonPresenceSubsystem::QueryUserPresence(FUniqueNetIdPtr TargetUser, TDelegate<void(bool)> OnComplete)
{
	const IOnlinePresencePtr Presence = OSS->GetPresenceInterface();
	check(Presence.IsValid())

	if (!TargetUser.IsValid())
	{
		UE_LOG(LogAccelByteCommonPresence, Warning, TEXT("TargetUser is not valid"));
		return;
	}

	Presence->QueryPresence(*TargetUser, IOnlinePresence::FOnPresenceTaskCompleteDelegate::CreateWeakLambda(this,
		[OnComplete](const FUniqueNetId& UserId, const bool bWasSuccessful)
		{
			OnComplete.ExecuteIfBound(bWasSuccessful);
#if UE_BUILD_DEVELOPMENT
			const TSharedRef<const FUniqueNetIdAccelByteUser> ABUser = FUniqueNetIdAccelByteUser::Cast(UserId);
			UE_LOG(LogAccelByteCommonPresence, Log, TEXT("QueryPresence: %s successful: %s"),
				*ABUser->GetAccelByteId(),
				bWasSuccessful? TEXT("TRUE") : TEXT("FALSE"));
#endif
		}));
}

FABOnlineUserPresence UAccelByteCommonPresenceSubsystem::GetCachedUserPresence(FUniqueNetIdPtr TargetUser) const
{
	const IOnlinePresencePtr Presence = OSS->GetPresenceInterface();
	check(Presence.IsValid())

	TSharedPtr<FOnlineUserPresence> OutPresence;
	Presence->GetCachedPresence(*TargetUser, OutPresence);

	return BlueprintableOnlineUserPresence(OutPresence->Status);
}

void UAccelByteCommonPresenceSubsystem::UpdateActivity(const EABActivityState ActivityState, const int32 LocalPlayerIndex)
{
	const FString PresenceString = ParsePresenceToString(ActivityStateMap[ActivityState], LocalPlayerIndex);

	// Set Presence
	SetPresence_Internal(PresenceString, LocalPlayerIndex);
}

void UAccelByteCommonPresenceSubsystem::PresenceInSessionDelegateSetup()
{
	const IOnlineSessionPtr Session = OSS->GetSessionInterface();
	check(Session.IsValid())

	// called at the start when hosting a P2P session
	Session->OnStartSessionCompleteDelegates.AddWeakLambda(this, [this](FName, bool)
	{
		UE_LOG(LogAccelByteCommonPresence, Log, TEXT("P2P Session started, set Presence Activity: InMatch"))
		UpdateActivity(EABActivityState::InMatch, 0);
	});

	// called when joining a P2P or a DS session
	Session->OnJoinSessionCompleteDelegates.AddWeakLambda(this, [this](FName, EOnJoinSessionCompleteResult::Type)
	{
		UE_LOG(LogAccelByteCommonPresence, Log, TEXT("P2P / DS Session joined, set Presence Activity: InMatch"))
		UpdateActivity(EABActivityState::InMatch, 0);
	});

	// called when quiting / ending a P2P or DS session
	Session->OnEndSessionCompleteDelegates.AddWeakLambda(this, [this](FName, bool)
	{
		UE_LOG(LogAccelByteCommonPresence, Log, TEXT("P2P / DS Session ended, set Presence Activity: Available"))
		UpdateActivity(EABActivityState::Available, 0);
	});
}

void UAccelByteCommonPresenceSubsystem::SetPresence_Internal(const FString& PresenceString, const int32 LocalPlayerIndex)
{
	const IOnlinePresencePtr Presence = OSS->GetPresenceInterface();
	check(Presence.IsValid())
	const IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
	check(Identity.IsValid())

	const FUniqueNetId& LocalUserId = *Identity->GetUniquePlayerId(LocalPlayerIndex);

	FOnlineUserPresenceStatus Status;
	Status.State = DesiredPresence;
	Status.StatusStr = PresenceString;

	Presence->SetPresence(LocalUserId, Status, IOnlinePresence::FOnPresenceTaskCompleteDelegate::CreateWeakLambda(this,
		[](const FUniqueNetId& /*UserId*/, const bool bWasSuccessful)
		{
			UE_LOG(LogAccelByteCommonPresence, Log, TEXT("SetPresence successful: %s"),
				bWasSuccessful? TEXT("TRUE") : TEXT("FALSE"));
		}));
}

FString UAccelByteCommonPresenceSubsystem::ParsePresenceToString (const FString& ActivityState, const int32 LocalPlayerIndex) const
{
	const IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
	check(Identity.IsValid())

	const FUniqueNetId& LocalUserId = *Identity->GetUniquePlayerId(LocalPlayerIndex);

	const TSharedRef<const FUniqueNetIdAccelByteUser> ABLocalUser = FUniqueNetIdAccelByteUser::Cast(LocalUserId);
	FString LoggedInPlatform = ABLocalUser->GetPlatformType();

	// password login will return empty string
	LoggedInPlatform = LoggedInPlatform == "" ? "PC" : LoggedInPlatform;

	FString ParsedStatusString = FString::Printf(TEXT("%s=%s,%s=%s"),
		*PresenceStatusStr_LoggedInPlatformKey, *LoggedInPlatform,
		*PresenceStatusStr_Activity, *ActivityState);

	return ParsedStatusString;
}

void UAccelByteCommonPresenceSubsystem::ParsePresenceFromString(
	FString& OutLoggedInPlatform,
	FString& OutActivityState,
	const FString& ActivityString)
{
	// Split array
	TArray<FString> SplitStrings;
	ActivityString.ParseIntoArray(SplitStrings, TEXT(","));

	OutLoggedInPlatform = ParsePresenceFromString_Helper(SplitStrings, PresenceStatusStr_LoggedInPlatformKey);
	OutActivityState = ParsePresenceFromString_Helper(SplitStrings, PresenceStatusStr_Activity);
}

FString UAccelByteCommonPresenceSubsystem::ParsePresenceFromString_Helper(
	TArray<FString>& ArrayStrings,
	const FString& Key,
	const bool bRemoveFoundFromArray)
{
	FString ResultString = "";
	for (uint8 i = 0; i < ArrayStrings.Num(); ++i)
	{
		FString LStr = "";
		FString RStr = "";
		ArrayStrings[i].Split("=", &LStr, &RStr);
		if (LStr == Key)
		{
			ResultString = RStr;
			if (bRemoveFoundFromArray)
			{
				// remove from array to make it faster to search the next value
				ArrayStrings.RemoveAt(i);
			}
			break;
		}
	}
	return ResultString;
}

FABOnlineUserPresence UAccelByteCommonPresenceSubsystem::BlueprintableOnlineUserPresence(
	const FOnlineUserPresenceStatus UserPresence)
{
	FString LoggedInPlatform = "";
	FString ActivityStateString = "";
	ParsePresenceFromString(LoggedInPlatform, ActivityStateString, UserPresence.StatusStr);

	// Available by default
	EABActivityState ActivityState = EABActivityState::Available;
	if (ActivityStateString == Activity_InMatch)
	{
		ActivityState = EABActivityState::InMatch;
	}

	return FABOnlineUserPresence(
		static_cast<EABOnlinePresenceState>(UserPresence.State),
		LoggedInPlatform,
		ActivityState);
}
