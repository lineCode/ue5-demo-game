// Copyright (c) 2019 - 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Misc/MessageDialog.h"
#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "AccelByteEndgameSettings.generated.h"
/*
* @brief UObject for storing settings into configuration file.
*/
UCLASS(Config = Engine)
class UAccelbyteEndgameSettings : public UObject {
    GENERATED_BODY()
public:
    UAccelbyteEndgameSettings() = default;

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    FString BaseAddress {};

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    FString APIKey {};

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    FString GameID {};

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    FString NamespaceID {};
};
