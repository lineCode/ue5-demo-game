#include configuration
. .\00.Configuration.ps1


Write-Output "---Step 10: Modify Justice configuration for game server---"

Write-Output "Reverting ${DefaultEngineConfigPath}..."
Invoke-Expression "git checkout -- ${DefaultEngineConfigPath}"
Write-Output "Reverting ${DefaultEngineConfigPath}...Done"

Write-Output "Modifying ${DefaultEngineConfigPath}..."
$iniContent = Get-Content -path $DefaultEngineConfigPath
$iniContent = $iniContent.replace('<<game_server_client_id>>', $GameServer_ClientID)
$iniContent = $iniContent.replace('<<game_server_secret>>', $GameServer_ClientSecret)
Set-Content -Path $DefaultEngineConfigPath -Value $iniContent -Force
Write-Output "Modifying ${DefaultEngineConfigPath}...DONE!"

Write-Output "---Step 10: Modify Justice configuration for game server---DONE"