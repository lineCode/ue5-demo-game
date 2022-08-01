// Copyright (c) 2020-2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "InformationManager.h"
#include "BlackBoxCommon.h"
#include "BlackBoxLog.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Misc/EngineVersion.h"
#include "accelbyte/cpp/blackbox.h"
#if BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX
#    include <XUser.h>
#    include <GDKUserManager.h>
#    include <string>
#elif BLACKBOX_UE_SONY
#    include <sdk_version.h>
#endif

#define _BLACKBOX_STRINGIZE(s) #s
#define BLACKBOX_STRINGIZE(s) _BLACKBOX_STRINGIZE(s)

FInfoManager::FInfoManager()
{
}

FInfoManager::~FInfoManager()
{
}

InputInformation& FInfoManager::GetKeyInformation()
{
    return InputInfo;
}

void FInfoManager::SetupKeyInformation(APlayerController* PlayerCtrl)
{
#if BLACKBOX_UE_WINDOWS
    if (!IsValid(PlayerCtrl) || PlayerCtrl->PlayerInput == nullptr || !IsValid(PlayerCtrl->PlayerInput)) {
        return;
    }
    // Reset everything first
    InputInfo.ActionKeyPair.Empty();
    TArray<FInputActionKeyMapping> ActionMaps = PlayerCtrl->PlayerInput->ActionMappings;
    TArray<FInputAxisKeyMapping> AxisMaps = PlayerCtrl->PlayerInput->AxisMappings;

    InputInfo.ActionKeyPair.Emplace(TEXT("Shift Modifier"), EKeys::LeftShift);
    InputInfo.ActionKeyPair.Emplace(TEXT("Shift Modifier"), EKeys::RightShift);
    InputInfo.ActionKeyPair.Emplace(TEXT("Alt Modifier"), EKeys::LeftAlt);
    InputInfo.ActionKeyPair.Emplace(TEXT("Alt Modifier"), EKeys::RightAlt);
    InputInfo.ActionKeyPair.Emplace(TEXT("Ctrl Modifier"), EKeys::LeftControl);
    InputInfo.ActionKeyPair.Emplace(TEXT("Ctrl Modifier"), EKeys::RightControl);

    // Add analog inputs
    for (const auto& map : AxisMaps) {
        InputInfo.ActionKeyPair.Emplace(
            FString::Printf(TEXT("%s (%+.2f)"), *map.AxisName.ToString(), map.Scale), map.Key);
        // we currently only support 64 bit key mapping
        if (InputInfo.ActionKeyPair.Num() >= 64) {
            break;
        }
    }

    // Add digital inputs
    for (const auto& map : ActionMaps) {
        InputInfo.ActionKeyPair.Emplace(map.ActionName.ToString(), map.Key);
        // we currently only support 64 bit key mapping
        if (InputInfo.ActionKeyPair.Num() >= 64) {
            break;
        }
    }
    std::vector<std::string> action_names_str;
    action_names_str.resize(InputInfo.ActionKeyPair.Num());
    std::vector<const char*> action_names;
    for (UINT64 iKey = 0; iKey < InputInfo.ActionKeyPair.Num(); iKey++) {
        action_names_str[iKey] = TCHAR_TO_UTF8(*(InputInfo.ActionKeyPair[iKey].Key));
        action_names.emplace_back(action_names_str[iKey].c_str());
    }
    blackbox::setup_key_input(action_names.data(), action_names.size());
#endif
}

bool FInfoManager::IsKeyInformationPresent()
{
    return InputInfo.ActionKeyPair.Num() != 0;
}

void FInfoManager::ResetKeyInformation()
{
#if BLACKBOX_UE_WINDOWS
    InputInfo.ActionKeyPair.Empty();
    blackbox::setup_key_input(nullptr, 0);
#endif
}

CPUInformation& FInfoManager::GetCPUInformation()
{
    return CPUInfo;
}

void FInfoManager::SetCPUInformation(const CPUInformation& CPUInfo_)
{
    this->CPUInfo = CPUInfo_;
}

GPUInformation& FInfoManager::GetGPUInformation()
{
    return GPUInfo;
}

void FInfoManager::SetGPUInformation(const GPUInformation& GPUInfo_)
{
    this->GPUInfo = GPUInfo_;
    blackbox::config_set_rendering_gpu_name(TCHAR_TO_UTF8(*GPUInfo.Model));
    blackbox::config_set_gpu_ver(TCHAR_TO_UTF8(*GPUInfo.DriverVer));
}

