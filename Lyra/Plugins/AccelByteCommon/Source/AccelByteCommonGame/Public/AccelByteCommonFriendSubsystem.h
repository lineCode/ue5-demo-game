// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager."

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "UObject/NoExportTypes.h"
#include "User/SocialUser.h"
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
	FString DisplayName;

	FABFriendSubsystemOnlineUser()
	{
	}

	FABFriendSubsystemOnlineUser(
		const FUniqueNetIdRepl UserId,
		const FString& DisplayName) :
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
	FString LoggedInPlatform;

	UPROPERTY(BlueprintReadOnly)
	EABFriendSubsystemInviteStatus InviteStatus = EABFriendSubsystemInviteStatus::Unknown;

	UPROPERTY(BlueprintReadOnly)
	bool bIsPlayingThisGame = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsInParty = false;

	FABFriendSubsystemOnlineFriend()
	{
	}

	FABFriendSubsystemOnlineFriend(
		const FUniqueNetIdRepl UserId,
		const FString& DisplayName,
		const FString& LoggedInPlatform,
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
		const FString& DisplayName) :
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

DECLARE_DYNAMIC_DELEGATE_TwoParams(FFriendPresenceChange, bool, bIsPlayingThisGame, FString, LoggedInPlatform);
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
	 * Get Display Name and Logged In platform of local user from SocialToolkit
	 *
	 * @param DisplayName Display name output
	 * @param Platform Logged in platform output
	 * @param LocalPlayer Local player object
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void GetLocalUserDisplayNameAndPlatform(FString& DisplayName, FString& Platform, ULocalPlayer* LocalPlayer);

	/**
	 * Send friend request to another user.
	 *
	 * @param LocalPlayer Target local player object
	 * @param DisplayName Exact DisplayName of the user to be requested
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void SendFriendRequest(ULocalPlayer* LocalPlayer, FString DisplayName);

	/**
	 * Accept pending received friend request.
	 *
	 * @param LocalPlayer Local player object
	 * @param SenderUniqueId Friend request sender Unique Id
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void AcceptFriendRequest(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl SenderUniqueId);

	/**
	 * Reject pending received friend request.
	 *
	 * @param LocalPlayer Local player object
	 * @param SenderUniqueId Friend request sender Unique Id
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void RejectFriendRequest(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl SenderUniqueId);

	/**
	 * Unfriend a user.
	 *
	 * @param LocalPlayer Local player object
	 * @param TargetUniqueId Target user Unique Id
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void Unfriend(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl TargetUniqueId);

	/**
	 * Cancel sent friend request
	 *
	 * @param LocalPlayer Local player object
	 * @param TargetUniqueId Target user Unique Id
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void CancelSentFriendRequest(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl TargetUniqueId);

	/**
	 * Set OnFriendListChange delegate. Will be called everytime Friends list changes.
	 *
	 * @param LocalPlayer Local player object
	 * @param OnListChange Delegate that will be executed upon friends, incoming request, and outgoing request list change
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void OnFriendsListChange(ULocalPlayer* LocalPlayer, FFriendVoidDelegate OnListChange);

	/**
	 * Set OnFriendListChange delegate. Will be called everytime Friends list changes.
	 *
	 * @param LocalPlayer Local player object
	 * @param TargetUniqueId Target user Unique Id
	 * @param OnPresenceChange Delegate that will be executed upon friend's presence change
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void OnUserPresenceChange(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl TargetUniqueId, FFriendPresenceChange OnPresenceChange);

	static TArray<FABFriendSubsystemOnlineFriend> BlueprintableSocialUserListConversion(TArray<USocialUser*> SocialUsers);

private:

	static EABFriendSubsystemInviteStatus BlueprintableInviteStatusConversion(EInviteStatus::Type InviteStatus);
};
