// Copyright (c) 2018 AccelByte, inc. All rights reserved.


#include "AsyncAction_ABPresenceSubsystem.h"

UAsyncAction_ABPresenceSubsystem* UAsyncAction_ABPresenceSubsystem::GetUserPresence(UObject* WorldContextObject,
	FUniqueNetIdRepl TargetUserId)
{
	UAsyncAction_ABPresenceSubsystem* Action = NewObject<UAsyncAction_ABPresenceSubsystem>();

	if (WorldContextObject)
	{
		UAccelByteCommonPresenceSubsystem* PresenceSubsystem =
			WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UAccelByteCommonPresenceSubsystem>();
		check(PresenceSubsystem)

		Action->PresenceSubsystem = PresenceSubsystem;
		Action->TargetUserId = TargetUserId.GetUniqueNetId();
	}
	else
	{
		Action->SetReadyToDestroy();
	}

	return Action;
}

void UAsyncAction_ABPresenceSubsystem::Activate()
{
	Super::Activate();

	PresenceSubsystem->QueryUserPresence(TargetUserId, TDelegate<void(bool)>::CreateWeakLambda(
		this, [this](bool bWasSuccessful)
		{
			FABOnlineUserPresence UserPresence;

			if (bWasSuccessful)
			{
				UserPresence = PresenceSubsystem->GetCachedUserPresence(TargetUserId);
			}
			else
			{
				UserPresence.State = EABOnlinePresenceState::Offline;
			}

			OnComplete.Broadcast(TargetUserId, UserPresence);
		}));
}
