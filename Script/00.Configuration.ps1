# Basic Variable
$BlackBoxCLIPath = $PSScriptRoot + "\..\Tools\BlackBoxCLI-x64-windows-2.3.0\BlackBoxCLI.exe"
$BlackBoxCLILinuxPath = $PSScriptRoot + "\..\Tools\BlackBoxCLI-linux-2.3.0\BlackBoxCLI"
$GameProjectPath = $PSScriptRoot + "\..\TutorialProject"
$GameProjectFile = $GameProjectPath + "\TutorialProject.uproject"
$Namespace = "avengers"
$JusticeNamespace = "blackboxmultiplayergames"
$UE4BatchFile = "E:\UE4-Github-4.27.2-release\Engine\Build\BatchFiles\RunUAT.bat"
$OutputWindowsGameClientFolder = $PSScriptRoot + "\..\Artifact\WindowGameClient"
$OutputWindowsGameServerFolder = $PSScriptRoot + "\..\Artifact\WindowGameServer"
$OutputLinuxGameServerFolder = $PSScriptRoot + "\..\Artifact\LinuxGameServer"

$DefaultEngineConfigPath = $PSScriptRoot + "\..\TutorialProject\Config\DefaultEngine.ini"
$ArmadaDSUploaderPath = $PSScriptRoot + "\..\Tools\DSUploader-x64-windows-1.1.2\dsuploader.exe" 

# use version json value for making new version for blackbox and armada build
$VersionJSONPath = $PSScriptRoot + "\..\version.json" 
$VersionJsonObject = Get-Content $VersionJSONPath | ConvertFrom-Json
$Version = $VersionJsonObject.version

# Confidential variable, will be stored on the bitbucket variable
$APIKey = $BB_APIKEY
$UE4GameEnginePath = $BB_UNREAL_GAME_ENGINE_PATH

# Justice Configuration
$GameClient_ClientID = $BB_GAME_CLIENT_CLIENT_ID
$GameClient_ClientSecret = $BB_GAME_CLIENT_CLIENT_SECRET

$GameServer_ClientID = $BB_GAME_SECRET_CLIENT_ID
$GameServer_ClientSecret = $BB_GAME_SECRET_CLIENT_SECRET

$DSUploader_ClientID = $BB_DS_UPLOADER_CLIENT_ID


