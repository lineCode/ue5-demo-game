// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager."


#include "AccelByteCommonFriendSubsystem.h"

#include "CommonUserSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemUtils.h"

void UAccelByteCommonFriendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OSS = Online::GetSubsystem(GetWorld());
	check(OSS);

	SetFriendsNotifDelegates();
}

void UAccelByteCommonFriendSubsystem::GetFriendsList(
	const TDelegate<void(TArray<FABFriendSubsystemOnlineFriend>&)>& OnComplete,
	int32 LocalPlayerIndex)
{
	const IOnlineFriendsPtr FriendsPtr = OSS->GetFriendsInterface();
	check(FriendsPtr);

	TArray<TSharedRef<FOnlineFriend>> OnlineFriends;
	if (FriendsPtr->GetFriendsList(LocalPlayerIndex, FriendsListName, OnlineFriends))
	{
		TArray<FABFriendSubsystemOnlineFriend> ABOnlineFriends = BlueprintableSocialUserListConversion(OnlineFriends);
		OnComplete.ExecuteIfBound(ABOnlineFriends);
	}
	else
	{
		FriendsPtr->ReadFriendsList(LocalPlayerIndex, FriendsListName, FOnReadFriendsListComplete::CreateWeakLambda(this,
			[FriendsPtr, LocalPlayerIndex, OnComplete](int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FString& /*ListName*/, const FString& /*ErrorStr*/)
			{
				TArray<TSharedRef<FOnlineFriend>> OnlineFriends;
				FriendsPtr->GetFriendsList(LocalPlayerIndex, FriendsListName, OnlineFriends);

				TArray<FABFriendSubsystemOnlineFriend> ABOnlineFriends = BlueprintableSocialUserListConversion(OnlineFriends);
				OnComplete.ExecuteIfBound(ABOnlineFriends);
			}));
	}
}

bool UAccelByteCommonFriendSubsystem::GetFriendByUniqueNetId(
	FABFriendSubsystemOnlineFriend& OutFoundFriend,
	const FUniqueNetIdRepl TargetId,
	int32 LocalPlayerIndex)
{
	const IOnlineFriendsPtr FriendsPtr = OSS->GetFriendsInterface();
	check(FriendsPtr);

	/*
	 * GetFriend doesn't work 100% of the time. Manually search the array for now.
	 * Case which 'it' dont work: A and B not friend > A send B friend request > A can found B with it's Unique Id found
	 * by searching by name > B cannot found A with it's Unique Id found by searching by name, although A exists in B's
	 * inbound list
	 */
	TArray<TSharedRef<FOnlineFriend>> OnlineFriends;
	if (!FriendsPtr->GetFriendsList(LocalPlayerIndex, FriendsListName, OnlineFriends))
	{
		UE_LOG(LogAccelByteCommonFriend, Warning, TEXT("Friends list have not yet cached. Call Read friendlist before using this function"));
		return false;
	}

	const TSharedRef<const FUniqueNetIdAccelByteUser> ABTargetUser =
		FUniqueNetIdAccelByteUser::Cast(*TargetId.GetUniqueNetId());
	for (const TSharedRef<FOnlineFriend> OnlineFriend : OnlineFriends)
	{
		const TSharedRef<const FUniqueNetIdAccelByteUser> ABUser =
			FUniqueNetIdAccelByteUser::Cast(*OnlineFriend->GetUserId());
		if (ABUser->GetAccelByteId() == ABTargetUser->GetAccelByteId())
		{
			OutFoundFriend = BlueprintableOnlineFriendConversion(OnlineFriend);
			return true;
		}
	}

	return false;
}

void UAccelByteCommonFriendSubsystem::GetLocalUserDisplayNameAndPlatform(
	FString& OutDisplayName, FString& OutPlatform, const int32 LocalPlayerIndex) const
{
	const IOnlineIdentityPtr IdentityPtr = OSS->GetIdentityInterface();
	check(IdentityPtr);

	const TSharedRef<const FUniqueNetIdAccelByteUser> ABUser =
		FUniqueNetIdAccelByteUser::Cast(*IdentityPtr->GetUniquePlayerId(LocalPlayerIndex));

	OutDisplayName = IdentityPtr->GetPlayerNickname(LocalPlayerIndex);
	OutPlatform = ABUser->GetPlatformType() == "" ? PlatformType_Password : ABUser->GetPlatformType();
}

void UAccelByteCommonFriendSubsystem::SendFriendRequest(FUniqueNetIdRepl TargetId, int32 LocalPlayerIndex)
{
	const IOnlineFriendsPtr FriendsPtr = OSS->GetFriendsInterface();
	check(FriendsPtr);

	FriendsPtr->SendInvite(LocalPlayerIndex, *TargetId.GetUniqueNetId(), FriendsListName);
}

void UAccelByteCommonFriendSubsystem::AcceptFriendRequest(FUniqueNetIdRepl SenderUniqueId, const int32 LocalPlayerIndex)
{
	const IOnlineFriendsPtr FriendsPtr = OSS->GetFriendsInterface();
	check(FriendsPtr);

	FriendsPtr->AcceptInvite(LocalPlayerIndex, *SenderUniqueId.GetUniqueNetId(), FriendsListName);
}

void UAccelByteCommonFriendSubsystem::RejectFriendRequest(FUniqueNetIdRepl SenderUniqueId, const int32 LocalPlayerIndex)
{
	const IOnlineFriendsPtr FriendsPtr = OSS->GetFriendsInterface();
	check(FriendsPtr);

	FriendsPtr->RejectInvite(LocalPlayerIndex, *SenderUniqueId.GetUniqueNetId(), FriendsListName);
}

