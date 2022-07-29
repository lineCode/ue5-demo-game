// Copyright (c) 2018 AccelByte, inc. All rights reserved.


#include "AccelByteCommonPartySubsystem.h"

#include "Online.h"
#include "OnlineSubsystemUtils.h"
#include "SocialManager.h"
#include "Core/AccelByteMultiRegistry.h"
#include "Messaging/CommonGameDialog.h"
#include "Party/SocialParty.h"

void UAccelByteCommonPartySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OSS = Online::GetSubsystem(GetWorld());

	SetPartyNotifDelegates();
}

void UAccelByteCommonPartySubsystem::GetPartyMember(
	const int32 LocalPlayerIndex,
	TArray<FABPartySubsystemPartyMember>& OutABPartyMembers,
	EPartyStatus& OutPartyStatus)
{
	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid());

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr.IsValid());

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);

		const FOnlinePartyConstPtr OnlineParty = PartyPtr->GetParty(*LocalUserId, PartyTypeId);

		if (OnlineParty.IsValid())
		{
			if (OnlineParty->State == EPartyState::Active)
			{
				// party for local player found
				TArray<FOnlinePartyMemberConstRef> PartyMembers;
				PartyPtr->GetPartyMembers(
					*LocalUserId,
					OnlineParty->PartyId.Get(),
					PartyMembers);

				const FUniqueNetIdPtr LeaderUserId = OnlineParty->LeaderId;

				OutABPartyMembers =
					BlueprintablePartyMembers(
						PartyMembers, LocalUserId.ToSharedRef(), LeaderUserId.ToSharedRef());
				OutPartyStatus = EPartyStatus::PartyValid;
			}
			else if (
				OnlineParty->State == EPartyState::CreatePending ||
				OnlineParty->State == EPartyState::JoinPending)
			{
				OutPartyStatus = EPartyStatus::PartyDataLoading;
			}
			else
			{
				OutPartyStatus = EPartyStatus::NoParty;
			}
			return;
		}
		// no party for local user found
		OutPartyStatus = EPartyStatus::NoParty;
	}
}

FABPartySubsystemPartyMember UAccelByteCommonPartySubsystem::GetPartyMemberByAccelByteIdString(
	FString AccelByteIdString, int32 LocalPlayerIndex)
{
	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid());

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr.IsValid());

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);

		const FOnlinePartyConstPtr OnlineParty = PartyPtr->GetParty(*LocalUserId, PartyTypeId);

		if (OnlineParty.IsValid())
		{
			// GetPartyMember doesn't work with UniqueIdString since AB OSS also need the composite structure
			TArray<FOnlinePartyMemberConstRef> PartyMembers;
			PartyPtr->GetPartyMembers(
				*LocalUserId,
				OnlineParty->PartyId.Get(),
				PartyMembers);
			for (FOnlinePartyMemberConstRef& PartyMember : PartyMembers)
			{
				const TSharedRef<const FUniqueNetIdAccelByteUser> ABUser =
					FUniqueNetIdAccelByteUser::Cast(*PartyMember->GetUserId());

				if (ABUser->GetAccelByteId() == AccelByteIdString)
				{
					const FUniqueNetIdPtr LeaderUserId = OnlineParty->LeaderId;
					return BlueprintablePartyMembers(
						{PartyMember},
						LocalUserId.ToSharedRef(),
						LeaderUserId.ToSharedRef())[0];
				}
			}
		}
	}

	return FABPartySubsystemPartyMember();
}

FString UAccelByteCommonPartySubsystem::GetLocalPlayerAccelByteIdString(const int32 LocalPlayerIndex) const
{
	FString IdString = "";

	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid());

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);
		const TSharedRef<const FUniqueNetIdAccelByteUser> ABUser = FUniqueNetIdAccelByteUser::Cast(*LocalUserId);

		IdString = ABUser->GetAccelByteId();
	}

	return IdString;
}

FUniqueNetIdRepl UAccelByteCommonPartySubsystem::GetPartyLeaderIdIfPartyExist(
	bool& OutbIsPartyExist, const int32 LocalPlayerIndex)
{
	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid());

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);

		const FOnlinePartyConstPtr OnlineParty = PartyPtr->GetParty(*LocalUserId, PartyTypeId);

		if (OnlineParty.IsValid())
		{
			OutbIsPartyExist = true;
			return OnlineParty->LeaderId;
		}
	}
	OutbIsPartyExist = false;
	return nullptr;
}

