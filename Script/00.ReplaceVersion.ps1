
$param1=$args[0]

if ($param1)
{
    $VersionJSONPath = $PSScriptRoot + "\..\version.json" 
    $VersionJsonObject = Get-Content $VersionJSONPath | ConvertFrom-Json
    $Version = $VersionJsonObject.version
    Write-Output "Current version from version.json: $Version"
    Write-Output "Override version with: $param1"

    $newVersionObject = @{
    version = $param1    
    }
    $jsonString = $newVersionObject | ConvertTo-Json

    Set-Content -Path $VersionJSONPath -Value $jsonString -Force

}
else
{
    Write-Error "Error: Parameter is empty"
    exit -1
}