OSInformation& FInfoManager::GetOSInformation()
{
    if (IsOSInfoEmpty()) {
        OSInfo.Architecture = FString(ANSI_TO_TCHAR(blackbox::info_get_os_architecture()));
        OSInfo.ComputerName = FString(ANSI_TO_TCHAR(blackbox::info_get_computer_name()));
        OSInfo.Name = FString(ANSI_TO_TCHAR(blackbox::info_get_os_name()));
        OSInfo.UserName = FString(ANSI_TO_TCHAR(blackbox::info_get_host_user_name()));
        OSInfo.Version = FString(ANSI_TO_TCHAR(blackbox::info_get_os_version()));
        return OSInfo;
    }
    else {
        return OSInfo;
    }
}

void FInfoManager::SetOSInformation(const OSInformation& OSInfo_)
{
    this->OSInfo = OSInfo_;
}

ConfigInformation FInfoManager::GetConfigInformation()
{
    ConfigInformation ConfigInfo;
    ConfigInfo.FPS = blackbox::config_get_fps();
    ConfigInfo.KPS = blackbox::config_get_kps();
    ConfigInfo.TotalRecordingSecond = blackbox::config_get_total_second();
    ConfigInfo.SubtitleType = UTF8_TO_TCHAR(blackbox::config_get_subtitle_type());
    ConfigInfo.EnableCrashReporter = blackbox::config_get_enable_crash_reporter();
    ConfigInfo.StoreDXDiag = blackbox::config_get_store_dxdiag();
    ConfigInfo.StoreCrashVideo = blackbox::config_get_store_crash_video();
    ConfigInfo.EnableBasicProfiling = blackbox::config_get_enable_basic_profiling();
    ConfigInfo.EnableCPUProfiling = blackbox::config_get_enable_cpu_profiling();
    ConfigInfo.EnableGPUProfiling = blackbox::config_get_enable_gpu_profiling();
    ConfigInfo.EnableMemoryProfiling = blackbox::config_get_enable_memory_profiling();

    return ConfigInfo;
}

SDKInformation FInfoManager::GetSDKInformation()
{
    SDKInformation SDKInfo;
    SDKInfo.APIKey = UTF8_TO_TCHAR(blackbox::config_get_api_key());
    SDKInfo.BaseUrl = UTF8_TO_TCHAR(blackbox::config_get_base_url());
    SDKInfo.IamUrl = UTF8_TO_TCHAR(blackbox::config_get_iam_url());
    SDKInfo.DownloadURL = UTF8_TO_TCHAR(blackbox::config_get_sdk_download_url());
    SDKInfo.LatestReleaseURL = UTF8_TO_TCHAR(blackbox::config_get_release_json_url());
    SDKInfo.GameVersionId = UTF8_TO_TCHAR(blackbox::config_get_game_version_id());
    SDKInfo.ProjectId = UTF8_TO_TCHAR(blackbox::config_get_project_id());
    SDKInfo.Namespace = UTF8_TO_TCHAR(blackbox::config_get_namespace());
    SDKInfo.BuildId = UTF8_TO_TCHAR(blackbox::config_get_build_id());
    SDKInfo.CoreSDKConfigPath = UTF8_TO_TCHAR(blackbox::config_get_config_path());

    return SDKInfo;
}

void FInfoManager::SetSDKInformation(const SDKInformation& SDKInfo)
{
    UpdateBlackBoxConfiguration(SDKInfo);
}

ProcessInformation& FInfoManager::GetProcessInformation()
{
    return ProcInfo;
}

void FInfoManager::SetProcessInformation(const ProcessInformation& ProcInfo_)
{
    this->ProcInfo = ProcInfo_;
    UpdateBlackBoxClientInformation(this->ProcInfo);
}

#if BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX
std::string FInfoManager::GetGamertag()
{
    UE_LOG(LogBlackBox, Log, TEXT("Gamertag fetch starting"));
    std::string Gamertag = "Xbox User";
    // Use 0 seat index since UE4 always attempts to put the default user on seat 0
#    if ENGINE_MAJOR_VERSION == 4
    FPlatformUserId SeatIndex = 0;
#    else
    FPlatformUserId SeatIndex = FPlatformMisc::GetPlatformUserForUserIndex(0);
#    endif

    UE_LOG(LogBlackBox, Log, TEXT("Fetching default user handle..."));
    FGDKUserHandle UserHandle = FGDKUserManager::Get().GetUserHandleByPlatformId(SeatIndex);
    if (UserHandle == NULL) {
        UE_LOG(LogBlackBox, Warning, TEXT("Cannot fetch default user handle, using placeholder username"));
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("Default user handle successfully fetched, fetching user gamertag..."));
        const size_t GamertagLen = 32;
        char GamertagBuffer[GamertagLen]{};
        size_t GamertagBufSize = sizeof(GamertagBuffer);
        if (SUCCEEDED(XUserGetGamertag(
                UserHandle, XUserGamertagComponent::Classic, GamertagLen, GamertagBuffer, &GamertagBufSize))) {
            UE_LOG(LogBlackBox, Log, TEXT("Gamertag successfully fetched"));
            Gamertag = GamertagBuffer;
        }
        else {
            UE_LOG(LogBlackBox, Warning, TEXT("Cannot fetch gamertag, using placeholder username"));
        }
    }
    UE_LOG(LogBlackBox, Log, TEXT("Gamertag fetching finished"));
    return Gamertag;
}
#endif

