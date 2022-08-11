
#include configuration
. $PSScriptRoot\00.Configuration.ps1

# dynamic variable will be different on each job

Write-Output "---Step 1: Creaing New version---"

$GameProjectPathAbsolute = Resolve-Path -Path $GameProjectPath

# Execute the command
Write-Output "${BlackBoxCLIPath}  version add --name ${Version} --namespace ${Namespace} --apikey ${APIKey} --game-project ${GameProjectPathAbsolute}"
Invoke-Expression "${BlackBoxCLIPath}  version add --name ${Version} --namespace ${Namespace} --apikey ${APIKey} --game-project ${GameProjectPathAbsolute}"
Write-Output "---Step 1: Creaing New version--- DONE"