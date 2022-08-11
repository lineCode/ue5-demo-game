// Copyright (c) 2019 - 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "BlackBoxSDKModule.h"

#include "BlackBoxLog.h"
#include "BlackBoxSettings.h"
#include <cstddef>
#if WITH_EDITOR
#    include "BlackBoxSettingsCustomization.h"
#endif
#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "EngineGlobals.h"
#include "Engine.h"
#include "Engine/GameEngine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformDriver.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "HAL/PlatformOutputDevices.h"
#include "HAL/PlatformProcess.h"
#include "InformationManager.h"
#include "BlackBoxBackbufferManager.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Paths.h"
#include "Misc/Compression.h"
#include "Misc/EngineVersion.h"
#include "Endpoints.h"
#include "BlackBoxCommon.h"
#include "BlackBoxTraceWriter.h"
#include "CrashHandler/BlackBoxCrashHandler.h"
#if WITH_EDITOR
#    include "PropertyEditorModule.h"
#endif

#if BLACKBOX_UE_WINDOWS
#    include "Windows/WindowsPlatformCrashContext.h"
#    include "Windows/WindowsPlatformProcess.h"
#elif BLACKBOX_UE_XBOXONE
#    if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION < 26)
#        include "XboxOne/XboxOnePlatformCrashContext.h"
#        include "XboxOne/XboxOnePlatformProcess.h"
#    else
#        include "XboxOnePlatformCrashContext.h"
#        include "XboxOnePlatformProcess.h"
#    endif
#elif BLACKBOX_UE_XBOXONEGDK
#    include "XboxCommonPlatformCrashContext.h"
#    if ENGINE_MAJOR_VERSION == 4
#        include "XboxOneGDKPlatformProcess.h"
#    else
#        include "XboxCommonPlatformProcess.h"
#    endif
#elif BLACKBOX_UE_XSX
#    include "XSXPlatformProcess.h"
#    include "XboxCommonPlatformCrashContext.h"
#elif BLACKBOX_UE_PS4
#    include "PS4PlatformProcess.h"
#elif BLACKBOX_UE_PS5
#    include "PS5PlatformProcess.h"
#elif BLACKBOX_UE_LINUX
#    include "Unix/UnixPlatformProcess.h"
#    include "Unix/UnixPlatformCrashContext.h"
#elif BLACKBOX_UE_MAC
#    include "Mac/MacPlatformProcess.h"
#endif

#include "accelbyte/cpp/blackbox.h"
#include "accelbyte/cpp/blackbox_http.h"
#include "BlackBoxUnrealHttp.h"

#include <string>
#include <array>
#include <memory>

#if WITH_EDITOR
#    include "ISettingsModule.h"
#    include "ISettingsSection.h"
#endif

#if BLACKBOX_UE_WINDOWS
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <Windows.h>
#endif

/** How many cycles the renderthread used (excluding idle time). It's set once per frame in FViewport::Draw. */
extern RENDERCORE_API uint32 GRenderThreadTime;
/** How many cycles the gamethread used (excluding idle time). It's set once per frame in FViewport::Draw. */
extern RENDERCORE_API uint32 GGameThreadTime;

namespace BlackboxSDK {
extern bool IsAPIKeyOverriden;
extern bool IsGameVersionIDOverriden;
extern bool IsNamespaceOverriden;

static bool
ZlibCompress(void* CompressedBuffer, int32& CompressedSize, const void* UncompressedBuffer, int32 UncompressedSize)
{
    return FCompression::CompressMemory(
        NAME_Zlib, CompressedBuffer, CompressedSize, UncompressedBuffer, UncompressedSize);
}
static void SessionCreatedCallback(const blackbox::callback_http_response&, const char*);
static void MachineInfoGatherCallback();
static void OnPlaytestIdRetrieved(const char*);
} // namespace BlackboxSDK

class MissionOutputDevice : public FOutputDevice {
public:
    MissionOutputDevice()
    {
        check(GLog);
        GLog->AddOutputDevice(this);
        if (GLog->IsRedirectingTo(this))
            return; // Never gets hit

        return;
    };

    ~MissionOutputDevice()
    {
        if (GLog != nullptr) {
            GLog->RemoveOutputDevice(this);
        }
    };

    virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override
    {
        FString Format = FOutputDeviceHelper::FormatLogLine(Verbosity, Category, V, GPrintLogTimes);
        auto Conv = StringCast<ANSICHAR>(*Format);
        const char* UE_log_char = Conv.Get();
        blackbox::collect_log_streaming_data(UE_log_char);
    }
};

#define LOCTEXT_NAMESPACE "FAccelByteBlackBoxSDK"

class FAccelByteBlackBoxSDKModule : public IAccelByteBlackBoxSDKModuleInterface {
public:
    FAccelByteBlackBoxSDKModule();
    ~FAccelByteBlackBoxSDKModule();
    void StartupModule() override;
    void ShutdownModule() override;
    void Start() override;
    void Stop() override;
    void Tick(float DeltaTime) override;
    void FeedKeyboardInput(APlayerController* PlayerController) override;
    void DeleteAdditionalInfoField(FString FieldName) override;
    void EmptyAdditionalInfo() override;
    bool UpdateAdditionalInfo(FString FieldName, FString Value) override;
    FString GetAdditionalInfoValue(FString FieldName) override;
    void EnableLog(bool Enable) override;
    void SetLogCallbackSeverity(uint8 MaxLogSeverity) override;
    static FAccelByteBlackBoxSDKModule* Instance();

private:
    void StartSDK();
    void StopSDK();
    void RegisterSettings();
    void UnregisterSettings();
    bool LoadSettings();
    void OnPropertyChanged(UObject* ModifiedObject, FPropertyChangedEvent& PropertyChangedEvent);
    void GatherProcessInformation();
    void GatherGPUInformation();
    void SetSDKInfoCrashContextXML();
    void SetOSInfoCrashContextXML();
    void ParseProfilingInformationFromCommandLine();

