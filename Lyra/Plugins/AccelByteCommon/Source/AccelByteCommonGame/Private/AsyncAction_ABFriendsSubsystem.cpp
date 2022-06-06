// Copyright (c) 2018 AccelByte, inc. All rights reserved.


#include "AsyncAction_ABFriendsSubsystem.h"

#include "Online.h"
#include "OnlineSubsystemUtils.h"

UAsyncAction_ABFriendsSubsystemQueryCachedList* UAsyncAction_ABFriendsSubsystemQueryCachedList::QueryCachedFriendsList(
	ULocalPlayer* LocalPlayer)
{
	UAsyncAction_ABFriendsSubsystemQueryCachedList* Action =
		NewObject<UAsyncAction_ABFriendsSubsystemQueryCachedList>();

	if (LocalPlayer)
	{
		Action->SocialToolkit = Cast<UAccelByteSocialToolkit>(USocialToolkit::GetToolkitForPlayer(LocalPlayer));
	}
	else
	{
		Action->SetReadyToDestroy();
	}

	return Action;
}

void UAsyncAction_ABFriendsSubsystemQueryCachedList::Activate()
{
	Super::Activate();

	if (SocialToolkit.IsValid())
	{
		SocialToolkit->OnQueryFriendsListSuccessDelegate =
			TDelegate<void(TArray<USocialUser*>)>::CreateWeakLambda(this, [this]
				(TArray<USocialUser*> SocialUsers)
			{
				FABFriendSubsystemOnlineFriends OnlineFriends;
				OnlineFriends.Data = UAccelByteCommonFriendSubsystem::BlueprintableSocialUserListConversion(SocialUsers);
				OnComplete.Broadcast(OnlineFriends);
			});
		SocialToolkit->QueryFriendsList();
	}
	else
	{
		SetReadyToDestroy();
	}
}

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
					if (bWasSuccessful)
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