void FInfoManager::UpdateBlackBoxConfiguration(const SDKInformation& SDKInfo_)
{
    blackbox::config_set_api_key(TCHAR_TO_UTF8(*SDKInfo_.APIKey));
    blackbox::config_set_base_url(TCHAR_TO_UTF8(*SDKInfo_.BaseUrl));
    blackbox::config_set_iam_url(TCHAR_TO_UTF8(*SDKInfo_.IamUrl));
    blackbox::config_set_sdk_download_url(TCHAR_TO_UTF8(*SDKInfo_.DownloadURL));
    blackbox::config_set_release_json_url(TCHAR_TO_UTF8(*SDKInfo_.LatestReleaseURL));
    blackbox::config_set_game_version_id(TCHAR_TO_UTF8(*SDKInfo_.GameVersionId));
    blackbox::config_set_project_id(TCHAR_TO_UTF8(*SDKInfo_.ProjectId));
    blackbox::config_set_namespace(TCHAR_TO_UTF8(*SDKInfo_.Namespace));
    blackbox::config_set_build_id(TCHAR_TO_UTF8(*SDKInfo_.BuildId));
    blackbox::config_set_config_path(TCHAR_TO_UTF8(*SDKInfo_.CoreSDKConfigPath));
    blackbox::import_default_config(TCHAR_TO_UTF8(*SDKInfo_.CoreSDKConfigPath));

#    if WITH_EDITOR
    blackbox::config_set_is_using_editor(true);
#    else
    blackbox::config_set_is_using_editor(false);
#    endif

#    if (ENGINE_MAJOR_VERSION == 4)
    blackbox::config_set_engine("UE4");
#    elif (ENGINE_MAJOR_VERSION == 5)
    blackbox::config_set_engine("UE5");
#    endif
    auto const engine_version = FEngineVersion::Current();
    blackbox::config_set_engine_major_version(engine_version.GetMajor());
    blackbox::config_set_engine_minor_version(engine_version.GetMinor());
    blackbox::config_set_engine_patch_version(engine_version.GetPatch());

#if BLACKBOX_UE_SONY
    FString PlatformSDKVersion{};

#    if BLACKBOX_UE_PS4
    PlatformSDKVersion = TEXT(BLACKBOX_STRINGIZE(SCE_ORBIS_SDK_VERSION));
#    elif BLACKBOX_UE_PS5
    PlatformSDKVersion = TEXT(BLACKBOX_STRINGIZE(SCE_PROSPERO_SDK_VERSION));
#    endif

    PlatformSDKVersion.RemoveFromStart(TEXT("("));
    PlatformSDKVersion.RemoveFromEnd(TEXT(")"));
    PlatformSDKVersion.RemoveFromEnd(TEXT("u"));

    blackbox::config_set_platform_sdk_version(std::string(TCHAR_TO_UTF8(*PlatformSDKVersion)).c_str());
#endif
}

void FInfoManager::UpdateBlackBoxClientInformation(const ProcessInformation& ProcInfo_)
{
    blackbox::config_set_pid(ProcInfo_.PID);
    blackbox::config_set_crash_folder(TCHAR_TO_UTF8(*ProcInfo_.CrashFolder));
    blackbox::config_set_crash_guid(TCHAR_TO_UTF8(*ProcInfo_.CrashGUID));
    blackbox::config_set_blackbox_helper_path(TCHAR_TO_UTF8(*ProcInfo_.BlackBoxHelperPath));
    blackbox::config_set_blackbox_issue_reporter_path(TCHAR_TO_UTF8(*ProcInfo_.BlackBoxIssueReporterPath));
    blackbox::config_set_log_source_file_path(TCHAR_TO_UTF8(*ProcInfo_.LogSourceFilePath));
}

bool FInfoManager::IsOSInfoEmpty() const
{
    bool Empty = (OSInfo.Architecture.Len() == 0) && (OSInfo.ComputerName.Len() == 0) && (OSInfo.Country.Len() == 0) &&
                 (OSInfo.Locale.Len() == 0) && (OSInfo.Name.Len() == 0) && (OSInfo.RendererApi.Len() == 0) &&
                 (OSInfo.UserName.Len() == 0) && (OSInfo.Version.Len() == 0);
    return Empty;
}
