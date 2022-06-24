#include configuration
. $PSScriptRoot\00.Configuration.ps1



Write-Output $DefaultEngineConfigPath

Write-Output "---Step 4: Modifying Justice Confguration for Game Client---"

Write-Output "Reverting ${DefaultEngineConfigPath}..."
Invoke-Expression "git checkout -- ${DefaultEngineConfigPath}"
Write-Output "Reverting ${DefaultEngineConfigPath}...Done"

Write-Output "Modifying ${DefaultEngineConfigPath}..."
$iniContent = Get-Content -path $DefaultEngineConfigPath
$iniContent = $iniContent.replace('<<game_client_client_id>>', $GameClient_ClientID)
$iniContent = $iniContent.replace('<<game_client_client_secret>>', $GameClient_ClientSecret)
$iniContent = $iniContent.replace('<<turn_server_secret>>', $TurnServerSecret)

$iniContent = $iniContent.replace('<<endgame_apikey>>', $EndgameAPIKey)
$iniContent = $iniContent.replace('<<endgame_game_id>>', $EndgameGameID)
$iniContent = $iniContent.replace('<<endgame_namespace_id>>', $EndgameNamespaceID)

Set-Content -Path $DefaultEngineConfigPath -Value $iniContent -Force
Write-Output "Modifying ${DefaultEngineConfigPath}...DONE!"

Write-Output "---Step 4: Modifying Justice Confguration for Game Client---DONE"

