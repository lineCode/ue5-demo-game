#include configuration
. .\0.Configuration.ps1

Write-Output "---Step 13: Upload  Game Server (Linux) Binaries---"

# Dynamic Data
$GameServerPath = $OutputLinuxGameServerFolder + "\LinuxServer"
$EntryPoint = "TutorialProjectServer.sh"

# Execute the command
Invoke-Expression "${BlackBoxCLIPath} build upload-binaries --entry-point ${EntryPoint} --namespace ${Namespace} --apikey ${APIKey} --game-project=${GameProjectPath} --game-engine=${UE4GameEnginePath} --game-archive=${GameServerPath}"

Write-Output "---Step 13: Upload  Game Server (Linux) Binaries---DONE"