void UAccelByteCommonPartySubsystem::InviteToPartyIfPartyExist(FUniqueNetIdRepl TargetUniqueId, bool& OutbIsPartyExist, const int32 LocalPlayerIndex)
{
	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid());

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);

		const FOnlinePartyConstPtr OnlineParty = PartyPtr->GetParty(*LocalUserId, PartyTypeId);

		if (OnlineParty.IsValid())
		{
			PartyPtr->SendInvitation(*LocalUserId, *OnlineParty->PartyId,*TargetUniqueId.GetUniqueNetId());
			OutbIsPartyExist = true;
			return;
		}
	}
	OutbIsPartyExist = false;
}

void UAccelByteCommonPartySubsystem::KickFromPartyIfPartyExist(FUniqueNetIdRepl TargetUniqueId, bool& OutbIsPartyExist, const int32 LocalPlayerIndex)
{
	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid());

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);

		const FOnlinePartyConstPtr OnlineParty = PartyPtr->GetParty(*LocalUserId, PartyTypeId);

		if (OnlineParty.IsValid())
		{
			PartyPtr->KickMember(*LocalUserId, *OnlineParty->PartyId,*TargetUniqueId.GetUniqueNetId());
			OutbIsPartyExist = true;
			return;
		}
	}
	OutbIsPartyExist = false;
}

void UAccelByteCommonPartySubsystem::PromoteAsLeaderIfPartyExist(FUniqueNetIdRepl TargetUniqueId, bool& OutbIsPartyExist, const int32 LocalPlayerIndex)
{
	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid());

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);

		const FOnlinePartyConstPtr OnlineParty = PartyPtr->GetParty(*LocalUserId, PartyTypeId);

		if (OnlineParty.IsValid())
		{
			PartyPtr->PromoteMember(*LocalUserId, *OnlineParty->PartyId,*TargetUniqueId.GetUniqueNetId());
			OutbIsPartyExist = true;
			return;
		}
	}
	OutbIsPartyExist = false;
}

void UAccelByteCommonPartySubsystem::LeavePartyIfInParty(bool& OutbWasInParty, const FPartyVoidDelegate& OnComplete, int32 LocalPlayerIndex, int32 NewPartyMemberLimit)
{
	OutbWasInParty = false;

	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid());

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);

		const FOnlinePartyConstPtr OnlineParty = PartyPtr->GetParty(*LocalUserId, PartyTypeId);

		if (OnlineParty.IsValid())
		{
			PartyPtr->LeaveParty(*LocalUserId, *OnlineParty->PartyId, FOnLeavePartyComplete::CreateWeakLambda(this,
				[this, LocalPlayerIndex, NewPartyMemberLimit, OnComplete](
					const FUniqueNetId& LocalUserId,
					const FOnlinePartyId& PartyId,
					const ELeavePartyCompletionResult Result)
				{
					if (ShouldAutoCreateParty())
					{
						UE_LOG(LogAccelByteCommonParty, Log, TEXT("bAutoCreateParty true: creating party"));
						CreateParty(LocalPlayerIndex, TDelegate<void()>::CreateWeakLambda(this, [OnComplete]()
						{
							OnComplete.ExecuteIfBound();
						}), NewPartyMemberLimit);
					}
					else
					{
						OnComplete.ExecuteIfBound();
					}
				}));
			OutbWasInParty = true;
		}
	}
}

void UAccelByteCommonPartySubsystem::CreatePartyIfNotExist(bool& OutbWasNotInParty, int32 LocalPlayerIndex, int32 NewPartyMemberLimit)
{
	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid());

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);

		const FOnlinePartyConstPtr OnlineParty = PartyPtr->GetParty(*LocalUserId, PartyTypeId);

		if (!OnlineParty.IsValid())
		{
			CreateParty(LocalPlayerIndex, TDelegate<void()>(), NewPartyMemberLimit);
			OutbWasNotInParty = true;
			return;
		}
	}
	OutbWasNotInParty = false;
}