    void RegisterViewportResizedCallback();
    void UnregisterViewportResizedCallback();
    void ViewportResized(FViewport*, uint32);
    void OnGameStart(const UWorld::FActorsInitializedParams& InitializationValues);
    void OnGameStop(UWorld* World, bool BoolArg1, bool BoolArg2);
    bool OnEngineTick(float TickTime);

#if BLACKBOX_UE_WINDOWS
private:
    void OnGameCrash();

private:
    FDelegateHandle OnCrashHandle;
#endif

private:
    static FAccelByteBlackBoxSDKModule* Self;
    void* BlackBoxDllHandle = nullptr;
    FString SessionId;
    FString PlaytestId;
    FString SDKVersion;
    TUniquePtr<FInfoManager> InfoManager;
    TUniquePtr<FBackbufferManager> BackbufferManager;
    TUniquePtr<FBlackBoxCrashHandler> CrashHandler;
    FDelegateHandle OnPropertyChangedDelegateHandle;
    FDelegateHandle OnBackBufferReadyDelegate;
    FDelegateHandle OnViewportResizedHandle;
    FString IssueFolder;
    FString IssueReporterPath;
    bool ConfigValidated = false;
    bool InGameSession = false;
    TUniquePtr<MissionOutputDevice> BlackBoxLogger;
    friend void BlackboxSDK::SessionCreatedCallback(const blackbox::callback_http_response&, const char*);
    friend void BlackboxSDK::MachineInfoGatherCallback();
    friend void BlackboxSDK::OnPlaytestIdRetrieved(const char*);
};
FAccelByteBlackBoxSDKModule* FAccelByteBlackBoxSDKModule::Self = nullptr;

void BlackboxSDK::SessionCreatedCallback(const blackbox::callback_http_response& resp, const char* session_id)
{
    auto ModuleInstance = FAccelByteBlackBoxSDKModule::Instance();
    if (ModuleInstance) {
        ModuleInstance->SessionId = FString::Printf(TEXT("%s"), UTF8_TO_TCHAR(session_id));
        if (!resp.is_success) {
            UE_LOG(
                LogBlackBox,
                Error,
                TEXT("Cannot create a new session [%d], Trace ID: %s"),
                resp.http_status,
                UTF8_TO_TCHAR(resp.trace_id));
            return;
        }
        blackbox::unreal::WriteSessionID(UTF8_TO_TCHAR(session_id));
        UE_LOG(LogBlackBox, Log, TEXT("Got Session ID: %s"), *ModuleInstance->SessionId);
#if !BLACKBOX_UE_SONY
        FPlatformCrashContext::SetGameData(TEXT("BlackBox.SessionID"), ModuleInstance->SessionId);
#endif
    }
}

void BlackboxSDK::OnPlaytestIdRetrieved(const char* playtest_id)
{
    auto ModuleInstance = FAccelByteBlackBoxSDKModule::Instance();
    if (ModuleInstance) {
        ModuleInstance->PlaytestId = FString::Printf(TEXT("%s"), UTF8_TO_TCHAR(playtest_id));
#if !BLACKBOX_UE_SONY
        FPlatformCrashContext::SetGameData(TEXT("BlackBox.PlayTestID"), ModuleInstance->PlaytestId);
#endif
    }
}

void BlackboxSDK::MachineInfoGatherCallback()
{
    auto ModuleInstance = FAccelByteBlackBoxSDKModule::Instance();
    if (ModuleInstance) {
        ModuleInstance->SetOSInfoCrashContextXML();
    }
}

#if BLACKBOX_UE_WINDOWS
void FAccelByteBlackBoxSDKModule::OnGameCrash()
{
    blackbox::handle_crash();
}
#endif

void FAccelByteBlackBoxSDKModule::OnGameStart(const UWorld::FActorsInitializedParams& InitializationValues)
{
    if (IsValid(InitializationValues.World) && InitializationValues.World->GetGameInstance() != nullptr) {
        StartSDK();
        InGameSession = true;
    }
}

void FAccelByteBlackBoxSDKModule::OnGameStop(UWorld* World, bool BoolArg1, bool BoolArg2)
{
    if (IsValid(World) && World->GetGameInstance() != nullptr) {
        StopSDK();
        InGameSession = false;
    }
}

