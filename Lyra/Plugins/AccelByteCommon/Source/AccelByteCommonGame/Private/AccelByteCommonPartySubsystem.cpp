// Copyright (c) 2018 AccelByte, inc. All rights reserved.


#include "AccelByteCommonPartySubsystem.h"

#include "Online.h"
#include "OnlineSubsystemUtils.h"
#include "SocialManager.h"
#include "Party/SocialParty.h"

void UAccelByteCommonPartySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OSS = Online::GetSubsystem(GetWorld());
}

void UAccelByteCommonPartySubsystem::GetPartyMember(
	const int32 LocalPlayerIndex,
	TArray<FABPartySubsystemPartyMember>& ABPartyMembers,
	EPartyStatus& PartyStatus)
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

				ABPartyMembers =
					BlueprintablePartyMembers(
						PartyMembers, LocalUserId.ToSharedRef(), LeaderUserId.ToSharedRef());
				PartyStatus = EPartyStatus::PartyValid;
			}
			else if (
				OnlineParty->State == EPartyState::CreatePending ||
				OnlineParty->State == EPartyState::JoinPending)
			{
				PartyStatus = EPartyStatus::PartyDataLoading;
			}
			else
			{
				PartyStatus = EPartyStatus::NoParty;
			}
			return;
		}
		// no party for local user found
		PartyStatus = EPartyStatus::NoParty;
	}
}

FUniqueNetIdRepl UAccelByteCommonPartySubsystem::GetPartyLeaderIdIfPartyExist(bool& bIsPartyExist, const int32 LocalPlayerIndex)
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
			bIsPartyExist = true;
			return OnlineParty->LeaderId;
		}
	}
	bIsPartyExist = false;
	return nullptr;
}

void UAccelByteCommonPartySubsystem::InviteToPartyIfPartyExist(FUniqueNetIdRepl TargetUniqueId, bool& bIsPartyExist, const int32 LocalPlayerIndex)
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
			bIsPartyExist = true;
			return;
		}
	}
	bIsPartyExist = false;
}

void UAccelByteCommonPartySubsystem::KickFromPartyIfPartyExist(FUniqueNetIdRepl TargetUniqueId, bool& bIsPartyExist, const int32 LocalPlayerIndex)
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
			bIsPartyExist = true;
			return;
		}
	}
	bIsPartyExist = false;
}

void UAccelByteCommonPartySubsystem::PromoteAsLeaderIfPartyExist(FUniqueNetIdRepl TargetUniqueId, bool& bIsPartyExist, const int32 LocalPlayerIndex)
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
			bIsPartyExist = true;
			return;
		}
	}
	bIsPartyExist = false;
}

void UAccelByteCommonPartySubsystem::LeavePartyIfInParty(bool& bWasInParty, int32 LocalPlayerIndex)
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
				[this, LocalPlayerIndex](
					const FUniqueNetId& LocalUserId,
					const FOnlinePartyId& PartyId,
					const ELeavePartyCompletionResult Result)
				{
					bool bAutoCreateParty = false;
					GConfig->GetBool(TEXT("AccelByteSocialToolkit"), TEXT("bAutoCreateParty"), bAutoCreateParty, GEngineIni);

					if (bAutoCreateParty)
					{
						UE_LOG(LogTemp, Log, TEXT("bAutoCreateParty true: creating party"));
						CreateParty(LocalPlayerIndex);
					}
				}));
			bWasInParty = true;
			return;
		}
	}
	bWasInParty = false;
}

void UAccelByteCommonPartySubsystem::CreatePartyIfNotExist(bool& bWasNotInParty, int32 LocalPlayerIndex)
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
			CreateParty(LocalPlayerIndex);
			bWasNotInParty = true;
			return;
		}
	}
	bWasNotInParty = false;
}

