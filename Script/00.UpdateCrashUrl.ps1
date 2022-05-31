
#include configuration
. $PSScriptRoot\00.Configuration.ps1

# dynamic variable will be different on each job

Write-Output "---Step 0: Update Crash Reporter URL---"

$GameProjectPathAbsolute = Resolve-Path -Path $GameProjectPath

# Execute the command
Write-Output "${BlackBoxCLIPath}  crash-url set --game-engine ${UE4GameEnginePath} --namespace ${Namespace} --apikey ${APIKey}"
Invoke-Expression "${BlackBoxCLIPath}  crash-url set --game-engine ${UE4GameEnginePath} --namespace ${Namespace} --apikey ${APIKey}"
Write-Output "---Step 0: Update Crash Reporter--- DONE"