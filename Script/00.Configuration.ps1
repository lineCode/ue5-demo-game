# Basic Variable
$BlackBoxCLIPath = $PSScriptRoot + "\..\Tools\BlackBoxCLI-x64-windows\BlackBoxCLI.exe"
$BlackBoxCLILinuxPath = $PSScriptRoot + "\..\Tools\BlackBoxCLI-linux\BlackBoxCLI"
$GameProjectPath = $PSScriptRoot + "\..\Lyra"
$GameProjectFile = $GameProjectPath + "\LyraStarterGame.uproject"
$Namespace = "demo"
$JusticeNamespace = "abdemogame"

$OutputWindowsGameClientFolder = $PSScriptRoot + "\..\Artifact\WindowGameClient"
$OutputWindowsGameServerFolder = $PSScriptRoot + "\..\Artifact\WindowGameServer"
$OutputLinuxGameServerFolder = $PSScriptRoot + "\..\Artifact\LinuxGameServer"

$DefaultEngineConfigPath = $PSScriptRoot + "\..\Lyra\Config\DefaultEngine.ini"
$ArmadaDSUploaderPath = $PSScriptRoot + "\..\Tools\DSUploader-x64-windows\dsuploader.exe" 

# use version json value for making new version for blackbox and armada build
$VersionJSONPath = $PSScriptRoot + "\..\version.json" 
$VersionJsonObject = Get-Content $VersionJSONPath | ConvertFrom-Json
$Version = $VersionJsonObject.version

# Confidential variable, will be stored on the bitbucket variable
$APIKey = $env:BB_APIKEY
$UE4GameEnginePath = $env:BB_UNREAL_GAME_ENGINE_PATH
$UE4BatchFile = $env:BB_UNREAL_GAME_ENGINE_PATH + "\Engine\Build\BatchFiles\RunUAT.bat"

# Justice Configuration
$GameClient_ClientID = $env:BB_GAME_CLIENT_CLIENT_ID
$GameClient_ClientSecret = $env:BB_GAME_CLIENT_CLIENT_SECRET

$GameServer_ClientID = $env:BB_GAME_SERVER_CLIENT_ID
$GameServer_ClientSecret = $env:BB_GAME_SERVER_CLIENT_SECRET

$DSUploader_ClientID = $env:BB_DS_UPLOADER_CLIENT_ID


# Steam credentials
$SteamCDMPath = $env:BB_STEAM_CMD_PATH
$SteamAccoutName = $env:BB_STEAM_ACCOUNT_NAME
$SteamAccoutPassword = $env:BB_STEAM_ACCOUNT_PASSWORD