void UAccelByteCommonPartySubsystem::SetPartyNotifDelegates(int32 LocalPlayerIndex)
{
	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr);

		// Called on join another party
		PartyPtr->OnPartyJoinedDelegates.AddWeakLambda(this,
			[this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId)
			{
				UE_LOG(LogAccelByteCommonParty, Log, TEXT("Local joined party"));
				OnPartyJoinedDelegate.Broadcast();
			});

		// Called when user joined party
		PartyPtr->OnPartyMemberJoinedDelegates.AddWeakLambda(this,
			[this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId)
			{
				UE_LOG(LogAccelByteCommonParty, Log, TEXT("Other player Joined party"));
				OnPartyInfoChangedDelegate.Broadcast();
			});

		// Called on left party
		PartyPtr->OnPartyExitedDelegates.AddWeakLambda(this,
			[this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId)
			{
				UE_LOG(LogAccelByteCommonParty, Log, TEXT("Local user left party"));
				OnPartyInfoChangedDelegate.Broadcast();
			});

		// Called on member left party
		PartyPtr->OnPartyMemberExitedDelegates.AddWeakLambda(this,
			[this](
				const FUniqueNetId& /*LocalUserId*/,
				const FOnlinePartyId& /*PartyId*/,
				const FUniqueNetId& /*MemberId*/,
				const EMemberExitedReason /*Reason*/)
			{
				UE_LOG(LogAccelByteCommonParty, Log, TEXT("Party member exited"));
				OnPartyInfoChangedDelegate.Broadcast();
			});

		// Called on member successfully promoted
		PartyPtr->OnPartyMemberPromotedDelegates.AddWeakLambda(this,
			[this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& NewLeaderId)
			{
				UE_LOG(LogAccelByteCommonParty, Log, TEXT("Party member promoted"));
				OnPartyInfoChangedDelegate.Broadcast();
			});

		// Called upon party data change
		PartyPtr->OnPartyDataReceivedDelegates.AddWeakLambda(this,
			[this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FName& Namespace, const FOnlinePartyData& PartyData)
			{
				UE_LOG(LogAccelByteCommonParty, Log, TEXT("Party Data Received"));
				OnPartyDataChangedDelegate.Broadcast();
			});

		const IOnlineFriendsPtr FriendsPtr = OSS->GetFriendsInterface();
		check(FriendsPtr);

		// Called upon receiving party invite
		PartyPtr->OnPartyInviteReceivedDelegates.AddWeakLambda(this,
			[LocalPlayerIndex, FriendsPtr, this](
				const FUniqueNetId& LocalUserId,
				const FOnlinePartyId& PartyId,
				const FUniqueNetId& SenderId)
			{
				// prevent confirmation menu to be shown multiple times
				if (!bIsPartyInviteConfirmationShown)
				{
					const FABPartySubsystemPartyMember Sender(
						FUniqueNetIdRepl(SenderId),
						FriendsPtr->GetFriend(LocalPlayerIndex, SenderId, "")->GetDisplayName(),
						&PartyId);

					ShowReceivedInvitePopup(this, Sender, LocalPlayerIndex);
				}
			});
	}
}

void UAccelByteCommonPartySubsystem::AcceptPartyInvite(FUniqueNetIdRepl SenderUniqueId, int32 LocalPlayerIndex)
{
	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid());

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);

		const FOnlinePartyConstPtr OnlineParty = PartyPtr->GetParty(*LocalUserId, PartyTypeId);

		if (OnlineParty.IsValid())
		{
			PartyPtr->LeaveParty(*LocalUserId, *OnlineParty->PartyId, FOnLeavePartyComplete::CreateWeakLambda(this,
				[PartyPtr, SenderUniqueId, LocalUserId, this](
					const FUniqueNetId& ResultLocalUserId,
					const FOnlinePartyId& PartyId,
					const ELeavePartyCompletionResult Result)
				{
					AcceptInviteRequest(PartyPtr, LocalUserId, SenderUniqueId.GetUniqueNetId());
				}));
		}
		else
		{
			AcceptInviteRequest(PartyPtr, LocalUserId, SenderUniqueId.GetUniqueNetId());
		}
	}
}

void UAccelByteCommonPartySubsystem::RejectPartyInvite(FUniqueNetIdRepl SenderUniqueId, int32 LocalPlayerIndex)
{
	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid())

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);

		PartyPtr->RejectInvitation(*LocalUserId, *SenderUniqueId.GetUniqueNetId());
	}
}

