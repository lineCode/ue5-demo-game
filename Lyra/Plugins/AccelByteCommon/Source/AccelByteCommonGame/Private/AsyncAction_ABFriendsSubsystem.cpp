// Copyright (c) 2018 AccelByte, inc. All rights reserved.


#include "AsyncAction_ABFriendsSubsystem.h"

#include "Online.h"
#include "OnlineSubsystemUtils.h"
#include "SocialToolkit.h"

UAsyncAction_ABFriendsSubsystemGetFriendList* UAsyncAction_ABFriendsSubsystemGetFriendList::GetFriendList(
	ULocalPlayer* LocalPlayer)
{
	UAsyncAction_ABFriendsSubsystemGetFriendList* Action =
		NewObject<UAsyncAction_ABFriendsSubsystemGetFriendList>();

	if (LocalPlayer)
	{
		Action->SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer);
		Action->LocalPlayerIndex = LocalPlayer->GetLocalPlayerIndex();
	}
	else
	{
		Action->SetReadyToDestroy();
	}

	return Action;
}

void UAsyncAction_ABFriendsSubsystemGetFriendList::Activate()
{
	Super::Activate();

	if (SocialToolkit.IsValid())
	{
		const IOnlineSubsystem* OSS = SocialToolkit->GetSocialOss(ESocialSubsystem::Primary);
		check(OSS);

		const IOnlineFriendsPtr FriendsPtr = OSS->GetFriendsInterface();
		check(FriendsPtr);

		TArray<TSharedRef<FOnlineFriend>> OutFriend;
		if (!FriendsPtr->GetFriendsList(LocalPlayerIndex, "", OutFriend))
		{
			FriendsPtr->ReadFriendsList(LocalPlayerIndex, "", FOnReadFriendsListComplete::CreateWeakLambda(this,
				[this](int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
				{
					TriggerAction();
				}));
		}
		else
		{
			TriggerAction();
		}
	}
	else
	{
		SetReadyToDestroy();
	}
}

void UAsyncAction_ABFriendsSubsystemGetFriendList::TriggerAction() const
{
	FABFriendSubsystemOnlineFriends OnlineFriends;
	const TArray<USocialUser*> SocialUsers = SocialToolkit->GetAllUsers();
	OnlineFriends.Data = UAccelByteCommonFriendSubsystem::BlueprintableSocialUserListConversion(SocialUsers);
	OnComplete.Broadcast(OnlineFriends);
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
