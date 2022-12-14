// Copyright (c) 2020-2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "BlackBoxSettings.h"
#include "BlackBoxLog.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/IConsoleManager.h"
#include "accelbyte/cpp/blackbox.h"

namespace BlackboxSDK {
bool IsAPIKeyOverriden = true;
bool IsGameVersionIDOverriden = true;
bool IsNamespaceOverriden = true;
} // namespace BlackboxSDK

FString BlackBoxConsoleCommand::ConvertToString(const BlackBoxConfigValE& value)
{
    if (value == BlackBoxConfigValE::On) {
        return "1";
    }
    else if (value == BlackBoxConfigValE::Off) {
        return "0";
    }

    return "webconfig";
}

BlackBoxConfigValE BlackBoxConsoleCommand::ConvertToConfigEnum(const FString& value)
{
    if (value.Equals("1")) {
        return BlackBoxConfigValE::On;
    }
    else if (value.Equals("0")) {
        return BlackBoxConfigValE::Off;
    }

    return BlackBoxConfigValE::WebConfig;
}

void UBlackBoxSettings::ShowMustRestartDialog()
{
    const FText MustRestartTitle = FText::FromString("BlackBox Setting");
    const FText MustRestartMessage =
        FText::FromString("Unreal Editor must be restarted for the changes to take effect");
    FMessageDialog::Open(EAppMsgType::Type::Ok, MustRestartMessage, &MustRestartTitle);
}

UBlackBoxSettings::UBlackBoxSettings()
{
    ConsoleCommands.Add(BlackBoxConsoleCommand{
        TEXT("BlackBox.SetBasicProfiling"), TEXT("Set BlackBox enable profiling local config"), EnableBasicProfiling});
    ConsoleCommands.Add(BlackBoxConsoleCommand{
        TEXT("BlackBox.SetCrashReporter"),
        TEXT("Set BlackBox enable crash reporter local config"),
        EnableCrashReporter});
    ConsoleCommands.Add(BlackBoxConsoleCommand{
        TEXT("BlackBox.SetHardwareInformationGathering"),
        TEXT("Set BlackBox enable hardware info gathering local config"),
        EnableHardwareInformationGathering,
        true});
    ConsoleCommands.Add(BlackBoxConsoleCommand{
        TEXT("BlackBox.SetStoreCrashVideo"),
        TEXT("Set BlackBox enable store crash video local config"),
        EnableStoreCrashVideo});

    for (auto& NewCommand : ConsoleCommands) { CreateConsoleCommand(NewCommand); }
}

void UBlackBoxSettings::InitializeLocalConfigProperties()
{
    blackbox::load_local_config(TCHAR_TO_UTF8(FApp::GetProjectName()));

    FString EnableCrashReporterStr = UTF8_TO_TCHAR(blackbox::local_config_get_enable_crash_reporter());
    FString EnableStoreCrashVideoStr = UTF8_TO_TCHAR(blackbox::local_config_get_store_crash_video());
    FString EnableHardwareGatheringStr = UTF8_TO_TCHAR(blackbox::local_config_get_store_dxdiag());
    FString EnableProfilingStr = UTF8_TO_TCHAR(blackbox::local_config_get_enable_basic_profiling());

    /*Backward Support*/
    if (EnableHardwareGatheringStr.IsEmpty()) {
        EnableHardwareGatheringStr = "1";
        bool HardwareGatheringValue{};
        FString EngineConfig = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
        if (GConfig->GetBool(
                TEXT("/Script/BlackBoxSDK.BlackBoxSettings"),
                TEXT("EnableHardwareInformationGathering"),
                HardwareGatheringValue,
                EngineConfig)) {
            if (!HardwareGatheringValue) {
                EnableHardwareGatheringStr = "0";
            }
        }
        blackbox::local_config_set_store_dxdiag(TCHAR_TO_UTF8(*EnableHardwareGatheringStr));
    }

    EnableCrashReporter = BlackBoxConsoleCommand::ConvertToConfigEnum(EnableCrashReporterStr);
    EnableStoreCrashVideo = BlackBoxConsoleCommand::ConvertToConfigEnum(EnableStoreCrashVideoStr);
    EnableHardwareInformationGathering = BlackBoxConsoleCommand::ConvertToConfigEnum(EnableHardwareGatheringStr);
    EnableBasicProfiling = BlackBoxConsoleCommand::ConvertToConfigEnum(EnableProfilingStr);
}