bool UAccelByteCommonPartySubsystem::IsLocalUserLeader(int32 LocalPlayerIndex) const
{
	bool bIsLocalUserLeader = false;

	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid());

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);

		const FOnlinePartyConstPtr OnlineParty = PartyPtr->GetParty(*LocalUserId, PartyTypeId);

		if (OnlineParty.IsValid())
		{
			const TSharedRef<const FUniqueNetIdAccelByteUser> ABLocalUser =
					FUniqueNetIdAccelByteUser::Cast(*LocalUserId);

			const TSharedRef<const FUniqueNetIdAccelByteUser> ABLeaderUser =
					FUniqueNetIdAccelByteUser::Cast(*OnlineParty->LeaderId);

			// Native unique user id doesn't always work
			bIsLocalUserLeader = ABLocalUser->GetAccelByteId() == ABLeaderUser->GetAccelByteId();
		}
	}

	return bIsLocalUserLeader;
}

bool UAccelByteCommonPartySubsystem::ShouldAutoCreateParty()
{
	bool bAutoCreateParty = false;
	GConfig->GetBool(TEXT("AccelByteSocialToolkit"), TEXT("bAutoCreateParty"), bAutoCreateParty, GEngineIni);
	return bAutoCreateParty;
}

int32 UAccelByteCommonPartySubsystem::GetPartyMemberMax(const int32 LocalPlayerIndex, EPartyMatchType PartyMatchType)
{
	int32 MaxPartyMembers = 0;

	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid());

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);

		const FOnlinePartyConstPtr OnlineParty = PartyPtr->GetParty(*LocalUserId, PartyTypeId);

		if (OnlineParty.IsValid())
		{
			MaxPartyMembers = OnlineParty->GetConfiguration()->MaxMembers;

			/**
			 * Due to an issue in AB OSS, the joining member will still have their configured max party member as 0
			 * This is used as a workaround for that.
			 */
			if (MaxPartyMembers > 0)
			{
				return MaxPartyMembers;
			}

			MaxPartyMembers = GetPartyMemberLimitPreset(PartyMatchType);
		}
	}
	return MaxPartyMembers;
}

int32 UAccelByteCommonPartySubsystem::GetPartyMemberLimitPreset(EPartyMatchType PartyMatchType)
{
	int32 MaxPartyMembers = 0;

	switch (PartyMatchType)
	{
	case EPartyMatchType::QuickMatch:
		GConfig->GetInt(TEXT("AccelByteSocialToolkit"), TEXT("MaxPartyMembers"), MaxPartyMembers, GEngineIni);
		break;
	case EPartyMatchType::CustomSession:
		GConfig->GetInt(TEXT("AccelByteSocialToolkit"), TEXT("MaxPartyMembers_CustomSession"), MaxPartyMembers, GEngineIni);
		break;
	}

	return MaxPartyMembers;
}

void UAccelByteCommonPartySubsystem::SetPartyDataString(int32 LocalPlayerIndex, FString PartyAttrName, FString PartyAttrValue)
{
	SetPartyData(LocalPlayerIndex, {{PartyAttrName, PartyAttrValue}});
}

FString UAccelByteCommonPartySubsystem::SetPartyDataArrayOfString(int32 LocalPlayerIndex, FString PartyAttrName,
	TArray<FString> PartyAttrValues, const bool bAppend, const bool bUpdateImmediately)
{
	FString ArrayValuesInString = "";

	if (bAppend)
	{
		ArrayValuesInString = GetCachedPartyDataString(LocalPlayerIndex, PartyAttrName);
	}

	ArrayValuesInString += SetPartyDataArrayOfString_Helper(PartyAttrValues);

	if (bUpdateImmediately)
	{
		SetPartyData(LocalPlayerIndex, {{PartyAttrName, ArrayValuesInString}});
	}

	return ArrayValuesInString;
}

