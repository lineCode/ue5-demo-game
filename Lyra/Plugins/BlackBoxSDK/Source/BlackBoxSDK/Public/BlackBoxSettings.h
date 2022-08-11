// Copyright (c) 2019 - 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Misc/MessageDialog.h"
#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "BlackBoxLogSeverity.h"
#include "BlackBoxSettings.generated.h"

UENUM()
enum class BlackBoxConfigValE : uint8 {
    Off UMETA(DisplayName = "Off"),
    On UMETA(DisplayName = "On"),
    WebConfig UMETA(DisplayName = "Web Config")
};

struct BlackBoxConsoleCommand {
    static FString ConvertToString(const BlackBoxConfigValE& value);
    static BlackBoxConfigValE ConvertToConfigEnum(const FString& value);

    FString Name{};
    FString Help{};
    BlackBoxConfigValE& SettingVar;
    bool NeedRestartOnChange{};
};

/**
 * @brief UObject for storing settings into configuration file.
 */
UCLASS(Config = Engine)
class BLACKBOXSDK_API UBlackBoxSettings : public UObject {
    GENERATED_BODY()
public:
    UBlackBoxSettings();

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

    UPROPERTY(EditAnywhere, Category = "User Preferences")
    BlackBoxConfigValE EnableBasicProfiling = BlackBoxConfigValE::Off;

    UPROPERTY(EditAnywhere, Category = "User Preferences")
    BlackBoxConfigValE EnableCrashReporter = BlackBoxConfigValE::On;

    UPROPERTY(EditAnywhere, Category = "User Preferences")
    BlackBoxConfigValE EnableHardwareInformationGathering = BlackBoxConfigValE::On;

    UPROPERTY(EditAnywhere, Category = "User Preferences")
    BlackBoxConfigValE EnableStoreCrashVideo = BlackBoxConfigValE::On;

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Experimental")
    bool ExperimentalServerBuildIdFeature = false;

    // <Property Name, Initial Value>
    TMap<FString, int> NeedToRestartOnChangedProperties{};

    TArray<BlackBoxConsoleCommand> ConsoleCommands{};

    void InitializeLocalConfigProperties();

    void InitializeNeedToRestartOnChangeProperties();

    bool CheckNeedToRestart(const FString& ChangedPropertiesName);

    void ApplyLocalConfigProperties();

    void ShowMustRestartDialog();

#if defined(WITH_EDITOR) && WITH_EDITOR
#    if ((ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION < 25))
    virtual bool CanEditChange(const UProperty* InProperty) const override;
#    else
    virtual bool CanEditChange(const FProperty* InProperty) const override;
#    endif
#endif
private:
    void CreateConsoleCommand(BlackBoxConsoleCommand& NewConsoleCommand);
};