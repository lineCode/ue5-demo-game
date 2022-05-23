#include configuration
. .\00.Configuration.ps1

Write-Output "---Step 12: Upload game symbol for Game Server (Linux) Executable using WSL---"

# Dynamic Data
$GameServerPath = $OutputLinuxGameServerFolder + "\LinuxServer"
$Platform = "linux-server"

$BlackBoxCLIAbsolute = Resolve-Path -Path $BlackBoxCLILinuxPath
$GameProjectAbsolute = Resolve-Path -Path $GameProjectPath
$GameEngineAbsolute = Resolve-Path -Path $UE4GameEnginePath
$GameServerPathAbsolute = Resolve-Path -Path $GameServerPath


$BlackBoxCLIAbsoluteString = $BlackBoxCLIAbsolute.Path.replace('\','/')
$found = $BlackBoxCLIAbsoluteString -match "(.):"
if ($found) {
    $BBXDrive = $matches[1]
    $BlackBoxCLIAbsoluteString = $BlackBoxCLIAbsoluteString.Replace($BBXDrive + ':/', "/mnt/" + $BBXDrive.ToLower() + "/")
}


$GameProjectAbsoluteString = $GameProjectAbsolute.Path.replace('\','/')
$found = $GameProjectAbsoluteString -match "(.):"
if ($found) {
    $GameProjecDrive = $matches[1]
    $GameProjectAbsoluteString = $GameProjectAbsoluteString.Replace($GameProjecDrive + ':/', "/mnt/" + $GameProjecDrive.ToLower() + "/")
}

$GameEngineAbsoluteString = $GameEngineAbsolute.Path.replace('\','/')
$found = $GameEngineAbsoluteString -match "(.):"
if ($found) {
    $GameEngineDrive = $matches[1]
    $GameEngineAbsoluteString = $GameEngineAbsoluteString.Replace($GameEngineDrive + ':/', "/mnt/" + $GameEngineDrive.ToLower() + "/")
}


$GameServerPathAbsoluteString = $GameServerPathAbsolute.Path.replace('\','/')
$found = $GameServerPathAbsoluteString -match "(.):"
if ($found) {
    $GameServerDrive = $matches[1]
    $GameServerPathAbsoluteString = $GameServerPathAbsoluteString.Replace($GameServerDrive + ':/', "/mnt/" + $GameServerDrive.ToLower() + "/")
}


# Execute the command using BlackBoxCLI linux version on WSL
Invoke-Expression "wsl ${BlackBoxCLIAbsoluteString} upload --namespace ${Namespace} --apikey ${APIKey} --platform-arch x64 --platform-name ${Platform} --game-project=${GameProjectAbsoluteString} --game-engine=${GameEngineAbsoluteString} --game-server-archive=${GameServerPathAbsoluteString}"


Write-Output "---Step 12: Upload game symbol for Game Server (Linux) Executable using WSL---DONE"