// Copyright (c) 2018 AccelByte, inc. All rights reserved.


#include "AccelByteCommonPartySubsystem.h"

#include "Online.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AccelByteMultiRegistry.h"
#include "Messaging/CommonGameDialog.h"
#include "Party/SocialParty.h"

void UAccelByteCommonPartySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OSS = Online::GetSubsystem(GetWorld());

	SetPartyNotifDelegates();

	// OnLobbyConnected sequence
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(OSS->GetIdentityInterface());
	check(IdentityInterface);
	IdentityInterface->AddOnConnectLobbyCompleteDelegate_Handle(0, FOnConnectLobbyCompleteDelegate::CreateWeakLambda(
		this, [this](int32 LocalUserNum, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const FString& /*Error*/)
		{
			// query friends, needed for the OnPartyInvite popup to work
			const IOnlineFriendsPtr OnlineFriends = OSS->GetFriendsInterface();
			check(OnlineFriends);
			OnlineFriends->ReadFriendsList(LocalUserNum, "");

			// create party if bAutoCreateParty config is true
			if (ShouldAutoCreateParty())
			{
				CreateParty(LocalUserNum);
			}
		}));
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
					LeaveParty_Helper(OnComplete, NewPartyMemberLimit, LocalPlayerIndex);
				}));
			OutbWasInParty = true;
		}
		else
		{
			LeaveParty_Helper(OnComplete, NewPartyMemberLimit, LocalPlayerIndex);
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
					const TSharedPtr<FOnlineFriend> Friend = FriendsPtr->GetFriend(LocalPlayerIndex, SenderId, "");
					if (Friend.IsValid())
					{
						const FABPartySubsystemPartyMember Sender(
							FUniqueNetIdRepl(SenderId),
							FriendsPtr->GetFriend(LocalPlayerIndex, SenderId, "")->GetDisplayName(),
							&PartyId);

						ShowReceivedInvitePopup(this, Sender, LocalPlayerIndex);
					}
					else
					{
						UE_LOG(LogAccelByteCommonParty, Warning, TEXT("Query friend is not yet finished. Skipping this invite"));
					}
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
	GConfig->GetBool(TEXT("AccelByteSocial"), TEXT("bAutoCreateParty"), bAutoCreateParty, GEngineIni);
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

int32 UAccelByteCommonPartySubsystem::GetPartyMemberLimitPreset(EPartyMatchType PartyMatchType) const
{
	int32 MaxPartyMembers = 0;

	switch (PartyMatchType)
	{
	case EPartyMatchType::QuickMatch:
		GConfig->GetInt(TEXT("AccelByteSocial"), TEXT("MaxPartyMembers"), MaxPartyMembers, GEngineIni);
		break;
	case EPartyMatchType::CustomSession:
		GConfig->GetInt(TEXT("AccelByteSocial"), TEXT("MaxPartyMembers_CustomSession"), MaxPartyMembers, GEngineIni);
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

ECustomSessionTeam UAccelByteCommonPartySubsystem::GetLocalPlayerTeam(int32 LocalPlayerIndex) const
{
	ECustomSessionTeam Team = ECustomSessionTeam::NoTeam;

	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid());

		const FString LocalAccelByteIdString = GetLocalPlayerAccelByteIdString(LocalPlayerIndex);

		const FString Team1String = GetCachedPartyDataString(LocalPlayerIndex, PartyAttrName_CustomSession_Team1);
		const FString Team2String = GetCachedPartyDataString(LocalPlayerIndex, PartyAttrName_CustomSession_Team2);
		const FString QueueString = GetCachedPartyDataString(LocalPlayerIndex, PartyAttrName_CustomSession_Queue);

		if (Team1String.Find(LocalAccelByteIdString) != -1)
		{
			Team = ECustomSessionTeam::Team1;
		}
		else if (Team2String.Find(LocalAccelByteIdString) != -1)
		{
			Team = ECustomSessionTeam::Team2;
		}
		else if (QueueString.Find(LocalAccelByteIdString) != -1)
		{
			Team = ECustomSessionTeam::Queue;
		}
	}

	return Team;
}

void UAccelByteCommonPartySubsystem::CycleLocalPlayerTeam(bool CycleNext, int32 LocalPlayerIndex)
{
	if (OSS)
	{
		const FString LocalAccelByteIdString = GetLocalPlayerAccelByteIdString(LocalPlayerIndex);
		const ECustomSessionTeam LocalUserTeamOld = GetLocalPlayerTeam(LocalPlayerIndex);
		const int32 LocalUserTeamOldIndex = static_cast<int32>(LocalUserTeamOld);
		const int32 MaxMemberPerTeam = GetPartyMemberLimitPreset(EPartyMatchType::CustomSession) / 2;
		const int32 Dif = CycleNext? 1 : -1;
		constexpr int32 MaxTeamIndex = static_cast<int32>(ECustomSessionTeam::Max);

		TMap<FString, FString> TempData;

		// cycle logic
		ECustomSessionTeam LocalUserTeamNew = LocalUserTeamOld;
		int32 LocalUserTeamNewIndex = LocalUserTeamOldIndex;
		bool bIsInValidTeam = false;
		while (!bIsInValidTeam)
		{
			// increment / decrement team
			LocalUserTeamNewIndex = PositiveModulo(LocalUserTeamNewIndex + Dif, MaxTeamIndex);
			LocalUserTeamNew = static_cast<ECustomSessionTeam>(LocalUserTeamNewIndex);

			// check if target team member valid
			if (LocalUserTeamNew == ECustomSessionTeam::NoTeam || LocalUserTeamNew == ECustomSessionTeam::Max)
			{
				bIsInValidTeam = false;
				continue;
			}

			// check if target team member full
			if (GetTeamMembersNum(LocalUserTeamNew, LocalPlayerIndex) >= MaxMemberPerTeam)
			{
				bIsInValidTeam = false;
				continue;
			}

			bIsInValidTeam = true;
		}

		// update party data
		if (GetLocalPlayerTeam(LocalPlayerIndex) != ECustomSessionTeam::NoTeam)
		{
			const FString RemoveResult = RemoveStringFromPartyDataArrayOfString(
				LocalPlayerIndex,
				PartyAttr_CustomSession_Team[LocalUserTeamOld],
				LocalAccelByteIdString,
				false);
			TempData.Add(PartyAttr_CustomSession_Team[LocalUserTeamOld], RemoveResult);
		}

		const FString AppendResult = SetPartyDataArrayOfString(
			LocalPlayerIndex,
			PartyAttr_CustomSession_Team[LocalUserTeamNew],
			{LocalAccelByteIdString},
			true, false);
		TempData.Add(PartyAttr_CustomSession_Team[LocalUserTeamNew], AppendResult);

		SetPartyData(LocalPlayerIndex, TempData);
	}
}

int32 UAccelByteCommonPartySubsystem::GetTeamMembersNum(ECustomSessionTeam Team, const int32 LocalPlayerIndex) const
{
	const TArray<FString> TeamMember = GetCachedPartyDataArrayOfString(LocalPlayerIndex, PartyAttr_CustomSession_Team[Team]);
	return TeamMember.Num();
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

	UE_LOG(LogAccelByteCommonParty, Log, TEXT("Cached party data:: Key: %s | Value: %s"), *PartyAttrName, *DataString);

	return DataString;
}

TArray<FString> UAccelByteCommonPartySubsystem::GetCachedPartyDataArrayOfString(
	int32 LocalPlayerIndex,
	FString PartyAttrName) const
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
			OnPartyDataSetDelegate.Broadcast();
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

int32 UAccelByteCommonPartySubsystem::PositiveModulo(const int32 i, const int32 n)
{
	return ((i % n) + n) % n;
}

void UAccelByteCommonPartySubsystem::LeaveParty_Helper(const FPartyVoidDelegate& OnComplete, const int32 NewPartyMemberLimit, const int32 LocalPlayerIndex)
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
}