void UAccelByteCommonPartySubsystem::SetOnPartyInviteRequestReceivedDelegate(FPartyInviteReceived OnInvitationReceived, int32 LocalPlayerIndex)
{
	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr.IsValid());

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const IOnlineFriendsPtr FriendsPtr = OSS->GetFriendsInterface();
		check(FriendsPtr);

		PartyPtr->OnPartyInviteReceivedDelegates.AddWeakLambda(this,
			[OnInvitationReceived, LocalPlayerIndex, FriendsPtr](
				const FUniqueNetId& LocalUserId,
				const FOnlinePartyId& PartyId,
				const FUniqueNetId& SenderId)
			{
				const FABPartySubsystemPartyMember Sender(
					FUniqueNetIdRepl(SenderId),
					FriendsPtr->GetFriend(LocalPlayerIndex, SenderId, "")->GetDisplayName(),
					&PartyId);
				OnInvitationReceived.ExecuteIfBound(Sender);
			});
	}
}

void UAccelByteCommonPartySubsystem::SetOnPartyDataChangeDelegate(FPartyVoidDelegate OnChange, int32 LocalPlayerIndex)
{
	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr);

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		// Called when user joined party
		PartyPtr->OnPartyMemberJoinedDelegates.AddWeakLambda(this,
			[OnChange](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId)
			{
				UE_LOG(LogTemp, Log, TEXT("Party OSS: Joined party"));
				OnChange.ExecuteIfBound();
			});

		// Called on left party
		PartyPtr->OnPartyExitedDelegates.AddWeakLambda(this,
			[this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId)
			{
				UE_LOG(LogTemp, Log, TEXT("Party OSS: Local user left party"));
			});

		// Called on join another party
		PartyPtr->OnPartyJoinedDelegates.AddWeakLambda(this,
			[OnChange](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId)
			{
				UE_LOG(LogTemp, Log, TEXT("Party OSS: Local user left party"));
				OnChange.ExecuteIfBound();
			});

		// Called on member left party
		PartyPtr->OnPartyMemberExitedDelegates.AddWeakLambda(this,
			[OnChange](
				const FUniqueNetId& /*LocalUserId*/,
				const FOnlinePartyId& /*PartyId*/,
				const FUniqueNetId& /*MemberId*/,
				const EMemberExitedReason /*Reason*/)
			{
				UE_LOG(LogTemp, Log, TEXT("Party OSS: Party member exited"));
				OnChange.ExecuteIfBound();
			});

		// Called on member successfully promoted
		PartyPtr->OnPartyMemberPromotedDelegates.AddWeakLambda(this,
			[OnChange](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& NewLeaderId)
			{
				UE_LOG(LogTemp, Log, TEXT("Party OSS: Party member promoted"));
				OnChange.ExecuteIfBound();
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

bool UAccelByteCommonPartySubsystem::IsLocalUserLeader(int32 LocalPlayerIndex)
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
			return LocalUserId == OnlineParty->LeaderId;
		}
	}
	return false;
}

bool UAccelByteCommonPartySubsystem::ShouldAutoCreateParty()
{
	bool bAutoCreateParty = false;
	GConfig->GetBool(TEXT("AccelByteSocialToolkit"), TEXT("bAutoCreateParty"), bAutoCreateParty, GEngineIni);
	return bAutoCreateParty;
}

void UAccelByteCommonPartySubsystem::CreateParty(int32 LocalPlayerIndex, TDelegate<void()> OnComplete)
{
	if (OSS)
	{
		const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
		check(PartyPtr);

		const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
		check(IdentityPtr);

		const FUniqueNetIdPtr LocalUserId = IdentityPtr->GetUniquePlayerId(LocalPlayerIndex);

		PartyPtr->CreateParty(
			*LocalUserId,
			PartyTypeId,
			FPartyConfiguration(),
			FOnCreatePartyComplete::CreateWeakLambda(this,
				[OnComplete](
					const FUniqueNetId& LocalUserId,
					const TSharedPtr<const FOnlinePartyId>& PartyId,
					const ECreatePartyCompletionResult Result)
				{
					OnComplete.ExecuteIfBound();
				}));
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
	for (const FOnlinePartyMemberConstRef PartyMember : PartyMembers)
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
	for (const IOnlinePartyJoinInfoConstRef PartyJoinInfo : PartyJoinInfos)
	{
		if (PartyJoinInfo->GetSourceUserId() == SenderUniqueId)
		{
			TargetPartyJoinInfo = PartyJoinInfo;
		}
	}
	PartyPtr->JoinParty(*LocalUserUniqueId, *TargetPartyJoinInfo);
}
