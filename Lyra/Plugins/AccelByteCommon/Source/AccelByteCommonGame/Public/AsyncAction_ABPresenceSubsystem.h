// Copyright (c) 2018 AccelByte, inc. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteCommonPresenceSubsystem.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_ABPresenceSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSuccessGetUserPresence, FUniqueNetIdRepl, UserId, FABOnlineUserPresence, UserPresence);

/**
 * 
 */
UCLASS()
class ACCELBYTECOMMONGAME_API UAsyncAction_ABPresenceSubsystem : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	/**
	 * Get a specific user's presence. Will call endpoint.
	 *
	 * @param WorldContextObject World context object
	 * @param TargetUserId Unique Net Id of the target user
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Presence", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UAsyncAction_ABPresenceSubsystem* GetUserPresence(UObject* WorldContextObject, FUniqueNetIdRepl TargetUserId);

	UPROPERTY(BlueprintAssignable)
	FOnSuccessGetUserPresence OnComplete;

protected:

	virtual void Activate() override;
	FUniqueNetIdPtr TargetUserId;
	UAccelByteCommonPresenceSubsystem* PresenceSubsystem;
};