FAccelByteBlackBoxSDKModule::FAccelByteBlackBoxSDKModule()
{
    Self = this;
#if BLACKBOX_UE_WINDOWS
#    if UE_BUILD_SHIPPING || UE_BUILD_TEST
    FString DllPath = FPaths::ProjectPluginsDir() / TEXT("BlackBoxSDK/DLLs/x64/Win") / TEXT("blackbox-core.dll");
#    else
    FString DllPath =
        FPaths::ProjectPluginsDir() / TEXT("BlackBoxSDK/DLLs/x64/Win/relwithdebinfo") / TEXT("blackbox-core.dll");
#    endif
#elif BLACKBOX_UE_LINUX
#    if UE_BUILD_SHIPPING || UE_BUILD_TEST
    FString DllPath = FPaths::ProjectPluginsDir() / TEXT("BlackBoxSDK/Libs/x64/Linux") / TEXT("libblackbox-core.so");
#    else
    FString DllPath =
        FPaths::ProjectPluginsDir() / TEXT("BlackBoxSDK/Libs/x64/Linux/relwithdebinfo") / TEXT("libblackbox-core.so");
#    endif
#elif BLACKBOX_UE_MAC
#    if UE_BUILD_SHIPPING || UE_BUILD_TEST
    FString DllPath = FPaths::ProjectPluginsDir() / TEXT("BlackBoxSDK/Libs/x64/Mac") / TEXT("libblackbox-core.dylib");
#    else
    FString DllPath =
        FPaths::ProjectPluginsDir() / TEXT("BlackBoxSDK/Libs/x64/Mac/relwithdebinfo") / TEXT("libblackbox-core.dylib");
#    endif
#elif BLACKBOX_UE_XBOXONEGDK
    FString DllPath =
        FPaths::ProjectPluginsDir() / TEXT("BlackBoxSDK/DLLs/x64/XBCommon") / TEXT("BlackBoxSDK-XboxSeriesX.dll");
#elif BLACKBOX_UE_XSX
    FString DllPath =
        FPaths::ProjectPluginsDir() / TEXT("BlackBoxSDK/DLLs/x64/XSX") / TEXT("BlackBoxSDK-XboxSeriesX.dll");
#endif
#if BLACKBOX_UE_WINDOWS || BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX || BLACKBOX_UE_LINUX || BLACKBOX_UE_MAC
    BlackBoxDllHandle = FPlatformProcess::GetDllHandle(*DllPath);
    if (!BlackBoxDllHandle) {
        UE_LOG(LogBlackBox, Fatal, TEXT("Can't find BlackBox DLL, please make sure that the file exist"));
        return;
    }
#endif

#if BLACKBOX_UE_WINDOWS
    OnCrashHandle = FCoreDelegates::OnHandleSystemError.AddRaw(this, &FAccelByteBlackBoxSDKModule::OnGameCrash);
#endif
}

FAccelByteBlackBoxSDKModule::~FAccelByteBlackBoxSDKModule()
{
    Self = nullptr;
#if BLACKBOX_UE_WINDOWS
    FCoreDelegates::OnHandleSystemError.Remove(OnCrashHandle);
#endif
#if BLACKBOX_UE_WINDOWS || BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX || BLACKBOX_UE_LINUX || BLACKBOX_UE_MAC
    if (BlackBoxDllHandle) {
        FPlatformProcess::FreeDllHandle(BlackBoxDllHandle);
    }
#endif
}

FAccelByteBlackBoxSDKModule* FAccelByteBlackBoxSDKModule::Instance()
{
    return Self;
}

void FAccelByteBlackBoxSDKModule::StartupModule()
{
    UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Startup Module"));
    BlackBoxLogger = MakeUnique<MissionOutputDevice>();
    std::string sdk_ver = blackbox::info_get_version();
    SDKVersion = FString(UTF8_TO_TCHAR(sdk_ver.c_str()));
    UE_LOG(LogBlackBox, Log, TEXT("SDK Version: %s"), *SDKVersion);

    RegisterSettings();
    if (!LoadSettings()) {
        ShutdownModule();
        return;
    }
    // Instantiate Setting Storage Class
    InfoManager = MakeUnique<FInfoManager>();
#if BLACKBOX_UE_PS5 || BLACKBOX_UE_PS4
    // set up the http caller before initializing the module (module is not needed to set this instance)
    blackbox::unreal::set_unreal_singleton(new blackbox::unreal::sdk_http_impl());
#endif
    // Gather Settings Value
    blackbox::config_set_gpu_device_id(GRHIDeviceId);
    BackbufferManager = MakeUnique<FBackbufferManager>();
    blackbox::init_module(BackbufferManager->GetActiveRenderingAPI());
    blackbox::start_gather_device_info(&BlackboxSDK::MachineInfoGatherCallback);

#if BLACKBOX_UE_PS5 || BLACKBOX_UE_PS4
    blackbox::config_set_engine_version(TCHAR_TO_UTF8(*FEngineVersion::Current().ToString()));
#endif

    GatherProcessInformation();
    SetSDKInfoCrashContextXML();
    blackbox::set_file_compression_function(&BlackboxSDK::ZlibCompress);
    // Start up Crash Handler
    CrashHandler = MakeUnique<FBlackBoxCrashHandler>();

    RegisterViewportResizedCallback();
    FWorldDelegates::OnWorldInitializedActors.AddRaw(this, &FAccelByteBlackBoxSDKModule::OnGameStart);
    FWorldDelegates::OnWorldCleanup.AddRaw(this, &FAccelByteBlackBoxSDKModule::OnGameStop);
#if ENGINE_MAJOR_VERSION == 4
    using Ticker = FTicker;
#else
    using Ticker = FTSTicker;
#endif
    Ticker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FAccelByteBlackBoxSDKModule::OnEngineTick));

    IssueFolder = FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("Issues"));
    IssueReporterPath =
        FPaths::ProjectPluginsDir() / +"BlackBoxSDK/issue_reporter/" + "x64" + "/blackbox_issue_reporter.exe";
    UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Startup Module - END"));
}

void FAccelByteBlackBoxSDKModule::ShutdownModule()
{
    UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Shutdown Module"));
    blackbox::stop_recording();
    blackbox::suspend_shared_mem();
    BackbufferManager->UnregisterBackbufferCallback();
    UnregisterViewportResizedCallback();
    UnregisterSettings();
    blackbox::shutdown_module();
#if BLACKBOX_UE_PS5 || BLACKBOX_UE_PS4
    blackbox::unreal::destroy_unreal_singleton();
#endif
    UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Shutdown Module - END"));
}