void UAccelByteCommonFriendSubsystem::DeleteFriend(FUniqueNetIdRepl TargetUniqueId, const int32 LocalPlayerIndex)
{
	const IOnlineFriendsPtr FriendsPtr = OSS->GetFriendsInterface();
	check(FriendsPtr);

	FriendsPtr->DeleteFriend(LocalPlayerIndex, *TargetUniqueId.GetUniqueNetId(), FriendsListName);
}

void UAccelByteCommonFriendSubsystem::SetFriendsNotifDelegates()
{
	const IOnlineFriendsPtr FriendsPtr = OSS->GetFriendsInterface();
	check(FriendsPtr);

	FriendsPtr->OnFriendRemovedDelegates.AddWeakLambda(this, [this](const FUniqueNetId& /*UserId*/, const FUniqueNetId& /*FriendId*/)
	{
		UE_LOG(LogAccelByteCommonFriend, Log, TEXT("Friend removed"));
		OnFriendsListChange.Broadcast();
	});

	FriendsPtr->OnFriendsChangeDelegates->AddWeakLambda(this, [this]()
	{
		UE_LOG(LogAccelByteCommonFriend, Log, TEXT("Friend list changed"));
		OnFriendsListChange.Broadcast();
	});

	FriendsPtr->OnInviteAcceptedDelegates.AddWeakLambda(this, [this](const FUniqueNetId& /*UserId*/, const FUniqueNetId& /*FriendId*/)
	{
		UE_LOG(LogAccelByteCommonFriend, Log, TEXT("Outgoing friend invite accepted"));
		OnFriendsListChange.Broadcast();
	});

	FriendsPtr->OnInviteAbortedDelegates.AddWeakLambda(this, [this](const FUniqueNetId& /*UserId*/, const FUniqueNetId& /*FriendId*/)
	{
		UE_LOG(LogAccelByteCommonFriend, Log, TEXT("Outgoing friend invite aborted"));
		OnFriendsListChange.Broadcast();
	});

	FriendsPtr->OnInviteReceivedDelegates.AddWeakLambda(this, [this](const FUniqueNetId& /*UserId*/, const FUniqueNetId& /*FriendId*/)
	{
		UE_LOG(LogAccelByteCommonFriend, Log, TEXT("Friend invite received"));
		OnFriendsListChange.Broadcast();
	});

	FriendsPtr->OnInviteRejectedDelegates.AddWeakLambda(this, [this](const FUniqueNetId& /*UserId*/, const FUniqueNetId& /*FriendId*/)
	{
		UE_LOG(LogAccelByteCommonFriend, Log, TEXT("Outgoing friend invite rejected"));
		OnFriendsListChange.Broadcast();
	});

	FriendsPtr->OnOutgoingInviteSentDelegates->AddWeakLambda(this, [this]()
	{
		UE_LOG(LogAccelByteCommonFriend, Log, TEXT("Friend invite sent"));
		OnFriendsListChange.Broadcast();
	});

	FriendsPtr->OnDeleteFriendCompleteDelegates->AddWeakLambda(this, [this](int32, bool /*bWasSuccessful*/, const FUniqueNetId& /*FriendId*/, const FString& /*ListName*/, const FString& /*ErrorStr*/)
	{
		UE_LOG(LogAccelByteCommonFriend, Log, TEXT("Friend deleted"));
		OnFriendsListChange.Broadcast();
	});

	FriendsPtr->OnRejectInviteCompleteDelegates->AddWeakLambda(this, [this](int32, bool /*bWasSuccessful*/, const FUniqueNetId& /*FriendId*/, const FString& /*ListName*/, const FString& /*ErrorStr*/)
	{
		UE_LOG(LogAccelByteCommonFriend, Log, TEXT("Incoming friend request rejected"));
		OnFriendsListChange.Broadcast();
	});
}

TArray<FABFriendSubsystemOnlineFriend> UAccelByteCommonFriendSubsystem::BlueprintableSocialUserListConversion(
	TArray<TSharedRef<FOnlineFriend>> OnlineFriends)
{
	TArray<FABFriendSubsystemOnlineFriend> ABOnlineFriends;

	for (const TSharedRef<FOnlineFriend>& OnlineFriend : OnlineFriends)
	{
		if (OnlineFriend->GetInviteStatus() != EInviteStatus::Unknown)
		{
			const TSharedRef<const FUniqueNetIdAccelByteUser> ABFriend =
					FUniqueNetIdAccelByteUser::Cast(*OnlineFriend->GetUserId());
			const FString FriendPlatformType = ABFriend->GetPlatformType() == ""? PlatformType_Password : ABFriend->GetPlatformType();
			const FOnlineUserPresence& FriendPresence = OnlineFriend->GetPresence();

			FABFriendSubsystemOnlineFriend ABOnlineFriend = FABFriendSubsystemOnlineFriend(
				OnlineFriend->GetUserId(),
				OnlineFriend->GetDisplayName(),
				FriendPlatformType,
				BlueprintableInviteStatusConversion(OnlineFriend->GetInviteStatus()),
				FriendPresence.bIsPlayingThisGame);

			ABOnlineFriends.Add(ABOnlineFriend);
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

FABFriendSubsystemOnlineFriend UAccelByteCommonFriendSubsystem::BlueprintableOnlineFriendConversion(
	TSharedPtr<FOnlineFriend> OnlineFriend)
{
	FABFriendSubsystemOnlineFriend ABOnlineFriend = FABFriendSubsystemOnlineFriend(
		OnlineFriend->GetUserId(),
		OnlineFriend->GetDisplayName());
	ABOnlineFriend.InviteStatus = BlueprintableInviteStatusConversion(OnlineFriend->GetInviteStatus());

	return ABOnlineFriend;
}
