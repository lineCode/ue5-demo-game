function Set-OrAddIniValue
{
    Param(
        [string]$FilePath,
        [hashtable]$keyValueList
    )

    $content = Get-Content $FilePath

    $keyValueList.GetEnumerator() | ForEach-Object {
        if ($content -match "^$($_.Key)=")
        {
            $content= $content -replace "^$($_.Key)=(.*)", "$($_.Key)=$($_.Value)"
        }
        else
        {
            $content += "$($_.Key)=$($_.Value)"
        }
    }

    $content | Set-Content $FilePath
}

# ForceVersion is comming from Bitbucket variable from bitbucket-pipelines.yml
$param1=$env:ForceVersion
$GameIniPath=$PSScriptRoot + "\..\Lyra\Config\DefaultGame.ini"
$VersionJSONPath = $PSScriptRoot + "\..\version.json" 
$VersionJsonObject = Get-Content $VersionJSONPath | ConvertFrom-Json
$Version = $VersionJsonObject.version

if ($param1)
{
    Write-Output "Current version from version.json: $Version"
    Write-Output "Override version with: $param1"

    $newVersionObject = @{
    version = $param1    
    }
    $jsonString = $newVersionObject | ConvertTo-Json

    Set-Content -Path $VersionJSONPath -Value $jsonString -Force
    $Version = $param1
}

Write-Output "Replace Version on DefaultGame.ini"
Set-OrAddIniValue -FilePath $GameIniPath -keyValueList @{
    ProjectVersion = $Version
}