void FAccelByteBlackBoxSDKModule::StartSDK()
{
    UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Start SDK"));
#if WITH_EDITOR
    blackbox::clear_pending_tasks();
#endif
    ConfigValidated = blackbox::validate_config();
    if (ConfigValidated) {
#if BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX
        UE_LOG(LogBlackBox, Log, TEXT("Sending previous crashes..."));
        blackbox::send_crash(TCHAR_TO_ANSI(TEXT("D:\\")), nullptr);
        UE_LOG(LogBlackBox, Log, TEXT("Fetching default user gamertag"));
        blackbox::set_gamertag(FInfoManager::GetGamertag().c_str());
#endif
        ParseProfilingInformationFromCommandLine();
        GatherGPUInformation();
        UE_LOG(LogBlackBox, Log, TEXT("Creating Session"));
        SDKInformation Info = InfoManager->GetSDKInformation();
        if (Info.BuildId.IsEmpty()) {
            blackbox::start_new_session_on_editor(
                &BlackboxSDK::SessionCreatedCallback, &BlackboxSDK::OnPlaytestIdRetrieved);
        }
        else {
            blackbox::start_new_session(&BlackboxSDK::SessionCreatedCallback, &BlackboxSDK::OnPlaytestIdRetrieved);
        }
        UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Start SDK - END"));
    }
    else {
        UE_LOG(LogBlackBox, Error, TEXT("BlackBox SDK configuration is invalid"));
    }
}

void FAccelByteBlackBoxSDKModule::StopSDK()
{
    UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Stop SDK"));
    if (ConfigValidated) {
        blackbox::stop_profiling();
        InfoManager->ResetKeyInformation();
        blackbox::clear_pending_tasks();
        blackbox::stop_log_streaming();
        UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Stop SDK - END"));
    }
}

[[deprecated]] void FAccelByteBlackBoxSDKModule::Start()
{
}
[[deprecated]] void FAccelByteBlackBoxSDKModule::Stop()
{
}

void FAccelByteBlackBoxSDKModule::RegisterSettings()
{
    UBlackBoxSettings* BlackBoxSettings = GetMutableDefault<UBlackBoxSettings>();
    BlackBoxSettings->InitializeLocalConfigProperties();
#if WITH_EDITOR
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
        SettingsModule->RegisterSettings(
            TEXT("Project"),
            TEXT("Plugins"),
            TEXT("AccelByte BlackBox SDK"),
            FText::FromName(TEXT("AccelByte BlackBox SDK")),
            FText::FromName(TEXT("Setup your plugin.")),
            BlackBoxSettings);
    }
    OnPropertyChangedDelegateHandle =
        FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &FAccelByteBlackBoxSDKModule::OnPropertyChanged);
#endif
}

void FAccelByteBlackBoxSDKModule::UnregisterSettings()
{
#if WITH_EDITOR
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
        SettingsModule->UnregisterSettings(TEXT("Project"), TEXT("Plugins"), TEXT("AccelByte BlackBox SDK"));
    }
    FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(OnPropertyChangedDelegateHandle);
#endif
}

