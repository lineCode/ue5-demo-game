// Copyright (c) 2018 AccelByte, inc. All rights reserved.


#include "AsyncAction_ABFriendsSubsystem.h"

#include "Online.h"
#include "OnlineSubsystemUtils.h"
#include "SocialToolkit.h"

UAsyncAction_ABFriendsSubsystemSearchUser* UAsyncAction_ABFriendsSubsystemSearchUser::SearchUserByExactDisplayName(
	UObject* WorldContextObject, int32 LocalTargetUserNum, FString DisplayName, bool bHideSelf)
{
	UAsyncAction_ABFriendsSubsystemSearchUser* Action =
		NewObject<UAsyncAction_ABFriendsSubsystemSearchUser>();

	if (WorldContextObject)
	{
		IOnlineSubsystem* OSS = Online::GetSubsystem(WorldContextObject->GetWorld());
		check(OSS);

		Action->OSS = OSS;
		Action->DisplayName = DisplayName;
		Action->bHideSelf = bHideSelf;
		Action->LocalTargetUserNum = LocalTargetUserNum;
	}
	else
	{
		Action->SetReadyToDestroy();
	}

	return Action;
}

void UAsyncAction_ABFriendsSubsystemSearchUser::Activate()
{
	Super::Activate();

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
				[LocalPlayerUserId, this](
					bool bWasSuccessful,
					const FUniqueNetId& UserId,
					const FString& Username,
					const FUniqueNetId& FoundUserId,
					const FString& ErrorMessage)
				{
					if (bWasSuccessful && FoundUserId.IsValid())
					{
						if (*LocalPlayerUserId.Get() == FoundUserId && bHideSelf)
						{
							OnNotFound.Broadcast();
						}
						else
						{
							/**
							* uses this directly because we only need username and user id for this implementation.
							* If more information is needed,
							* use QueryUserInfoj > set OnQueryUserInfoCompleteDelegates > GetUserInfo
							*/
							const FABFriendSubsystemOnlineUser OnlineUser(FoundUserId.AsShared(), Username);
							OnFound.Broadcast(OnlineUser);
						}
					}
					else
					{
						OnNotFound.Broadcast();
					}
				}));
	}
}

UAsyncAction_ABFriendsSubsystemGetFriendsList* UAsyncAction_ABFriendsSubsystemGetFriendsList::GetFriendsList(
	UAccelByteCommonFriendSubsystem* Target, int32 LocalPlayerIndex)
{
	UAsyncAction_ABFriendsSubsystemGetFriendsList* Action =
		NewObject<UAsyncAction_ABFriendsSubsystemGetFriendsList>();

	if (Target)
	{
		Action->FriendSubsystem = Target;
		Action->LocalPlayerIndex = LocalPlayerIndex;
	}
	else
	{
		Action->SetReadyToDestroy();
	}

	return Action;
}

void UAsyncAction_ABFriendsSubsystemGetFriendsList::Activate()
{
	Super::Activate();

	if (FriendSubsystem)
	{
		FriendSubsystem->GetFriendsList(TDelegate<void(TArray<FABFriendSubsystemOnlineFriend>&)>::CreateWeakLambda(
		this, [this](TArray<FABFriendSubsystemOnlineFriend>& Result)
		{
			const FABFriendSubsystemOnlineFriends ABOnlineFriends(Result);
			OnComplete.Broadcast(ABOnlineFriends);
		}), LocalPlayerIndex);
	}
	else
	{
		SetReadyToDestroy();
	}
}
