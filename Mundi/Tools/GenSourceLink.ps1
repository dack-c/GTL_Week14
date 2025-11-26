
param(
    [Parameter(Mandatory=$true)]
    [string]$ProjectDir,

    [Parameter(Mandatory=$true)]
    [string]$OutDir
)


$ProjectDir = $ProjectDir.TrimEnd('.').TrimEnd('\')
$OutDir = $OutDir.TrimEnd('.').TrimEnd('\')

if (-not (Test-Path $OutDir)) {
    New-Item -ItemType Directory -Path $OutDir -Force | Out-Null
}

$RepoRoot = git -C $ProjectDir rev-parse --show-toplevel 2>$null
if (-not $RepoRoot) {
    Write-Host "[SourceLink] Git repository not found - skipping"
    exit 0
}

$CommitHash = git -C $ProjectDir rev-parse HEAD 2>$null
if (-not $CommitHash) {
    Write-Host "[SourceLink] Failed to get commit hash - skipping"
    exit 0
}

$RemoteUrl = git -C $ProjectDir remote get-url origin 2>$null
if (-not $RemoteUrl) {
    Write-Host "[SourceLink] Failed to get remote URL - skipping"
    exit 0
}

# https://github.com/owner/repo.git -> https://raw.githubusercontent.com/owner/repo
$RawUrl = $RemoteUrl -replace '\.git$', ''
$RawUrl = $RawUrl -replace 'github\.com', 'raw.githubusercontent.com'

$RepoRoot = $RepoRoot -replace '/', '\'

$Sourcelink = @{
    documents = @{
        "$RepoRoot\*" = "$RawUrl/$CommitHash/*"
    }
}

$OutputPath = Join-Path $OutDir "sourcelink.json"
$Sourcelink | ConvertTo-Json | Set-Content $OutputPath -Encoding UTF8

Write-Host "[SourceLink] Generated: $OutputPath"
Write-Host "[SourceLink] Commit: $CommitHash"