bool FAccelByteBlackBoxSDKModule::LoadSettings()
{
    SDKInformation SDKInfo;
    SDKInfo.BaseUrl = TEXT(__BLACKBOX_BASE_URL__);
    SDKInfo.IamUrl = FString(TEXT(__BLACKBOX_BASE_URL__)) + TEXT("/iam");
    SDKInfo.DownloadURL = TEXT(__BLACKBOX_DOWNLOAD_URL__);
    SDKInfo.LatestReleaseURL = TEXT(__BLACKBOX_RELEASE_INFO_URL__);
    bool ExperimentalServerBuildIdFeature = GetDefault<UBlackBoxSettings>()->ExperimentalServerBuildIdFeature;
    bool IsServer = IsRunningDedicatedServer() && ExperimentalServerBuildIdFeature;

#if WITH_EDITOR
    SDKInfo.CoreSDKConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultBlackBox.ini");
#else
    SDKInfo.CoreSDKConfigPath = TEXT("");
#endif

    FString BlackBoxConfig = FPaths::ProjectConfigDir() / TEXT("BlackBox.ini");
    if (FPaths::FileExists(BlackBoxConfig)) {
        if (!GConfig->GetString(TEXT("BlackBoxSettings"), TEXT("APIKey"), SDKInfo.APIKey, BlackBoxConfig)) {
            UE_LOG(
                LogBlackBox,
                Log,
                TEXT("Missing APIKey value from BlackBox.ini, fallback to the entry in DefaultEngine.ini"));
            SDKInfo.APIKey = TEXT("");
            BlackboxSDK::IsAPIKeyOverriden = false;
        }
        if (!GConfig->GetString(
                TEXT("BlackBoxSettings"), TEXT("GameVersionID"), SDKInfo.GameVersionId, BlackBoxConfig)) {
            UE_LOG(
                LogBlackBox,
                Log,
                TEXT("Missing GameVersionId value from BlackBox.ini, fallback to the entry in DefaultEngine.ini"));
            SDKInfo.GameVersionId = TEXT("");
            BlackboxSDK::IsGameVersionIDOverriden = false;
        }
        if (!GConfig->GetString(TEXT("BlackBoxSettings"), TEXT("Namespace"), SDKInfo.Namespace, BlackBoxConfig)) {
            UE_LOG(
                LogBlackBox,
                Log,
                TEXT("Missing Namespace value from BlackBox.ini, fallback to the entry in DefaultEngine.ini"));
            SDKInfo.Namespace = TEXT("");
            BlackboxSDK::IsNamespaceOverriden = false;
        }
        if (SDKInfo.BuildId.IsEmpty()) {
            FString BuildId{};

            if (!IsServer) {
                if (!GConfig->GetString(TEXT("BlackBoxSettings"), TEXT("BuildID"), BuildId, BlackBoxConfig)) {
                    BuildId = TEXT("");
                }
            }
            else {
                if (!GConfig->GetString(TEXT("BlackBoxSettings"), TEXT("ServerBuildID"), BuildId, BlackBoxConfig)) {
                    BuildId = TEXT("");
                }
            }

            SDKInfo.BuildId = BuildId;
        }

#if WITH_EDITOR
        // register settings detail panel customization
        FPropertyEditorModule& PropertyModule =
            FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.RegisterCustomClassLayout(
            UBlackBoxSettings::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FBlackBoxSettingsCustomization::MakeInstance));
#endif
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("BlackBox.ini file isn't present, fallback to the entries in DefaultEngine.ini"));
        BlackboxSDK::IsNamespaceOverriden = false;
        BlackboxSDK::IsGameVersionIDOverriden = false;
        BlackboxSDK::IsAPIKeyOverriden = false;
    }
    if (SDKInfo.APIKey.IsEmpty()) {
        UE_LOG(LogBlackBox, Log, TEXT("Get APIKey entry from DefaultEngine.ini"));
        SDKInfo.APIKey = GetDefault<UBlackBoxSettings>()->APIKey;
    }
    if (SDKInfo.Namespace.IsEmpty()) {
        UE_LOG(LogBlackBox, Log, TEXT("Get Namespace entry from DefaultEngine.ini"));
        SDKInfo.Namespace = GetDefault<UBlackBoxSettings>()->Namespace;
    }
    if (SDKInfo.GameVersionId.IsEmpty()) {
        UE_LOG(LogBlackBox, Log, TEXT("Get GameVersionId entry from DefaultEngine.ini"));
        SDKInfo.GameVersionId = GetDefault<UBlackBoxSettings>()->GameVersionID;
    }

    if (SDKInfo.Namespace.IsEmpty()) {
        UE_LOG(LogBlackBox, Log, TEXT("Namespace not set"));
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("Namespace    : %s"), *SDKInfo.Namespace);
    }

    if (SDKInfo.GameVersionId.IsEmpty()) {
        UE_LOG(LogBlackBox, Log, TEXT("Version ID not set"));
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("Version ID   : %s"), *SDKInfo.GameVersionId);
    }

    GetMutableDefault<UBlackBoxSettings>()->InitializeNeedToRestartOnChangeProperties();

    FString DetectedBuildIdText = IsServer ? "Build ID (Server)" : "Build ID";
    if (SDKInfo.BuildId.IsEmpty()) {
        UE_LOG(LogBlackBox, Log, TEXT("%s not set"), *DetectedBuildIdText);
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("%s     : %s"), *DetectedBuildIdText, *SDKInfo.BuildId);
    }

    if (!GConfig->GetString(
            TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectID"), SDKInfo.ProjectId, GGameIni)) {
        UE_LOG(
            LogBlackBox,
            Fatal,
            TEXT("Cannot run SDK, Missing ProjectID in [/Script/EngineSettings.GeneralProjectSettings] of "
                 "DefaultGame.ini"));
        return false;
    }

    if (GetDefault<UBlackBoxSettings>()->EnableLog &&
        static_cast<uint8>(GetDefault<UBlackBoxSettings>()->LogSeverity) > 0) {
        EnableLog(true);
        SetLogCallbackSeverity(static_cast<uint8>(GetDefault<UBlackBoxSettings>()->LogSeverity));
    }
    else if (static_cast<uint8>(GetDefault<UBlackBoxSettings>()->LogSeverity) == 0) {
        GetMutableDefault<UBlackBoxSettings>()->EnableLog = true;
        EnableLog(true);
        GetMutableDefault<UBlackBoxSettings>()->LogSeverity = BlackBoxLogSeverity::WARNING;
        SetLogCallbackSeverity(static_cast<uint8>(BlackBoxLogSeverity::WARNING));
    }

    InfoManager->SetSDKInformation(SDKInfo);
    return true;
}

void FAccelByteBlackBoxSDKModule::OnPropertyChanged(
    UObject* ModifiedObject, FPropertyChangedEvent& PropertyChangedEvent)
{
    if (UBlackBoxSettings* settings = Cast<UBlackBoxSettings>(ModifiedObject)) {
        SDKInformation Info = InfoManager->GetSDKInformation();
        if (!BlackboxSDK::IsAPIKeyOverriden) {
            Info.APIKey = settings->APIKey;
        }
        if (!BlackboxSDK::IsNamespaceOverriden) {
            Info.Namespace = settings->Namespace;
        }
        if (!BlackboxSDK::IsGameVersionIDOverriden) {
            Info.GameVersionId = settings->GameVersionID;
        }
        if (settings->EnableLog && static_cast<uint8>(settings->LogSeverity) > 0) {
            EnableLog(true);
            SetLogCallbackSeverity(static_cast<uint8>(settings->LogSeverity));
        }
        else {
            EnableLog(false);
        }

        if (settings->CheckNeedToRestart(PropertyChangedEvent.GetPropertyName().ToString())) {
            settings->ShowMustRestartDialog();
        }

        settings->ApplyLocalConfigProperties();
        blackbox::save_local_config(TCHAR_TO_UTF8(FApp::GetProjectName()));

        InfoManager->SetSDKInformation(Info);
    }
}