FString UAccelByteCommonPartySubsystem::GetLocalPlayerTeam(int32 LocalPlayerIndex) const
{
	FString Team = "";

	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid());

		const FString LocalAccelByteIdString = GetLocalPlayerAccelByteIdString(LocalPlayerIndex);

		if (GetCachedPartyDataString(LocalPlayerIndex, PartyAttrName_CustomSession_Team1).Find(LocalAccelByteIdString) != -1)
		{
			Team = PartyAttrName_CustomSession_Team1;
		}
		else if (GetCachedPartyDataString(LocalPlayerIndex, PartyAttrName_CustomSession_Team2).Find(LocalAccelByteIdString) != -1)
		{
			Team = PartyAttrName_CustomSession_Team2;
		}
		else if (GetCachedPartyDataString(LocalPlayerIndex, PartyAttrName_CustomSession_Observer).Find(LocalAccelByteIdString) != -1)
		{
			Team = PartyAttrName_CustomSession_Observer;
		}
	}

	return Team;
}

void UAccelByteCommonPartySubsystem::ChangeLocalPlayerTeamToNextTeam(int32 LocalPlayerIndex)
{
	if (OSS)
	{
		const FString LocalAccelByteIdString = GetLocalPlayerAccelByteIdString(LocalPlayerIndex);
		const FString LocalUserTeam = GetLocalPlayerTeam(LocalPlayerIndex);

		TMap<FString, FString> TempData;
		if (LocalUserTeam == PartyAttrName_CustomSession_Team1)
		{
			const FString RemoveResult = RemoveStringFromPartyDataArrayOfString(
				LocalPlayerIndex, PartyAttrName_CustomSession_Team1, LocalAccelByteIdString, false);
			TempData.Add(PartyAttrName_CustomSession_Team1, RemoveResult);

			const FString AppendResult = SetPartyDataArrayOfString(
				LocalPlayerIndex, PartyAttrName_CustomSession_Team2, {LocalAccelByteIdString}, true, false);
			TempData.Add(PartyAttrName_CustomSession_Team2, AppendResult);
		}
		else if (LocalUserTeam == PartyAttrName_CustomSession_Team2)
		{
			const FString RemoveResult = RemoveStringFromPartyDataArrayOfString(
				LocalPlayerIndex, PartyAttrName_CustomSession_Team2, LocalAccelByteIdString, false);
			TempData.Add(PartyAttrName_CustomSession_Team2, RemoveResult);

			const FString AppendResult = SetPartyDataArrayOfString(
				LocalPlayerIndex, PartyAttrName_CustomSession_Observer, {LocalAccelByteIdString}, true, false);
			TempData.Add(PartyAttrName_CustomSession_Observer, AppendResult);
		}
		else
		{
			const FString RemoveResult = RemoveStringFromPartyDataArrayOfString(
				LocalPlayerIndex, PartyAttrName_CustomSession_Observer, LocalAccelByteIdString, false);
			TempData.Add(PartyAttrName_CustomSession_Observer, RemoveResult);

			const FString AppendResult = SetPartyDataArrayOfString(
				LocalPlayerIndex, PartyAttrName_CustomSession_Team1, {LocalAccelByteIdString}, true, false);
			TempData.Add(PartyAttrName_CustomSession_Team1, AppendResult);
		}
		SetPartyData(LocalPlayerIndex, TempData);
	}
}

void UAccelByteCommonPartySubsystem::ChangeLocalPlayerTeamToPreviousTeam(int32 LocalPlayerIndex)
{
	if (OSS)
	{
		const FString LocalAccelByteIdString = GetLocalPlayerAccelByteIdString(LocalPlayerIndex);
		const FString LocalUserTeam = GetLocalPlayerTeam(LocalPlayerIndex);

		TMap<FString, FString> TempData;
		if (LocalUserTeam == PartyAttrName_CustomSession_Team1)
		{
			const FString RemoveResult = RemoveStringFromPartyDataArrayOfString(
				LocalPlayerIndex, PartyAttrName_CustomSession_Team1, LocalAccelByteIdString, false);
			TempData.Add(PartyAttrName_CustomSession_Team1, RemoveResult);

			const FString AppendResult = SetPartyDataArrayOfString(
				LocalPlayerIndex, PartyAttrName_CustomSession_Observer, {LocalAccelByteIdString}, true, false);
			TempData.Add(PartyAttrName_CustomSession_Observer, AppendResult);
		}
		else if (LocalUserTeam == PartyAttrName_CustomSession_Team2)
		{
			const FString RemoveResult = RemoveStringFromPartyDataArrayOfString(
				LocalPlayerIndex, PartyAttrName_CustomSession_Team2, LocalAccelByteIdString, false);
			TempData.Add(PartyAttrName_CustomSession_Team2, RemoveResult);

			const FString AppendResult = SetPartyDataArrayOfString(
				LocalPlayerIndex, PartyAttrName_CustomSession_Team1, {LocalAccelByteIdString}, true, false);
			TempData.Add(PartyAttrName_CustomSession_Team1, AppendResult);
		}
		else
		{
			const FString RemoveResult = RemoveStringFromPartyDataArrayOfString(
				LocalPlayerIndex, PartyAttrName_CustomSession_Observer, LocalAccelByteIdString, false);
			TempData.Add(PartyAttrName_CustomSession_Observer, RemoveResult);

			const FString AppendResult = SetPartyDataArrayOfString(
				LocalPlayerIndex, PartyAttrName_CustomSession_Team2, {LocalAccelByteIdString}, true, false);
			TempData.Add(PartyAttrName_CustomSession_Team2, AppendResult);
		}
		SetPartyData(LocalPlayerIndex, TempData);
	}
}

