// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager."


#include "AccelByteCommonFriendSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "SocialManager.h"
#include "SocialToolkit.h"
#include "Party/PartyMember.h"
#include "Party/SocialParty.h"

void UAccelByteCommonFriendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UAccelByteCommonFriendSubsystem::GetLocalUserDisplayNameAndPlatform(FString& DisplayName, FString& Platform, ULocalPlayer* LocalPlayer)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		DisplayName = SocialToolkit->GetLocalUser().GetDisplayName(ESocialSubsystem::Primary);
		Platform = SocialToolkit->GetLocalUser().GetCurrentPlatform().ToString();
	}
}

void UAccelByteCommonFriendSubsystem::SendFriendRequest(ULocalPlayer* LocalPlayer, FString DisplayName)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		SocialToolkit->TrySendFriendInvite(DisplayName);
	}
}

void UAccelByteCommonFriendSubsystem::AcceptFriendRequest(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl SenderUniqueId)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		const USocialUser* SocialUser = SocialToolkit->FindUser(SenderUniqueId);
		check(SocialUser);

		SocialUser->AcceptFriendInvite(ESocialSubsystem::Primary);
	}
}

void UAccelByteCommonFriendSubsystem::RejectFriendRequest(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl SenderUniqueId)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		const USocialUser* SocialUser = SocialToolkit->FindUser(SenderUniqueId);
		check(SocialUser);

		SocialUser->RejectFriendInvite(ESocialSubsystem::Primary);
	}
}

void UAccelByteCommonFriendSubsystem::Unfriend(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl TargetUniqueId)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		const USocialUser* SocialUser = SocialToolkit->FindUser(TargetUniqueId);
		check(SocialUser);

		SocialUser->EndFriendship(ESocialSubsystem::Primary);
	}
}

void UAccelByteCommonFriendSubsystem::CancelSentFriendRequest(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl TargetUniqueId)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		const IOnlineSubsystem* OSS = SocialToolkit->GetSocialOss(ESocialSubsystem::Primary);
		check(OSS);

		const IOnlineFriendsPtr FriendsInf = OSS->GetFriendsInterface();
		check(FriendsInf);

		const FUniqueNetId* UniqueNetId = TargetUniqueId.GetUniqueNetId().Get();
		FriendsInf->DeleteFriend(LocalPlayer->GetLocalPlayerIndex(), *UniqueNetId, "");
	}
}

void UAccelByteCommonFriendSubsystem::OnFriendsListChange(
	ULocalPlayer* LocalPlayer, FFriendVoidDelegate OnListChange)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		SocialToolkit->OnFriendInviteReceived().AddWeakLambda(this, [OnListChange]
			(USocialUser& SocialUser, ESocialSubsystem Subsystem)
		{
			OnListChange.ExecuteIfBound();
		});
		SocialToolkit->OnFriendInviteSent().AddWeakLambda(this, [OnListChange]
			(USocialUser& SocialUser, ESocialSubsystem Subsystem)
		{
			OnListChange.ExecuteIfBound();
		});
		SocialToolkit->OnFriendshipEstablished().AddWeakLambda(this, [OnListChange]
			(USocialUser& SocialUser, ESocialSubsystem Subsystem, bool bIsNewlyEstablished)
		{
			OnListChange.ExecuteIfBound();
		});

		/**
		 * Social Toolkit does not cover On Receieved Friend Request accepted and rejected
		 * This is used as a workaround for that
		 */
		SocialToolkit->GetSocialOss(ESocialSubsystem::Primary)->GetFriendsInterface()->OnFriendsChangeDelegates->AddWeakLambda(
			this, [OnListChange]()
			{
				OnListChange.ExecuteIfBound();
			});
	}
}

void UAccelByteCommonFriendSubsystem::OnUserPresenceChange(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl TargetUniqueId,
	FFriendPresenceChange OnPresenceChange)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		USocialUser* SocialUser = SocialToolkit->FindUser(TargetUniqueId);

		SocialUser->OnUserPresenceChanged().AddWeakLambda(this,
			[OnPresenceChange, SocialUser](ESocialSubsystem SocialSubsystem)
		{
			OnPresenceChange.ExecuteIfBound(SocialUser->IsPlayingThisGame(), SocialUser->GetCurrentPlatform());
		});
	}
}

TArray<FABFriendSubsystemOnlineFriend> UAccelByteCommonFriendSubsystem::BlueprintableSocialUserListConversion(
	TArray<USocialUser*> SocialUsers)
{
	TArray<FABFriendSubsystemOnlineFriend> ABOnlineFriends;

	for (const USocialUser* SocialUser : SocialUsers)
	{
		if (SocialUser->GetFriendInviteStatus(ESocialSubsystem::Primary) != EInviteStatus::Unknown)
		{
			FABFriendSubsystemOnlineFriend OnlineFriend = FABFriendSubsystemOnlineFriend(
				SocialUser->GetUserId(ESocialSubsystem::Primary),
				SocialUser->GetDisplayName(),
				SocialUser->GetCurrentPlatform().ToString(),
				BlueprintableInviteStatusConversion(SocialUser->GetFriendInviteStatus(ESocialSubsystem::Primary)),
				SocialUser->IsPlayingThisGame());

			if (SocialUser->GetPartyMember(IOnlinePartySystem::GetPrimaryPartyTypeId()))
			{
				OnlineFriend.bIsInParty = true;
			}

			ABOnlineFriends.Add(OnlineFriend);
		}
	}

	return ABOnlineFriends;
}

EABFriendSubsystemInviteStatus UAccelByteCommonFriendSubsystem::BlueprintableInviteStatusConversion(EInviteStatus::Type InviteStatus)
{
	switch (InviteStatus)
	{
	case EInviteStatus::Type::Unknown:
		return EABFriendSubsystemInviteStatus::Unknown;
	case EInviteStatus::Type::Accepted:
		return EABFriendSubsystemInviteStatus::Accepted;
	case EInviteStatus::Type::PendingInbound:
		return EABFriendSubsystemInviteStatus::PendingInbound;
	case EInviteStatus::Type::PendingOutbound:
		return EABFriendSubsystemInviteStatus::PendingOutbound;
	case EInviteStatus::Type::Blocked:
		return EABFriendSubsystemInviteStatus::Blocked;
	case EInviteStatus::Type::Suggested:
		return EABFriendSubsystemInviteStatus::Suggested;
	default:
		return EABFriendSubsystemInviteStatus::Unknown;
	}
}
