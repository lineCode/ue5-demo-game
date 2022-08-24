// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager."

#pragma once

#include "CoreMinimal.h"
#include "AccelByteCommonPresenceSubsystem.h"
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
	EABFriendSubsystemInviteStatus InviteStatus = EABFriendSubsystemInviteStatus::Unknown;

	UPROPERTY(BlueprintReadWrite)
	FABOnlineUserPresence Presence;

	UPROPERTY(BlueprintReadOnly)
	bool bIsInParty = false;

	FABFriendSubsystemOnlineFriend()
	{
	}

	FABFriendSubsystemOnlineFriend(
		const FUniqueNetIdRepl UserId,
		const FString& DisplayName,
		const FString& LoggedInPlatform,
		const EABFriendSubsystemInviteStatus InviteStatus) :
	UserInfo(UserId, DisplayName),
	InviteStatus(InviteStatus)
	{
	}

	FABFriendSubsystemOnlineFriend(
		const FUniqueNetIdRepl UserId,
		const FString& DisplayName) :
	UserInfo(UserId, DisplayName)
	{
	}
};

#pragma endregion

DECLARE_LOG_CATEGORY_CLASS(LogAccelByteCommonFriend, Log, All);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FFriendPresenceChangeDelegate, bool, bIsPlayingThisGame, FString, LoggedInPlatform);
DECLARE_DYNAMIC_DELEGATE(FFriendVoidDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFriendMultiCastDelegate);

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
	 * Get Cached Friends, pending incoming and outgoing friend request list
	 *
	 * @param LocalPlayerIndex Local player index
	 * @param OnComplete Delegate that will be executed upon completion
	 */
	void GetFriendsList(const TDelegate<void(TArray<FABFriendSubsystemOnlineFriend>&)>& OnComplete, int32 LocalPlayerIndex = 0);

	/**
	 * Get Friend object by Unique Net Id
	 *
	 * @param OutFoundFriend Friend object output
	 * @param TargetId Unique Id to be search
	 * @param LocalPlayerIndex Local player index
	 *
	 * @return whether Friend object found or not
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends", meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool GetFriendByUniqueNetId(FABFriendSubsystemOnlineFriend& OutFoundFriend, const FUniqueNetIdRepl TargetId, int32 LocalPlayerIndex = 0);

	/**
	 * Get Display Name and Logged In platform of local user from SocialToolkit
	 *
	 * @param OutDisplayName Display name output
	 * @param OutPlatform Logged in platform output
	 * @param LocalPlayerIndex
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void GetLocalUserDisplayNameAndPlatform(FString& OutDisplayName, FString& OutPlatform, const int32 LocalPlayerIndex = 0) const;

	/**
	 * Send friend request to another user.
	 *
	 * @param TargetId Unique ID of the user to be requested
	 * @param LocalPlayerIndex
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void SendFriendRequest(FUniqueNetIdRepl TargetId, int32 LocalPlayerIndex = 0);

	/**
	 * Accept pending received friend request.
	 *
	 * @param SenderUniqueId Friend request sender Unique Id
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void AcceptFriendRequest(FUniqueNetIdRepl SenderUniqueId, const int32 LocalPlayerIndex = 0);

	/**
	 * Reject pending received friend request.
	 *
	 * @param SenderUniqueId Friend request sender Unique Id
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void RejectFriendRequest(FUniqueNetIdRepl SenderUniqueId, const int32 LocalPlayerIndex = 0);

	/**
	 * Unfriend or cancel sent friend request
	 *
	 * @param TargetUniqueId Target user Unique Id
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends")
	void DeleteFriend(FUniqueNetIdRepl TargetUniqueId, const int32 LocalPlayerIndex = 0);

	/**
	 * Convert FOnlineFriend to it's Blueprint's equivalent, FABFriendSubsystemOnlineFriend
	 *
	 * @param OnlineFriends Array of FOnlineFriend to be converted
	 *
	 * @return Converted array
	 */
	static TArray<FABFriendSubsystemOnlineFriend> BlueprintableSocialUserListConversion(TArray<TSharedRef<FOnlineFriend>> OnlineFriends);

	/**
	 * Execute when friends, pending incoming, and pending outgoing list changed
	 */
	UPROPERTY(BlueprintAssignable)
	FFriendMultiCastDelegate OnFriendsListChange;

private:
	static EABFriendSubsystemInviteStatus BlueprintableInviteStatusConversion(EInviteStatus::Type InviteStatus);

	static FABFriendSubsystemOnlineFriend BlueprintableOnlineFriendConversion(TSharedPtr<FOnlineFriend> OnlineFriend);

	void SetFriendsNotifDelegates();

	IOnlineSubsystem* OSS;

	static const inline FString PlatformType_Password = "PC";
	static const inline FString FriendsListName = "";
};
