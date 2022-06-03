// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager."

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemTypes.h"
#include "UObject/NoExportTypes.h"
#include "AccelByteCommonFriendSubsystem.generated.h"

#pragma region Blueprintable structs

UENUM(BlueprintType)
enum class EABFriendSubsystemInviteStatus : uint8
{
	/** unknown state */
	Unknown,
	/** Friend has accepted the invite */
	Accepted,
	/** Friend has sent player an invite, but it has not been accepted/rejected */
	PendingInbound,
	/** Player has sent friend an invite, but it has not been accepted/rejected */
	PendingOutbound,
	/** Player has been blocked */
	Blocked,
	/** Suggested friend */
	Suggested
};

USTRUCT(BlueprintType)
struct FABFriendSubsystemOnlineUser
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FUniqueNetIdRepl UserId;

	UPROPERTY(BlueprintReadOnly)
	FText DisplayName;

	FABFriendSubsystemOnlineUser()
	{
	}

	FABFriendSubsystemOnlineUser(
		const FUniqueNetIdRepl UserId,
		const FText& DisplayName) :
	UserId(UserId),
	DisplayName(DisplayName)
	{
	}
};

USTRUCT(BlueprintType)
struct FABFriendSubsystemOnlineFriend
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FABFriendSubsystemOnlineUser UserInfo;

	UPROPERTY(BlueprintReadOnly)
	FText LoggedInPlatform;

	UPROPERTY(BlueprintReadOnly)
	EABFriendSubsystemInviteStatus InviteStatus = EABFriendSubsystemInviteStatus::Unknown;

	UPROPERTY(BlueprintReadOnly)
	bool bIsPlayingThisGame = false;

	FABFriendSubsystemOnlineFriend()
	{
	}

	FABFriendSubsystemOnlineFriend(
		const FUniqueNetIdRepl UserId,
		const FText& DisplayName,
		const FText& LoggedInPlatform,
		const EABFriendSubsystemInviteStatus InviteStatus,
		const bool& bIsPlayingThisGame) :
	UserInfo(UserId, DisplayName),
	LoggedInPlatform(LoggedInPlatform),
	InviteStatus(InviteStatus),
	bIsPlayingThisGame(bIsPlayingThisGame)
	{
	}

	FABFriendSubsystemOnlineFriend(
		const FUniqueNetIdRepl UserId,
		const FText& DisplayName) :
	UserInfo(UserId, DisplayName)
	{
	}
};

/**
 * Blueprint cannot use delegate with TArray param
 * This struct is a workaround for that
 */
USTRUCT(BlueprintType)
struct FABFriendSubsystemOnlineFriends
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TArray<FABFriendSubsystemOnlineFriend> Data;
};

#pragma endregion

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnCompleteGetFriendsList, FABFriendSubsystemOnlineFriends, FriendsList);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnCompleteQueryUserMapping, FABFriendSubsystemOnlineUser, FoundUser);
DECLARE_DYNAMIC_DELEGATE(FFriendVoidDelegate);

/**
 * Friends related services Blueprint-able wrapper
 */
UCLASS()
class UAccelByteCommonFriendSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Attempt to retrieve cached Friends list (accepted and pending request). If does not exist, retrieve from endpoint.
	 *
	 * @param LocalTargetUserNum Local player number
	 * @param OnComplete Delegate that will be called upon completion
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void GetFriendsList(int32 LocalTargetUserNum, FOnCompleteGetFriendsList OnComplete);

	/**
	 * Search AccelByte user by the exact username.
	 *
	 * @param LocalTargetUserNum Local player number
	 * @param DisplayName Username to be search
	 * @param OnComplete Delegate that will be called upon completion
	 * @param OnNotFound Delegate that will be called upon user not found
	 * @param bHideSelf Whether to show this user's in search result or not
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void SearchUserByExactDisplayName(
		int32 LocalTargetUserNum,
		FString DisplayName,
		FOnCompleteQueryUserMapping OnComplete,
		FFriendVoidDelegate OnNotFound,
		bool bHideSelf = true);

	/**
	 * Send friend request to another user.
	 *
	 * @param LocalTargetUserNum Local player number
	 * @param TargetUniqueId Target user Unique Id
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void SendFriendRequest(int32 LocalTargetUserNum, FUniqueNetIdRepl TargetUniqueId);

	/**
	 * Accept pending received friend request.
	 *
	 * @param LocalTargetUserNum Local player number
	 * @param SenderUniqueId Friend request sender Unique Id
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void AcceptFriendRequest(int32 LocalTargetUserNum, FUniqueNetIdRepl SenderUniqueId);

	/**
	 * Reject pending received friend request.
	 *
	 * @param LocalTargetUserNum Local player number
	 * @param SenderUniqueId Friend request sender Unique Id
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void RejectFriendRequest(int32 LocalTargetUserNum, FUniqueNetIdRepl SenderUniqueId);

	/**
	 * Unfriend or cancel sent friend request to a user.
	 *
	 * @param LocalTargetUserNum Local player number
	 * @param TargetUniqueId Target user Unique Id
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void RemoveFriend(int32 LocalTargetUserNum, FUniqueNetIdRepl TargetUniqueId);

	/**
	 * Set OnFriendListChange delegate. Will be called everytime Friends list changes.
	 *
	 * @param LocalTargetUserNum Local player number
	 * @param OnChange Delegate that will be executed
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void OnFriendListChange(int32 LocalTargetUserNum, FFriendVoidDelegate OnChange);

private:

	static TArray<FABFriendSubsystemOnlineFriend> BlueprintableFriendsDataConversion(TArray<TSharedRef<FOnlineFriend>>& FriendsList);

	static EABFriendSubsystemInviteStatus BlueprintableInviteStatusConversion(EInviteStatus::Type InviteStatus);

	IOnlineSubsystem* OSS;
};
