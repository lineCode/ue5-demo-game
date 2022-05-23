#include configuration
. $PSScriptRoot\00.Configuration.ps1

# Dynamic Data
$BuildName = "gameserver-linux"


Write-Output "---Step 9: name the build for Game server---"
# Execute the command
Invoke-Expression "${BlackBoxCLIPath} build add-metadata --namespace ${Namespace} --apikey ${APIKey} --game-project=${GameProjectPath} --key build-name --value ${BuildName}"

Write-Output "---Step 9: name the build for Game server---DONE"