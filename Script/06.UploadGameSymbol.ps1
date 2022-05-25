#include configuration
. $PSScriptRoot\00.Configuration.ps1

# Dynamic Data
$Platform = "windows"
$GameClientPath = $OutputWindowsGameClientFolder + "\Windows"
$GameClientPathAbsolute = Resolve-Path -Path $GameClientPath

Write-Output "---Step 6: Uploading Game Symbol for Game client---"
# Execute the command
Invoke-Expression "${BlackBoxCLIPath} upload --namespace ${Namespace} --apikey ${APIKey} --platform-arch x64 --platform-name ${Platform} --game-project=${GameProjectPath} --game-engine=${UE4GameEnginePath} --game-archive=${GameClientPathAbsolute}"

Write-Output "---Step 6: Uploading Game Symbol for Game client---DONE"
