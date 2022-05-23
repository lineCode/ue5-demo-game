// Copyright (c) 2019 - 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

using UnrealBuildTool;
using System;
using System.IO;
using System.Reflection;
using System.Collections.Generic;
public class BlackBoxSDKPlatform_PS4 : BlackBoxSDKPlatform
{
    public BlackBoxSDKPlatform_PS4(ReadOnlyTargetRules inTarget, string inPluginDir, string inEngineDirectory) : base(inTarget, inPluginDir, inEngineDirectory)
    {

    }

#if UE_4_22_OR_LATER
    public override List<string> GetPrivateDefinitions()
    {
        return new List<string>();
    }
    public override List<string> GetPublicDefinitions()
    {
        return new List<string>();
    }
#else
    public override List<string> GetDefinitions()
    {
        return new List<string>();
    }
#endif
    public override List<string> GetRuntimeDependencies()
    {
        return new List<string>();
    }
    public override List<string> GetDebugRuntimeDependencies()
    {
        return new List<string>();
    }
    public override List<string> GetSystemRuntimeDependencies()
    {

        return new List<string>();
    }
    public override Dictionary<string, string> GetMustCopyDebugRuntimeDependencies()
    {
        return new Dictionary<string, string>();
    }

    public override Dictionary<string, string> GetMustCopySystemRuntimeDependencies()
    {
        return new Dictionary<string, string>();
    }

    public override List<string> GetPublicAdditionalLibraries()
    {
        return new List<string>() {
            Path.Combine(PluginDir, "Libs/x64/PS4/BlackBoxSDK-PS4.a"),
        };
    }

    public override List<string> GetPublicDelayLoadDLLs()
    {
        return new List<string>();
    }
    public override List<string> GetPlatformSpecificPrivateDependencyModuleNames()
    {
        return new List<string>();
    }

    public override List<string> GetPublicSystemIncludeDirs()
    {
        return new List<string>();
    }

    public override List<string> GetPublicSystemLibraries()
    {
        return new List<string>();
    }
    public override List<string> GetPrivateIncludeDirs()
    {
        return new List<string>();
    }
    public override List<string> GetEngineThirdPartyPrivateStaticDependencies()
    {
        return new List<string>();
    }
}