void UBlackBoxSettings::InitializeNeedToRestartOnChangeProperties()
{
    NeedToRestartOnChangedProperties.Add("ExperimentalServerBuildIdFeature", (int)ExperimentalServerBuildIdFeature);
    NeedToRestartOnChangedProperties.Add("EnableHardwareInformationGathering", (int)EnableHardwareInformationGathering);
}

bool UBlackBoxSettings::CheckNeedToRestart(const FString& ChangedPropertiesName)
{
    if (!NeedToRestartOnChangedProperties.Contains(ChangedPropertiesName)) {
        return false;
    }

    if (ChangedPropertiesName == "ExperimentalServerBuildIdFeature" &&
        NeedToRestartOnChangedProperties["ExperimentalServerBuildIdFeature"] != (int)ExperimentalServerBuildIdFeature) {
        return true;
    }
    else if (
        ChangedPropertiesName == "EnableHardwareInformationGathering" &&
        NeedToRestartOnChangedProperties["EnableHardwareInformationGathering"] !=
            (int)EnableHardwareInformationGathering) {
        return true;
    }

    return false;
}

void UBlackBoxSettings::ApplyLocalConfigProperties()
{
    blackbox::local_config_set_enable_crash_reporter(
        TCHAR_TO_UTF8(*BlackBoxConsoleCommand::ConvertToString(EnableCrashReporter)));
    blackbox::local_config_set_store_dxdiag(
        TCHAR_TO_UTF8(*BlackBoxConsoleCommand::ConvertToString(EnableHardwareInformationGathering)));
    blackbox::local_config_set_store_crash_video(
        TCHAR_TO_UTF8(*BlackBoxConsoleCommand::ConvertToString(EnableStoreCrashVideo)));
    blackbox::local_config_set_enable_basic_profiling(
        TCHAR_TO_UTF8(*BlackBoxConsoleCommand::ConvertToString(EnableBasicProfiling)));
}

void UBlackBoxSettings::CreateConsoleCommand(BlackBoxConsoleCommand& NewConsoleCommand)
{
    IConsoleManager::Get().RegisterConsoleCommand(
        *NewConsoleCommand.Name,
        *NewConsoleCommand.Help,
        FConsoleCommandWithArgsDelegate::CreateLambda([this, &NewConsoleCommand](const TArray<FString>& args) {
            if (args.Num() == 0) {
                UE_LOG(LogBlackBox, Error, TEXT("Value must be not empty. Expected: [off, on, webconfig)"));
                return;
            }

            auto value = args[0].ToLower();
            if (value == "on") {
                NewConsoleCommand.SettingVar = BlackBoxConfigValE::On;
            }
            else if (value == "off") {
                NewConsoleCommand.SettingVar = BlackBoxConfigValE::Off;
            }
            else if (value == "webconfig") {
                NewConsoleCommand.SettingVar = BlackBoxConfigValE::WebConfig;
            }
            else {
                UE_LOG(LogBlackBox, Error, TEXT("Value not recognized. Expected: [off, on, webconfig)"));
                return;
            }

            if (NewConsoleCommand.NeedRestartOnChange) {
                UE_LOG(LogBlackBox, Warning, TEXT("Unreal must be restarted for the changes to take effect"));
            }

            ApplyLocalConfigProperties();
            blackbox::save_local_config(TCHAR_TO_UTF8(FApp::GetProjectName()));
        }));
}

#if WITH_EDITOR

#    if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION < 25)
bool UBlackBoxSettings::CanEditChange(const UProperty* InProperty) const
#    else
bool UBlackBoxSettings::CanEditChange(const FProperty* InProperty) const
#    endif
{
#    if PLATFORM_WINDOWS
    if (InProperty->GetName() == FString("APIKey"))
        return !BlackboxSDK::IsAPIKeyOverriden;
    else if (InProperty->GetName() == FString("GameVersionID")) {
        return !BlackboxSDK::IsGameVersionIDOverriden;
    }
    else if (InProperty->GetName() == FString("Namespace")) {
        return !BlackboxSDK::IsNamespaceOverriden;
    }
    else
        return true;
#    else
    return false;
#    endif
}
#endif
