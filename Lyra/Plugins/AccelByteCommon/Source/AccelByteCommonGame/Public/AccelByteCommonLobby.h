// "// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.// This is licensed software from AccelByte Inc, for limitations// and restrictions contact your company contract manager."

#pragma once

#include "CoreMinimal.h"
#include "CommonUserSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AccelByteCommonLobby.generated.h"

class USocialManager;

// TODO: Make extension for SocialToolkit (mandatory)!

/**
 * Lobby, Party stuff
 */
UCLASS()
class ACCELBYTECOMMONGAME_API UAccelByteCommonLobby : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	UFUNCTION()
	virtual void HandleUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);
	
};
