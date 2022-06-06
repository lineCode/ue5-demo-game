// Copyright (c) 2018 AccelByte, inc. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteCommonFriendSubsystem.h"
#include "OnlineSubsystem.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_ABFriendsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCompleteQueryFriendsList, FABFriendSubsystemOnlineFriends, FriendsList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFoundSearchUser, FABFriendSubsystemOnlineUser, TargetUser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNotFoundSearchUser);

/**
 *
 */
UCLASS()
class ACCELBYTECOMMONGAME_API UAsyncAction_ABFriendsSubsystemGetFriendList : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Retrieved cached friends, incoming, and outgoing friends request list. If there's no cached data, retrieve from API
	 *
	 * @param LocalPlayer Local player object
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Friends", meta = (BlueprintInternalUseOnly = "true"))
	static UAsyncAction_ABFriendsSubsystemGetFriendList* GetFriendList(ULocalPlayer* LocalPlayer);

	UPROPERTY(BlueprintAssignable)
	FOnCompleteQueryFriendsList OnComplete;

protected:
	virtual void Activate() override;

	void TriggerAction() const;

	TWeakObjectPtr<USocialToolkit> SocialToolkit;
	int32 LocalPlayerIndex;
};

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