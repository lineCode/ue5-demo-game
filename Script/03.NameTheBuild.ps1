#include configuration
. $PSScriptRoot\00.Configuration.ps1

# Dynamic Data
$BuildName = "gameclient-windows"


Write-Output "---Step 3: Name The Build---"
# Execute the command
Invoke-Expression "${BlackBoxCLIPath} build add-metadata --namespace ${Namespace} --apikey ${APIKey} --game-project=${GameProjectPath} --key build-name --value ${BuildName}"
Write-Output "---Step 3: Name The Build---DONE"
