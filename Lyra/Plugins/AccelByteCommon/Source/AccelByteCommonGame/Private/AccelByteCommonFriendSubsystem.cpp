// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager."


#include "AccelByteCommonFriendSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

void UAccelByteCommonFriendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OSS = Online::GetSubsystem(GetWorld());
}

void UAccelByteCommonFriendSubsystem::GetFriendsList(int32 LocalTargetUserNum, FOnCompleteGetFriendsList OnComplete)
{
	if (OSS)
	{
		const IOnlineFriendsPtr FriendInf = OSS->GetFriendsInterface();
		check(FriendInf);

		TArray<TSharedRef<FOnlineFriend>> OutFriendsList;
		if(FriendInf->GetFriendsList(LocalTargetUserNum, "", OutFriendsList))
		{
			FABFriendSubsystemOnlineFriends OnlineFriends;
			OnlineFriends.Data = BlueprintableFriendsDataConversion(OutFriendsList);
			OnComplete.ExecuteIfBound(OnlineFriends);
		}
		else
		{
			FriendInf->ReadFriendsList(LocalTargetUserNum, "",
				FOnReadFriendsListComplete::CreateWeakLambda(this,
				[OnComplete, FriendInf, LocalTargetUserNum](int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
				{
					TArray<TSharedRef<FOnlineFriend>> OutFriendsList;
					FABFriendSubsystemOnlineFriends OnlineFriends;
					FriendInf->GetFriendsList(LocalTargetUserNum, "", OutFriendsList);
					OnlineFriends.Data = BlueprintableFriendsDataConversion(OutFriendsList);
					OnComplete.ExecuteIfBound(OnlineFriends);
				}));
		}
	}
}

void UAccelByteCommonFriendSubsystem::SearchUserByExactDisplayName(
	int32 LocalTargetUserNum,
	FString DisplayName,
	FOnCompleteQueryUserMapping OnComplete,
	FFriendVoidDelegate OnNotFound, bool bHideSelf)
{
	if (OSS)
	{
		const IOnlineUserPtr UserInf = OSS->GetUserInterface();
		const IOnlineIdentityPtr IdentityInf = OSS->GetIdentityInterface();
		check(UserInf);
		check(IdentityInf);

		const FUniqueNetIdPtr LocalPlayerUserId = IdentityInf->GetUniquePlayerId(LocalTargetUserNum);
		check(LocalPlayerUserId);

		UserInf->QueryUserIdMapping(*LocalPlayerUserId, DisplayName,
			IOnlineUser::FOnQueryUserMappingComplete::CreateWeakLambda(this,
				[OnNotFound, OnComplete, LocalPlayerUserId, bHideSelf](
					bool bWasSuccessful,
					const FUniqueNetId& UserId,
					const FString& Username,
					const FUniqueNetId& FoundUserId,
					const FString& ErrorMessage)
				{
					const bool bIsLocalPlayer = *LocalPlayerUserId.Get() == FoundUserId && bHideSelf;
					if (bWasSuccessful && !bIsLocalPlayer)
					{
						/**
						 * uses this directly because we only need username and user id for this implementation.
						 * If more information is needed,
						 * use QueryUserInfoj > set OnQueryUserInfoCompleteDelegates > GetUserInfo
						 */
						const FABFriendSubsystemOnlineUser BPOnlineUser(FoundUserId.AsShared(), FText::FromString(Username));
						OnComplete.ExecuteIfBound(BPOnlineUser);
					}
					else
					{
						OnNotFound.ExecuteIfBound();
					}
				}));
	}
}

void UAccelByteCommonFriendSubsystem::SendFriendRequest(int32 LocalTargetUserNum, FUniqueNetIdRepl TargetUniqueId)
{
	if (OSS)
	{
		const IOnlineFriendsPtr FriendsInf = OSS->GetFriendsInterface();
		check(FriendsInf);

		const FUniqueNetId* UniqueNetId = TargetUniqueId.GetUniqueNetId().Get();
		// Don't need to use FOnSendInviteComplete delegate. BP_OnFriendListChange will update the UI
		FriendsInf->SendInvite(LocalTargetUserNum, *UniqueNetId, "");
	}
}

void UAccelByteCommonFriendSubsystem::AcceptFriendRequest(int32 LocalTargetUserNum, FUniqueNetIdRepl SenderUniqueId)
{
	if (OSS)
	{
		const IOnlineFriendsPtr FriendsInf = OSS->GetFriendsInterface();
		check(FriendsInf);

		const FUniqueNetId* UniqueNetId = SenderUniqueId.GetUniqueNetId().Get();
		// Don't need to use FOnAcceptInviteComplete delegate. BP_OnFriendListChange will update the UI
		FriendsInf->AcceptInvite(LocalTargetUserNum, *UniqueNetId, "");
	}
}

void UAccelByteCommonFriendSubsystem::RejectFriendRequest(int32 LocalTargetUserNum, FUniqueNetIdRepl SenderUniqueId)
{
	if (OSS)
	{
		const IOnlineFriendsPtr FriendsInf = OSS->GetFriendsInterface();
		check(FriendsInf);

		const FUniqueNetId* UniqueNetId = SenderUniqueId.GetUniqueNetId().Get();
		FriendsInf->RejectInvite(LocalTargetUserNum, *UniqueNetId, "");
	}
}

void UAccelByteCommonFriendSubsystem::RemoveFriend(int32 LocalTargetUserNum, FUniqueNetIdRepl TargetUniqueId)
{
	if (OSS)
	{
		const IOnlineFriendsPtr FriendsInf = OSS->GetFriendsInterface();
		check(FriendsInf);

		const FUniqueNetId* UniqueNetId = TargetUniqueId.GetUniqueNetId().Get();
		FriendsInf->DeleteFriend(LocalTargetUserNum, *UniqueNetId, "");
	}
}

void UAccelByteCommonFriendSubsystem::OnFriendListChange(int32 LocalTargetUserNum, FFriendVoidDelegate OnChange)
{
	if (OSS)
	{
		const IOnlineFriendsPtr FriendsInf = OSS->GetFriendsInterface();
		check(FriendsInf);

		FriendsInf->ClearOnFriendsChangeDelegates(LocalTargetUserNum, this);
		FriendsInf->OnFriendsChangeDelegates->AddWeakLambda(this, [OnChange]()
		{
			OnChange.ExecuteIfBound();
		});
	}
}

TArray<FABFriendSubsystemOnlineFriend> UAccelByteCommonFriendSubsystem::BlueprintableFriendsDataConversion(
	TArray<TSharedRef<FOnlineFriend>>& FriendsList)
{
	TArray<FABFriendSubsystemOnlineFriend> FriendsListResult;
	for (const TSharedRef<FOnlineFriend>& Friend : FriendsList)
	{
		FABFriendSubsystemOnlineFriend BPFriend(
			Friend->GetUserId(),
			FText::FromString(Friend->GetDisplayName()),
			FText::FromString(Friend->GetPresence().GetPlatform()),
			BlueprintableInviteStatusConversion(Friend->GetInviteStatus()),
			Friend->GetPresence().bIsPlayingThisGame);
		FriendsListResult.Add(BPFriend);
	}

	return FriendsListResult;
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