static short previous_trigger_state = 0;
bool FAccelByteBlackBoxSDKModule::OnEngineTick(float TickTime)
{
    if (blackbox::config_get_store_crash_video() && BackbufferManager != nullptr && !BackbufferManager->GetIsActive()) {
        BackbufferManager->RegisterBackbufferCallback();
    }

    blackbox::engine_tick(TickTime);

    if (InGameSession) {
        blackbox::tick(TickTime);
        float game_thread_time_ms = FPlatformTime::ToMilliseconds(GGameThreadTime);
        float render_thread_time_ms = FPlatformTime::ToMilliseconds(GRenderThreadTime);
        blackbox::update_profiling_basic_data(TickTime, render_thread_time_ms, game_thread_time_ms);
    }
#if BLACKBOX_UE_WINDOWS
    short trigger_state_now = GetKeyState(VK_F8) & 0x8000;
    if ((trigger_state_now & 0x8000) == 0x8000 && (previous_trigger_state & 0x8000) == 0) {
        UE_LOG(LogBlackBox, Warning, TEXT("CAPTURING SCREENSHOT"));
        if (IssueFolder == "" || IssueFolder.IsEmpty()) {
            UE_LOG(LogBlackBox, Error, TEXT("Issue Folder does not exist"));
        }
        blackbox::capture_screenshot(TCHAR_TO_UTF8(*IssueFolder));
    }
    previous_trigger_state = trigger_state_now;
#endif
    return true;
}

void FAccelByteBlackBoxSDKModule::GatherProcessInformation()
{
    ProcessInformation ProcInfo;
    ProcInfo.PID = FPlatformProcess::GetCurrentProcessId();

    // Windows Specific helper and issue reporter applications
#if BLACKBOX_UE_WINDOWS
    FString arch{};
#    if defined _M_X64
    arch = "x64";
#    else
    arch = "x86";
#    endif

    // Helper
    FString HelperExePath{};
    HelperExePath = FPaths::ProjectPluginsDir() / +"BlackBoxSDK/helper/" + arch + "/blackbox_helper.exe";
    ProcInfo.BlackBoxHelperPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*HelperExePath);

    // Issue Reporter
    FString IssueReporterExePath{};
    IssueReporterExePath =
        FPaths::ProjectPluginsDir() / +"BlackBoxSDK/issue_reporter/" + arch + "/blackbox_issue_reporter.exe";
    ProcInfo.BlackBoxIssueReporterPath =
        IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*IssueReporterExePath);
#elif BLACKBOX_UE_LINUX
    // Helper
    FString HelperExePath{};
    HelperExePath = FPaths::ProjectPluginsDir() / +"BlackBoxSDK/helper/linux/blackbox_helper";
    ProcInfo.BlackBoxHelperPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*HelperExePath);
#endif

    // Gather Crash directory information
#if BLACKBOX_UE_WINDOWS
    FPlatformCrashContext CrashContext(ECrashContextType::Crash, TEXT("Crash"));
    TCHAR CrashGUID[FGenericCrashContext::CrashGUIDLength];
    CrashContext.GetUniqueCrashName(CrashGUID, FGenericCrashContext::CrashGUIDLength);
    const FString AppName = CrashContext.GetCrashGameName();
    FString CrashFolder = FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("Crashes"));
    FString CrashGUIDString = FString(CrashGUID);

#    if ((ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION < 26)) || !WITH_EDITOR
    FString BaseCrashGUID = CrashGUIDString.Left(CrashGUIDString.Len() - 5);
    FString Number = CrashGUIDString.Right(4);
    int NextNumber = FCString::Atoi(*Number) + 1;
    CrashGUIDString = FString::Printf(TEXT("%s_%04i"), *BaseCrashGUID, NextNumber);
#    endif

#    if !BLACKBOX_UE_WINDOWS
    CrashFolder = FPaths::Combine(*CrashFolder, *CrashGUIDString);
#    else
    CrashGUIDString = CrashGUIDString.Left(CrashGUIDString.Len() - 4);
#    endif

#    if UE_BUILD_DEVELOPMENT
    FString CrashFolderAbsolute = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*CrashFolder);
#    else
    FString CrashFolderAbsolute = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*CrashFolder);
#    endif

    ProcInfo.CrashFolder = CrashFolderAbsolute;
    ProcInfo.CrashGUID = CrashGUIDString;
#elif BLACKBOX_UE_LINUX || BLACKBOX_UE_MAC
    FString CrashGUIDString;
    if (!FParse::Value(FCommandLine::Get(), TEXT("CrashGUID="), CrashGUIDString) || CrashGUIDString.Len() <= 0) {
        CrashGUIDString = FGuid::NewGuid().ToString();
        FCommandLine::Append(*FString::Printf(TEXT(" -CrashGUID=%s"), *CrashGUIDString));
    }
    FString CrashFolder = FPaths::Combine(
        *FPaths::ProjectSavedDir(),
        TEXT("Crashes"),
        *FString::Printf(
            TEXT("%sinfo-%s-pid-%d-%s"), TEXT("crash"), FApp::GetProjectName(), getpid(), *CrashGUIDString));
    FString CrashFolderAbsolute = FPaths::ConvertRelativePathToFull(CrashFolder);
    UE_LOG(LogBlackBox, Log, TEXT("Crashfolder: %s"), *CrashFolderAbsolute);
    ProcInfo.CrashFolder = CrashFolderAbsolute;
    ProcInfo.CrashGUID = CrashGUIDString;