FString UAccelByteCommonPartySubsystem::RemoveStringFromPartyDataArrayOfString(
	int32 LocalPlayerIndex,
	FString PartyAttrName,
	FString PartyAttrValue,
	bool bUpdateImmediately)
{
	FString ArrayValuesInString = GetCachedPartyDataString(LocalPlayerIndex, PartyAttrName);
	ArrayValuesInString = ArrayValuesInString.Replace(*(PartyAttrValue + ","), TEXT(""), ESearchCase::CaseSensitive);

	if (bUpdateImmediately)
	{
		SetPartyDataString(LocalPlayerIndex, PartyAttrName, ArrayValuesInString);
	}

	return ArrayValuesInString;
}

FString UAccelByteCommonPartySubsystem::GetCachedPartyDataString(int32 LocalPlayerIndex, FString PartyAttrName) const
{
	FString DataString = "";

	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid())

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);
		if(LocalUserId.IsValid())
		{
			const FOnlinePartyConstPtr OnlineParty = PartyPtr->GetParty(*LocalUserId, PartyTypeId);
	
			if (OnlineParty.IsValid())
			{
				const FOnlinePartyDataConstPtr PartyData = PartyPtr->GetPartyData(*LocalUserId, *OnlineParty->PartyId, DefaultPartyDataNamespace);
				FVariantData VariantData;
				PartyData->GetAttribute(PartyAttrName, VariantData);
	
				DataString = VariantData.ToString();
			}
		}
	}

	return DataString;
}

TArray<FString> UAccelByteCommonPartySubsystem::GetCachedPartyDataArrayOfString(int32 LocalPlayerIndex,
	FString PartyAttrName)
{
	const FString ValueString = GetCachedPartyDataString(LocalPlayerIndex, PartyAttrName);
	TArray<FString> Values;

	ValueString.ParseIntoArray(Values, TEXT(","));
	return Values;
}

void UAccelByteCommonPartySubsystem::CreateParty(int32 LocalPlayerIndex, TDelegate<void()> OnComplete, int32 NewPartyMemberLimit)
{
	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr);

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);

		FPartyConfiguration Config;
		Config.bIsAcceptingMembers = true;

		Config.MaxMembers = NewPartyMemberLimit;

		PartyPtr->CreateParty(
			*LocalUserId,
			PartyTypeId,
			Config,
			FOnCreatePartyComplete::CreateWeakLambda(this,
				[OnComplete, this](
					const FUniqueNetId& LocalUserId,
					const TSharedPtr<const FOnlinePartyId>& PartyId,
					const ECreatePartyCompletionResult Result)
				{
					OnComplete.ExecuteIfBound();
				}));
	}
}

