// Copyright (c) 2019 - 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Misc/MessageDialog.h"
#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "BlackBoxLogSeverity.h"
#include "BlackBoxSettings.generated.h"

/**
 * @brief UObject for storing settings into configuration file.
 */
UCLASS(Config = Engine)
class BLACKBOXSDK_API UBlackBoxSettings : public UObject {
    GENERATED_BODY()
public:
    UBlackBoxSettings() = default;

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    FString APIKey{};

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    FString GameVersionID{};

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    FString Namespace{};

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    BlackBoxLogSeverity LogSeverity{};

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    bool EnableLog{};

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    bool EnableHardwareInformationGathering = true;

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    bool ExperimentalServerBuildIdFeature = false;

    bool InitialExperimentalServerBuildIdFeature = false;
    static void ShowMustRestartDialog()
    {
        const FText MustRestartTitle = FText::FromString("BlackBox Setting");
        const FText MustRestartMessage =
            FText::FromString("Unreal Editor must be restarted for the changes to take effect");
        FMessageDialog::Open(EAppMsgType::Type::Ok, MustRestartMessage, &MustRestartTitle);
    }

#if defined(WITH_EDITOR) && WITH_EDITOR
#    if ((ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION < 25))
    virtual bool CanEditChange(const UProperty* InProperty) const override;
#    else
    virtual bool CanEditChange(const FProperty* InProperty) const override;
#    endif
#endif
};