#endif

    ProcInfo.LogSourceFilePath = FPlatformOutputDevices::GetAbsoluteLogFilename();
    InfoManager->SetProcessInformation(ProcInfo);
}

void FAccelByteBlackBoxSDKModule::SetSDKInfoCrashContextXML()
{
#if BLACKBOX_UE_WINDOWS || BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX || BLACKBOX_UE_LINUX || BLACKBOX_UE_MAC
    SDKInformation SDKInfo = InfoManager->GetSDKInformation();
    FPlatformCrashContext::SetGameData(TEXT("BlackBox.GameVersionID"), SDKInfo.GameVersionId);
    FPlatformCrashContext::SetGameData(TEXT("BlackBox.BuildId"), SDKInfo.BuildId);
    FPlatformCrashContext::SetGameData(TEXT("BlackBox.Namespace"), SDKInfo.Namespace);
    FPlatformCrashContext::SetGameData(TEXT("BlackBox.SDKVersion"), SDKVersion);
#endif
}

void FAccelByteBlackBoxSDKModule::SetOSInfoCrashContextXML()
{
#if BLACKBOX_UE_WINDOWS || BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX || BLACKBOX_UE_LINUX || BLACKBOX_UE_MAC
    OSInformation& OSInfo = InfoManager->GetOSInformation();
    FPlatformCrashContext::SetGameData(TEXT("BlackBox.ComputerName"), OSInfo.ComputerName);
    FPlatformCrashContext::SetGameData(TEXT("BlackBox.UserName"), OSInfo.UserName);
#endif
}

void FAccelByteBlackBoxSDKModule::GatherGPUInformation()
{
    GPUInformation GpuInfo;
    GpuInfo.Model = GRHIAdapterName;
    GpuInfo.DriverVer = FPlatformMisc::GetGPUDriverInfo(GRHIAdapterName).UserDriverVersion;
    InfoManager->SetGPUInformation(GpuInfo);
}

void FAccelByteBlackBoxSDKModule::ParseProfilingInformationFromCommandLine()
{
    UE_LOG(LogBlackBox, Log, TEXT("Starting reading from command line"));
    std::vector<blackbox::types::game_symbol> Symbols;
    std::vector<blackbox::types::game_variable> Vars;
    std::string ModuleName;
    const TCHAR* CommandLine = FCommandLine::Get();
    FJsonSerializableArray Tokens;
    FJsonSerializableArray Switches;
    FCommandLine::Parse(CommandLine, Tokens, Switches);
    int Idx = 0;
    if (Tokens.Find("modname", Idx)) {
        FString TokenAt = Tokens[Idx + 1];
        if (Idx + 1 < Tokens.Num() && !Switches.Contains(TokenAt)) {
            ModuleName = TCHAR_TO_UTF8(*TokenAt);
        }
        else {
            UE_LOG(LogBlackBox, Log, TEXT("Could not parse Module name, command syntax error"));
            return;
        }
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("Failed to parse modname"));
    }
    if (!ModuleName.empty() && Tokens.Find("symbols", Idx)) {
        FString TokenAt;
        while (Idx < Tokens.Num()) {
            TokenAt = Tokens[Idx + 1];
            if (Switches.Contains(TokenAt)) {
                break;
            }
            FString SymAndRva = TokenAt;
            FString SymName;
            FString Rva;
            if (SymAndRva.Split("?", &SymName, &Rva)) {
                blackbox::types::game_symbol Sym;
                Sym.symbol_name = TCHAR_TO_UTF8(*SymName);
                Sym.relative_address = FCString::Atoi(*Rva);
                Sym.module_id = ModuleName;
                Symbols.push_back(Sym);
            }
            else {
                UE_LOG(LogBlackBox, Log, TEXT("Could not parse symbol and RVA, command syntax error"));
            }
            Idx++;
        }
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("Failed to parse symbols"));
    }
    if (!ModuleName.empty() && Tokens.Find("vars", Idx)) {
        FString TokenAt;
        while (Idx < Tokens.Num()) {
            TokenAt = Tokens[Idx + 1];
            if (Switches.Contains(TokenAt)) {
                break;
            }
            FString SymAndRvaAndSize = TokenAt;
            FString RvaAndSize;
            FString SymName;
            FString Rva;
            FString Size;
            if (SymAndRvaAndSize.Split("?", &SymName, &RvaAndSize)) {
                if (RvaAndSize.Split(":", &Rva, &Size)) {
                    blackbox::types::game_variable Var;
                    Var.variable_name = TCHAR_TO_UTF8(*SymName);
                    Var.relative_address = FCString::Atoi(*Rva);
                    Var.size = FCString::Atoi(*Size);
                    Vars.push_back(Var);
                }
            }
            else {
                UE_LOG(LogBlackBox, Log, TEXT("Could not parse instrumented variables, command syntax error"));
            }
            Idx++;
        }
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("Failed to parse variables"));
    }
    UE_LOG(LogBlackBox, Log, TEXT("Module Name: [%s]"), UTF8_TO_TCHAR(ModuleName.c_str()));
    for (int i = 0; i < Symbols.size(); i++) {
        UE_LOG(
            LogBlackBox,
            Log,
            TEXT("Sym: [%s:%d]"),
            UTF8_TO_TCHAR(Symbols[i].symbol_name.c_str()),
            Symbols[i].relative_address);
    }
    for (int i = 0; i < Vars.size(); i++) {
        UE_LOG(
            LogBlackBox,
            Log,
            TEXT("Var: [%s:%d:%d]"),
            UTF8_TO_TCHAR(Vars[i].variable_name.c_str()),
            Vars[i].relative_address,
            Vars[i].size);
    }
    if (!ModuleName.empty()) {
        blackbox::store_profiling_data(ModuleName.c_str(), Symbols.data(), Symbols.size(), Vars.data(), Vars.size());
    }
}

