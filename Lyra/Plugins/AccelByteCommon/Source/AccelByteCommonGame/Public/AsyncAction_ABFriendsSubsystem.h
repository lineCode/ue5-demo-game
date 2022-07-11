// Copyright (c) 2018 AccelByte, inc. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteCommonFriendSubsystem.h"
#include "OnlineSubsystem.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_ABFriendsSubsystem.generated.h"

#pragma region Blueprintable structs

USTRUCT(BlueprintType)
struct FABFriendSubsystemOnlineFriends
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TArray<FABFriendSubsystemOnlineFriend> Content;

	FABFriendSubsystemOnlineFriends(){}

	FABFriendSubsystemOnlineFriends(TArray<FABFriendSubsystemOnlineFriend> Content): Content(Content){}
};

#pragma endregion

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFoundSearchUser, FABFriendSubsystemOnlineUser, TargetUser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGetFriendsListComplete, FABFriendSubsystemOnlineFriends, FriendsList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNotFoundSearchUser);

/**
 *
 */
UCLASS()
class ACCELBYTECOMMONGAME_API UAsyncAction_ABFriendsSubsystemSearchUser : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Search user by exact user name
	 *
	 * @param WorldContextObject World context object
	 * @param LocalTargetUserNum Local user index
	 * @param DisplayName Display name to be search
	 * @param bHideSelf whether to hide Local user from the search result or not
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UAsyncAction_ABFriendsSubsystemSearchUser* SearchUserByExactDisplayName(
		UObject* WorldContextObject, int32 LocalTargetUserNum, FString DisplayName, bool bHideSelf = true);

	UPROPERTY(BlueprintAssignable)
	FOnFoundSearchUser OnFound;

	UPROPERTY(BlueprintAssignable)
	FOnNotFoundSearchUser OnNotFound;

protected:
	virtual void Activate() override;
	IOnlineSubsystem* OSS;
	int32 LocalTargetUserNum;
	FString DisplayName;
	bool bHideSelf;
};

/**
 *
 */
UCLASS()
class ACCELBYTECOMMONGAME_API UAsyncAction_ABFriendsSubsystemGetFriendsList : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Attempt to retrieve cached friends list. If hasn't been cached yet, retrieve from endpoint
	 *
	 * @param Target AccelByte Common Party Subsystem object
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends", meta = (BlueprintInternalUseOnly = "true"))
	static UAsyncAction_ABFriendsSubsystemGetFriendsList* GetFriendsList(UAccelByteCommonFriendSubsystem* Target, int32 LocalPlayerIndex);

	UPROPERTY(BlueprintAssignable)
	FOnGetFriendsListComplete OnComplete;

protected:
	virtual void Activate() override;
	UAccelByteCommonFriendSubsystem* FriendSubsystem;
	int32 LocalPlayerIndex;
};