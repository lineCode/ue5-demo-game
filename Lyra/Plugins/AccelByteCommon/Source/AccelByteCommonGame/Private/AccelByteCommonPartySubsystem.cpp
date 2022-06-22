// Copyright (c) 2018 AccelByte, inc. All rights reserved.


#include "AccelByteCommonPartySubsystem.h"

#include "Online.h"
#include "OnlineSubsystemUtils.h"
#include "SocialManager.h"
#include "Messaging/CommonGameDialog.h"
#include "Party/SocialParty.h"

void UAccelByteCommonPartySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OSS = Online::GetSubsystem(GetWorld());

	SetOnPartyInviteRequestReceivedDelegate();
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
						UE_LOG(LogAccelByteCommonParty, Log, TEXT("bAutoCreateParty true: creating party"));
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

void UAccelByteCommonPartySubsystem::SetOnPartyInviteRequestReceivedDelegate(int32 LocalPlayerIndex)
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
			[LocalPlayerIndex, FriendsPtr, this](
				const FUniqueNetId& LocalUserId,
				const FOnlinePartyId& PartyId,
				const FUniqueNetId& SenderId)
			{
				const FABPartySubsystemPartyMember Sender(
					FUniqueNetIdRepl(SenderId),
					FriendsPtr->GetFriend(LocalPlayerIndex, SenderId, "")->GetDisplayName(),
					&PartyId);

				ShowReceivedInvitePopup(this, Sender, LocalPlayerIndex);
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
			[OnChange, this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId)
			{
				UE_LOG(LogAccelByteCommonParty, Log, TEXT("Joined party"));
				OnChange.ExecuteIfBound();
			});

		// Called on left party
		PartyPtr->OnPartyExitedDelegates.AddWeakLambda(this,
			[this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId)
			{
				UE_LOG(LogAccelByteCommonParty, Log, TEXT("Local user left party"));
			});

		// Called on join another party
		PartyPtr->OnPartyJoinedDelegates.AddWeakLambda(this,
			[OnChange](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId)
			{
				UE_LOG(LogAccelByteCommonParty, Log, TEXT("Local joined party"));
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
				UE_LOG(LogAccelByteCommonParty, Log, TEXT("Party member exited"));
				OnChange.ExecuteIfBound();
			});

		// Called on member successfully promoted
		PartyPtr->OnPartyMemberPromotedDelegates.AddWeakLambda(this,
			[OnChange](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& NewLeaderId)
			{
				UE_LOG(LogAccelByteCommonParty, Log, TEXT("Party member promoted"));
				OnChange.ExecuteIfBound();
			});

	}
}

void UAccelByteCommonPartySubsystem::AcceptPartyInvite(FUniqueNetIdRepl SenderUniqueId, int32 LocalPlayerIndex)
{
	if (OSS)
	{const IOnlinePartyPtr PartyPtr = OSS->GetPartyInterface();
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

int32 UAccelByteCommonPartySubsystem::GetPartyMemberMax(const int32 LocalPlayerIndex)
{
	int32 MaxPartyMembers;

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
			GConfig->GetInt(TEXT("AccelByteSocialToolkit"), TEXT("MaxPartyMembers"), MaxPartyMembers, GEngineIni);
		}
	}
	return MaxPartyMembers;
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

		FPartyConfiguration Config;
		Config.bIsAcceptingMembers = true;

		int32 MaxPartyMembers = 0;
		GConfig->GetInt(TEXT("AccelByteSocialToolkit"), TEXT("MaxPartyMembers"), MaxPartyMembers, GEngineIni);
		Config.MaxMembers = MaxPartyMembers;

		PartyPtr->CreateParty(
			*LocalUserId,
			PartyTypeId,
			Config,
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
				});
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
