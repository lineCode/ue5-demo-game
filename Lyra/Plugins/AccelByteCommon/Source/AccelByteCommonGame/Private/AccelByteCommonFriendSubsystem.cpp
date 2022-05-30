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

void UAccelByteCommonFriendSubsystem::BP_GetFriendsList(int32 LocalTargetUserNum, FOnCompleteGetFriendsList OnComplete)
{
	if (OSS)
	{
		const IOnlineFriendsPtr FriendInf = OSS->GetFriendsInterface();
		check(FriendInf);

		TArray<TSharedRef<FOnlineFriend>> OutFriendsList;
		if(FriendInf->GetFriendsList(LocalTargetUserNum, "", OutFriendsList))
		{
			FBPOnlineFriends OnlineFriends;
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
					FBPOnlineFriends OnlineFriends;
					FriendInf->GetFriendsList(LocalTargetUserNum, "", OutFriendsList);
					OnlineFriends.Data = BlueprintableFriendsDataConversion(OutFriendsList);
					OnComplete.ExecuteIfBound(OnlineFriends);
				}));
		}
	}
}

void UAccelByteCommonFriendSubsystem::BP_SearchUserByExactUsername(
	int32 LocalTargetUserNum,
	FString Username,
	FOnCompleteQueryUserMapping OnComplete,
	FBPFriendVoidDelegate OnNotFound, bool bHideSelf)
{
	if (OSS)
	{
		const IOnlineUserPtr UserInf = OSS->GetUserInterface();
		const IOnlineIdentityPtr IdentityInf = OSS->GetIdentityInterface();
		check(UserInf);
		check(IdentityInf);

		const FUniqueNetIdPtr LocalPlayerUserId = IdentityInf->GetUniquePlayerId(LocalTargetUserNum);
		check(LocalPlayerUserId);

		UserInf->QueryUserIdMapping(*LocalPlayerUserId, Username,
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
						FBPOnlineUser BPOnlineUser(FoundUserId.AsShared(), FText::FromString(Username));
						OnComplete.ExecuteIfBound(BPOnlineUser);
					}
					else
					{
						OnNotFound.ExecuteIfBound();
					}
				}));
	}
}

void UAccelByteCommonFriendSubsystem::BP_SendFriendRequest(int32 LocalTargetUserNum, FUniqueNetIdRepl TargetUniqueId)
{
	if (OSS)
	{
		IOnlineFriendsPtr FriendsInf = OSS->GetFriendsInterface();
		check(FriendsInf);

		const FUniqueNetId* UniqueNetId = TargetUniqueId.GetUniqueNetId().Get();
		// Don't need to use FOnSendInviteComplete delegate. BP_OnFriendListChange will update the UI
		FriendsInf->SendInvite(LocalTargetUserNum, *UniqueNetId, "");
	}
}

void UAccelByteCommonFriendSubsystem::BP_AcceptFriendRequest(int32 LocalTargetUserNum, FUniqueNetIdRepl SenderUniqueId)
{
	if (OSS)
	{
		IOnlineFriendsPtr FriendsInf = OSS->GetFriendsInterface();
		check(FriendsInf);

		const FUniqueNetId* UniqueNetId = SenderUniqueId.GetUniqueNetId().Get();
		// Don't need to use FOnAcceptInviteComplete delegate. BP_OnFriendListChange will update the UI
		FriendsInf->AcceptInvite(LocalTargetUserNum, *UniqueNetId, "");
	}
}

void UAccelByteCommonFriendSubsystem::BP_RejectFriendRequest(int32 LocalTargetUserNum, FUniqueNetIdRepl SenderUniqueId)
{
	if (OSS)
	{
		IOnlineFriendsPtr FriendsInf = OSS->GetFriendsInterface();
		check(FriendsInf);

		const FUniqueNetId* UniqueNetId = SenderUniqueId.GetUniqueNetId().Get();
		FriendsInf->RejectInvite(LocalTargetUserNum, *UniqueNetId, "");
	}
}

void UAccelByteCommonFriendSubsystem::BP_RemoveFriend(int32 LocalTargetUserNum, FUniqueNetIdRepl TargetUniqueId)
{
	if (OSS)
	{
		IOnlineFriendsPtr FriendsInf = OSS->GetFriendsInterface();
		check(FriendsInf);

		const FUniqueNetId* UniqueNetId = TargetUniqueId.GetUniqueNetId().Get();
		FriendsInf->DeleteFriend(LocalTargetUserNum, *UniqueNetId, "");
	}
}

void UAccelByteCommonFriendSubsystem::BP_OnFriendListChange(int32 LocalTargetUserNum, FBPFriendVoidDelegate OnChange)
{
	if (OSS)
	{
		IOnlineFriendsPtr FriendsInf = OSS->GetFriendsInterface();
		check(FriendsInf);

		FriendsInf->ClearOnFriendsChangeDelegates(LocalTargetUserNum, this);
		FriendsInf->OnFriendsChangeDelegates->AddWeakLambda(this, [OnChange]()
		{
			OnChange.ExecuteIfBound();
		});
	}
}

TArray<FBPOnlineFriend> UAccelByteCommonFriendSubsystem::BlueprintableFriendsDataConversion(
	TArray<TSharedRef<FOnlineFriend>>& FriendsList)
{
	TArray<FBPOnlineFriend> FriendsListResult;
	for (const TSharedRef<FOnlineFriend>& Friend : FriendsList)
	{
		FBPOnlineFriend BPFriend(
			Friend->GetUserId(),
			FText::FromString(Friend->GetDisplayName()),
			FText::FromString(Friend->GetPresence().GetPlatform()),
			BlueprintableInviteStatusConversion(Friend->GetInviteStatus()),
			Friend->GetPresence().bIsPlayingThisGame);
		FriendsListResult.Add(BPFriend);
	}

	return FriendsListResult;
}

EBPInviteStatus UAccelByteCommonFriendSubsystem::BlueprintableInviteStatusConversion(EInviteStatus::Type InviteStatus)
{
	switch (InviteStatus)
	{
	case EInviteStatus::Type::Unknown:
		return EBPInviteStatus::Unknown;
	case EInviteStatus::Type::Accepted:
		return EBPInviteStatus::Accepted;
	case EInviteStatus::Type::PendingInbound:
		return EBPInviteStatus::PendingInbound;
	case EInviteStatus::Type::PendingOutbound:
		return EBPInviteStatus::PendingOutbound;
	case EInviteStatus::Type::Blocked:
		return EBPInviteStatus::Blocked;
	case EInviteStatus::Type::Suggested:
		return EBPInviteStatus::Suggested;
	default:
		return EBPInviteStatus::Unknown;
	}
}
