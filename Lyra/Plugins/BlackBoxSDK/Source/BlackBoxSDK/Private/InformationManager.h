// Copyright (c) 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "BlackBoxCommon.h"
#include <string>

class APlayerController;

struct CPUInformation {
    FString Model;
    uint64 Frequency;
};

struct GPUInformation {
    FString Model;
    uint64 Frequency;
    uint64 MemoryAmount;
    uint64 MemoryFrequency;
    FString DriverVer;
};

struct OSInformation {
    FString Name;
    FString Version;
    FString Architecture;
    FString RendererApi;
    FString Locale;
    FString Country;
    FString UserName;
    FString ComputerName;
};

struct SDKInformation {
    FString BaseUrl;
    FString IamUrl;
    FString DownloadURL;
    FString LatestReleaseURL;
    FString BuildId;
    FString GameVersionId;
    FString ProjectId;
    FString Namespace;
    FString APIKey;
    FString CoreSDKConfigPath;
    bool EnableHardwareInformationGathering;
};

struct ConfigInformation {
    uint32 FPS;
    uint32 KPS;
    uint32 TotalRecordingSecond;
    FString SubtitleType;
    bool EnableCrashReporter;
    bool StoreDXDiag;
    bool StoreCrashVideo;
    bool EnableBasicProfiling;
    bool EnableCPUProfiling;
    bool EnableGPUProfiling;
    bool EnableMemoryProfiling;
};

struct ProcessInformation {
    uint32 PID;
    FString BlackBoxHelperPath;
    FString CrashFolder;
    FString CrashGUID;
    FString LogSourceFilePath;
};

struct InputInformation {
    TArray<TPair<FString, FKey>> ActionKeyPair;
};

class FInfoManager {
public:
    FInfoManager();
    ~FInfoManager();

    InputInformation& GetKeyInformation();
    void SetupKeyInformation(APlayerController* PlayerCtrl);
    bool IsKeyInformationPresent();
    void ResetKeyInformation();

    CPUInformation& GetCPUInformation();
    void SetCPUInformation(const CPUInformation& CPUInfo_);

    GPUInformation& GetGPUInformation();
    void SetGPUInformation(const GPUInformation& GPUInfo_);

    OSInformation& GetOSInformation();
    void SetOSInformation(const OSInformation& OSInfo_);

    ConfigInformation GetConfigInformation();

    SDKInformation GetSDKInformation();
    void SetSDKInformation(const SDKInformation& SDKInfo);

    ProcessInformation& GetProcessInformation();
    void SetProcessInformation(const ProcessInformation& ProcInfo_);

#if BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX
    static std::string GetGamertag();
#endif

private:
    void UpdateBlackBoxConfiguration(const SDKInformation& SDKInfo_);
    void UpdateBlackBoxClientInformation(const ProcessInformation& ProcInfo_);
    bool IsOSInfoEmpty() const;

private:
    CPUInformation CPUInfo;
    GPUInformation GPUInfo;
    OSInformation OSInfo;
    ProcessInformation ProcInfo;
    InputInformation InputInfo;
};
