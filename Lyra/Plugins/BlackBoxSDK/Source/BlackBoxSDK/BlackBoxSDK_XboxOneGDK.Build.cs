// Copyright (c) 2019 - 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

using UnrealBuildTool;
using System;
using System.IO;
using System.Reflection;
using System.Collections.Generic;
public class BlackBoxSDKPlatform_XboxOneGDK : BlackBoxSDKPlatform
{
    private string XCurlLibDir;
    private string XCurlDllDir;
    private string XCurlExtensionDir;
    private string XSAPIExtensionDir;

    private void SetupXCurlDir()
    {
        System.Type GDKExportsClass = System.Type.GetType("UnrealBuildTool.GDKExports,UnrealBuildTool");
        if (GDKExportsClass != null)
        {
            var GetExtDirMethod = GDKExportsClass.GetMethod("GetExtensionDirectory");
            if (GetExtDirMethod != null)
            {
                XCurlExtensionDir = GetExtDirMethod.Invoke(null, new object[] { "Xbox.XCurl.API", false }) as string;
                if (XCurlExtensionDir != null && Directory.Exists(XCurlExtensionDir))
                {
                    // Add XCurl as a system library
                    XCurlLibDir = Path.Combine(XCurlExtensionDir, "Lib");
                    // Add the XCurl DLL as a runtime dependency that will be copied to the output dir
                    XCurlDllDir = GetExtDirMethod.Invoke(null, new object[] { "Xbox.XCurl.API", true }) as string;
                }
            }
        }
    }

    private void SetupXSAPIDir()
    {
        System.Type GDKExportsClass = System.Type.GetType("UnrealBuildTool.GDKExports,UnrealBuildTool");
        if (GDKExportsClass != null)
        {
            var GetExtDirMethod = GDKExportsClass.GetMethod("GetExtensionDirectory");
            if (GetExtDirMethod != null)
            {
                XSAPIExtensionDir = GetExtDirMethod.Invoke(null, new object[] { "Xbox.Services.API.C", false }) as string;
            }
        }
    }

    public BlackBoxSDKPlatform_XboxOneGDK(ReadOnlyTargetRules inTarget, string inPluginDir, string inEngineDirectory) : base(inTarget, inPluginDir, inEngineDirectory)
    {
        SetupXCurlDir();
        SetupXSAPIDir();
    }

    private string GetDLLsDirPath()
    {
        return Path.Combine(PluginDir, "DLLs/x64/XBCommon");
    }

    private string GetLibsDirPath()
    {
        return Path.Combine(PluginDir, "Libs/x64/XBCommon");
    }

    public override List<string> GetPrivateDefinitions()
    {
        return new List<string>
        {
            "BLACKBOX_USE_SHARED_LIBRARY",
            // Uncomment this to enable blackbox crash handling on xbox
            //"BLACKBOX_ENABLE_XBOX_GDK_CRASH_REPORT"
        };
    }
    
    public override List<string> GetPublicDefinitions()
    {
        return new List<string>
        {
            "_CRT_SECURE_NO_WARNINGS"
        };
    }

    public override List<string> GetRuntimeDependencies()
    {
        return new List<string>
        {
            Path.Combine(GetDLLsDirPath(), "BlackBoxSDK-XboxSeriesX.dll")
        };
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
        return new Dictionary<string, string>
        {
            {"$(TargetOutputDir)/XCurl.pdb", Path.Combine(XCurlDllDir, "XCurl.pdb")}
        };
    }

    public override Dictionary<string, string> GetMustCopySystemRuntimeDependencies()
    {
        return new Dictionary<string, string>
        {
            {"$(TargetOutputDir)/XCurl.dll", Path.Combine(XCurlDllDir, "XCurl.dll")},
            {"$(TargetOutputDir)/BlackBoxSDK-XboxSeriesX.dll", Path.Combine(GetDLLsDirPath(), "BlackBoxSDK-XboxSeriesX.dll")}
        };
    }

    public override List<string> GetPublicAdditionalLibraries()
    {
        return new List<string>
        {
            Path.Combine(GetLibsDirPath(), "BlackBoxSDK-XboxSeriesX.lib")
        };
    }

    public override List<string> GetPublicDelayLoadDLLs()
    {
        return new List<string>
        {
            "BlackBoxSDK-XboxSeriesX.dll",
            "XCurl.dll"
        };
    }
    public override List<string> GetPlatformSpecificPrivateDependencyModuleNames()
    {
        return new List<string>
        {
            "GDKCore"
        };
    }

    public override List<string> GetPublicSystemIncludeDirs()
    {
        // Add the XCurl headers as system includes
        List<string> IncludeDirs = new List<string>();
        IncludeDirs.Add(Path.Combine(XCurlExtensionDir, "Include"));
        if (XSAPIExtensionDir != null)
        {
            IncludeDirs.Add(Path.Combine(XSAPIExtensionDir, "Include"));
        }
        return IncludeDirs;
    }

    public override List<string> GetPublicSystemLibraries()
    {
        return new List<string>
        {
            Path.Combine(XCurlLibDir, "XCurl.lib")
        };
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