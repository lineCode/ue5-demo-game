
#include configuration
. .\0.Configuration.ps1

# dynamic variable will be different on each job

Write-Output "---Step 1: Creaing New version---"

# Execute the command
Invoke-Expression "${BlackBoxCLIPath}  version add --name ${Version} --namespace ${Namespace} --apikey ${APIKey} --game-project ${GameProjectPath}"
# Write-Output "${BlackBoxCLIPath}  version add --name ${Version} --namespace ${Namespace} --apikey ${APIKey} --game-project ${GameProjectPath}"

Write-Output "---Step 1: Creaing New version--- DONE"