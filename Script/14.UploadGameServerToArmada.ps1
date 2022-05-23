#include configuration
. $PSScriptRoot\00.Configuration.ps1

Write-Output "---Step 14: Upload  Game Server (Linux) Binaries to Armada---"

# Dynamic Data
$GameServerPath = $OutputLinuxGameServerFolder + "\LinuxServer"
$EntryPoint = "TutorialProjectServer.sh"

# there is a bug on DSUploader if the Path Parameter contain relative path such as /../
$GameServerPathAbsolute = Resolve-Path -Path $GameServerPath

# Execute the command
Invoke-Expression "${ArmadaDSUploaderPath} syncFolder --namespace blackboxmultiplayergames --hostname https://demo.accelbyte.io --command ${EntryPoint} --path ${GameServerPathAbsolute} --version ${Version} --s3dirname BlackBoxMultiplayerGameClean --id ${DSUploader_ClientID} --bucket justice-ds-upload-service-demo --debug-enabled --show-progress"


Write-Output "---Step 14: Upload  Game Server (Linux) Binaries to Armada---DONE"