void UAccelByteCommonPartySubsystem::ShowReceivedInvitePopup(
	const UObject* WorldContextObject,
	FABPartySubsystemPartyMember Sender,
	int32 LocalPlayerIndex)
{
	const ULocalPlayer* TargetLocalPlayer =
		WorldContextObject->GetWorld()->GetGameInstance()->GetPrimaryPlayerController(false)->GetLocalPlayer();

	const FText Header = FText::FromString("Party Invitation");
	const FText Body =
		FText::FromString(Sender.UserInfo.DisplayName +
		" sent party invitation" + LINE_TERMINATOR + "Would you like to accept?");

	UCommonGameDialogDescriptor* Descriptor = UCommonGameDialogDescriptor::CreateConfirmationYesNo(Header, Body);

	if (TargetLocalPlayer)
	{
		if (UCommonMessagingSubsystem* Messaging = TargetLocalPlayer->GetSubsystem<UCommonMessagingSubsystem>())
		{
			const FCommonMessagingResultDelegate ResultCallback = FCommonMessagingResultDelegate::CreateWeakLambda(this,
				[Sender, this, LocalPlayerIndex](ECommonMessagingResult Result)
				{
					switch (Result)
					{
					case ECommonMessagingResult::Confirmed:
						OnAcceptPartyInvitationDelegate.Broadcast();
						AcceptPartyInvite(Sender.UserInfo.UserId, LocalPlayerIndex);
						break;
					default:
						RejectPartyInvite(Sender.UserInfo.UserId, LocalPlayerIndex);
						break;
					}
					bIsPartyInviteConfirmationShown = false;
				});
			bIsPartyInviteConfirmationShown = true;
			Messaging->ShowConfirmation(Descriptor, ResultCallback);
		}
	}
}

FABPartySubsystemPartyMember UAccelByteCommonPartySubsystem::BlueprintablePartyMember(
	const FOnlinePartyMemberConstRef PartyMember)
{
	return FABPartySubsystemPartyMember(
		PartyMember->GetUserId(),
		PartyMember->GetDisplayName(),
		false);
}

TArray<FABPartySubsystemPartyMember> UAccelByteCommonPartySubsystem::BlueprintablePartyMembers(
	const TArray<FOnlinePartyMemberConstRef>& PartyMembers, FUniqueNetIdRef LocalUserId, FUniqueNetIdRef LeaderUserId)
{
	TArray<FABPartySubsystemPartyMember> ABPartyMembers;
	for (const FOnlinePartyMemberConstRef& PartyMember : PartyMembers)
	{
		FABPartySubsystemPartyMember ABPartyMember = FABPartySubsystemPartyMember(
			PartyMember->GetUserId(),
			PartyMember->GetDisplayName(),
			false);
		ABPartyMember.bIsLocalUser = ABPartyMember.UserInfo.UserId == LocalUserId;
		ABPartyMember.bIsLeader = ABPartyMember.UserInfo.UserId == LeaderUserId;
		ABPartyMembers.Add(ABPartyMember);
	}
	return ABPartyMembers;
}

void UAccelByteCommonPartySubsystem::AcceptInviteRequest(
	IOnlinePartyPtr PartyPtr,
	FUniqueNetIdPtr LocalUserUniqueId,
	FUniqueNetIdPtr SenderUniqueId)
{
	IOnlinePartyJoinInfoConstPtr TargetPartyJoinInfo;
	TArray<IOnlinePartyJoinInfoConstRef> PartyJoinInfos;
	PartyPtr->GetPendingInvites(
		*LocalUserUniqueId,
		PartyJoinInfos);
	for (const IOnlinePartyJoinInfoConstRef& PartyJoinInfo : PartyJoinInfos)
	{
		if (PartyJoinInfo->GetSourceUserId() == SenderUniqueId)
		{
			TargetPartyJoinInfo = PartyJoinInfo;
		}
	}
	PartyPtr->JoinParty(*LocalUserUniqueId, *TargetPartyJoinInfo);
}

void UAccelByteCommonPartySubsystem::SetPartyData(int32 LocalPlayerIndex, TMap<FString, FString> Datas) const
{
	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr);

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);

		const FOnlinePartyConstPtr OnlineParty = PartyPtr->GetParty(*LocalUserId, PartyTypeId);

		if (OnlineParty.IsValid())
		{
			FOnlinePartyData PartyData;

			for (TTuple<FString, FString>& Data : Datas)
			{
				PartyData.SetAttribute(Data.Key, Data.Value);
			}

			PartyPtr->UpdatePartyData(*LocalUserId, *OnlineParty->PartyId, DefaultPartyDataNamespace, PartyData);
		}
	}
}

FString UAccelByteCommonPartySubsystem::SetPartyDataArrayOfString_Helper(TArray<FString> InArray)
{
	FString Result = "";
	for (FString& Value : InArray)
	{
		Result += Value + ",";
	}
	return Result;
}
