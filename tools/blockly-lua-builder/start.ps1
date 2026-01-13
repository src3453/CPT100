param(
	[int]$Port = 8000
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $scriptDir

$python = Get-Command python -ErrorAction SilentlyContinue
if ($python) {
	Write-Host "Starting Python http.server on port $Port ..."
	Write-Host "Serving files from: $scriptDir"
	Write-Host "Open http://localhost:$Port/index.html in your browser"
	Start-Process powershell -ArgumentList "-NoProfile","-ExecutionPolicy","Bypass","-Command","cd `"$scriptDir`"; python -m http.server $Port" -WindowStyle Minimized
	Start-Sleep -Seconds 1
	Start-Process "http://localhost:$Port/index.html"
} else {
	Write-Host "Python not found. Opening index.html directly..."
	Start-Process "$scriptDir/index.html"
}