void FAccelByteBlackBoxSDKModule::EnableLog(bool Enable)
{
    if (Enable) {
        auto log_cb = [](blackbox::log::severity sev, const char* msg) {
            switch (sev) {
            case blackbox::log::VERBOSE:
                UE_LOG(LogBlackBox, Log, TEXT("%s"), UTF8_TO_TCHAR(msg));
                break;
            case blackbox::log::INFO:
                UE_LOG(LogBlackBox, Log, TEXT("%s"), UTF8_TO_TCHAR(msg));
                break;
            case blackbox::log::WARNING:
                UE_LOG(LogBlackBox, Warning, TEXT("%s"), UTF8_TO_TCHAR(msg));
                break;
            case blackbox::log::ERROR_:
                UE_LOG(LogBlackBox, Error, TEXT("%s"), UTF8_TO_TCHAR(msg));
                break;
            default:
                break;
            }
        };
        blackbox::set_log_callback(log_cb);
    }
    else {
        blackbox::set_log_callback(nullptr);
    }
}

void FAccelByteBlackBoxSDKModule::SetLogCallbackSeverity(uint8 MaxLogSeverity)
{
    uint8 LogSeverity = 0;
    switch (static_cast<BlackBoxLogSeverity>(MaxLogSeverity)) {
    case BlackBoxLogSeverity::ERROR_:
        LogSeverity = static_cast<uint8>(BlackBoxLogSeverity::ERROR_);
        break;
    case BlackBoxLogSeverity::WARNING:
        LogSeverity =
            static_cast<uint8>(BlackBoxLogSeverity::ERROR_) | static_cast<uint8>(BlackBoxLogSeverity::WARNING);
        break;
    case BlackBoxLogSeverity::INFO:
        LogSeverity = static_cast<uint8>(BlackBoxLogSeverity::ERROR_) |
                      static_cast<uint8>(BlackBoxLogSeverity::WARNING) | static_cast<uint8>(BlackBoxLogSeverity::INFO);
        break;
    case BlackBoxLogSeverity::VERBOSE:
        LogSeverity = static_cast<uint8>(BlackBoxLogSeverity::ERROR_) |
                      static_cast<uint8>(BlackBoxLogSeverity::WARNING) | static_cast<uint8>(BlackBoxLogSeverity::INFO) |
                      static_cast<uint8>(BlackBoxLogSeverity::VERBOSE);
        break;
    default:
        break;
    }
    blackbox::set_log_severity(LogSeverity);
}

[[deprecated]] void FAccelByteBlackBoxSDKModule::Tick(float DeltaTime)
{
}

void FAccelByteBlackBoxSDKModule::FeedKeyboardInput(APlayerController* PlayerController)
{
    if (!IsValid(PlayerController)) {
        return;
    }
    if (!InfoManager->IsKeyInformationPresent()) {
        InfoManager->SetupKeyInformation(PlayerController);
    }
    InputInformation& InputInfo = InfoManager->GetKeyInformation();
    for (uint64 i = 0; i < InputInfo.ActionKeyPair.Num(); i++) {
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 25)
        if (InputInfo.ActionKeyPair[i].Value.IsFloatAxis()) {
#else
        if (InputInfo.ActionKeyPair[i].Value.IsAxis1D()) {
#endif
            continue;
        }
        blackbox::update_key_input(i, PlayerController->IsInputKeyDown(InputInfo.ActionKeyPair[i].Value));
    }
}

void FAccelByteBlackBoxSDKModule::DeleteAdditionalInfoField(FString FieldName)
{
    blackbox::delete_additional_info_field(TCHAR_TO_UTF8(*FieldName));
}

void FAccelByteBlackBoxSDKModule::EmptyAdditionalInfo()
{
    blackbox::empty_additional_info();
}

bool FAccelByteBlackBoxSDKModule::UpdateAdditionalInfo(FString FieldName, FString Value)
{
    blackbox::update_additional_info(TCHAR_TO_UTF8(*FieldName), TCHAR_TO_UTF8(*Value));
    return true;
}

FString FAccelByteBlackBoxSDKModule::GetAdditionalInfoValue(FString FieldName)
{
    std::string Value = blackbox::get_additional_info_value(TCHAR_TO_UTF8(*FieldName));
    FString Out = UTF8_TO_TCHAR(Value.c_str());
    return Out;
}

void FAccelByteBlackBoxSDKModule::RegisterViewportResizedCallback()
{
#if BLACKBOX_UE_WINDOWS
    if (GEngine) {
        OnViewportResizedHandle =
            FViewport::ViewportResizedEvent.AddRaw(this, &FAccelByteBlackBoxSDKModule::ViewportResized);
    }
#endif
}

void FAccelByteBlackBoxSDKModule::UnregisterViewportResizedCallback()
{
#if BLACKBOX_UE_WINDOWS
    if (OnViewportResizedHandle.IsValid()) {
        FViewport::ViewportResizedEvent.Remove(OnViewportResizedHandle);
    }
#endif
}

void FAccelByteBlackBoxSDKModule::ViewportResized(FViewport*, uint32)
{
#if BLACKBOX_UE_WINDOWS
    blackbox::notify_change_game_resolution();
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAccelByteBlackBoxSDKModule, BlackBoxSDK)
