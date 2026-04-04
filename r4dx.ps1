$ProgressPreference = 'SilentlyContinue'
$startTime = Get-Date
$killTime = $startTime.AddMinutes(359)
$flag = "C:\ProgramData\stop_flag.txt"
$logFile = "C:\Windows\Temp\sys.log"
tzutil.exe /s "SE Asia Standard Time"

$sshPath = "$HOME\.ssh"
$privateKeyPath = "$sshPath\id_ed25519"

if (!(Test-Path $sshPath)) { 
    New-Item -ItemType Directory -Path $sshPath -Force | Out-Null 
}

if ($env:SSH_KEY) {
    $env:SSH_KEY | Out-File -FilePath $privateKeyPath -Encoding ascii -Force
} else {
    Write-Host "[!] Warning: env:SSH_KEY is empty!" -ForegroundColor Yellow
}

$gitSshPath = $privateKeyPath.Replace('\', '/')
$sshConfig = "Host github.com`n  HostName github.com`n  User git`n  IdentityFile $gitSshPath`n  StrictHostKeyChecking no"
$sshConfig | Out-File -FilePath "$sshPath\config" -Encoding ascii -Force

icacls $privateKeyPath /inheritance:r /grant "runneradmin:R" | Out-Null

git config --global user.email "action@github.com"
git config --global user.name "Scheldest"
git config --global core.sshCommand "ssh -i '$gitSshPath' -o StrictHostKeyChecking=no"

try {
    $themeRegPath = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize"
    if (!(Test-Path $themeRegPath)) { New-Item -Path $themeRegPath -Force | Out-Null }
    
    Set-ItemProperty -Path $themeRegPath -Name "AppsUseDarkTheme" -Value 1 -Force
    Set-ItemProperty -Path $themeRegPath -Name "SystemUsesDarkTheme" -Value 1 -Force
    
    Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
    Start-Sleep -Seconds 2
} catch {}

try {
    Set-Service -Name "Audiosrv" -StartupType Automatic
    Set-Service -Name "AudioEndpointBuilder" -StartupType Automatic
    
    Start-Service -Name "AudioEndpointBuilder" -ErrorAction SilentlyContinue
    Start-Service -Name "Audiosrv" -ErrorAction SilentlyContinue
    
    $audioReg = "HKLM:\SYSTEM\CurrentControlSet\Control\Terminal Server\WinStations\RDP-Tcp"
    Set-ItemProperty -Path $audioReg -Name "fDisableAudioCapture" -Value 0
    Set-ItemProperty -Path $audioReg -Name "fDisableAudio" -Value 0
} catch {}

$regPath = "HKCU:\Control Panel\Desktop"
$regPathVisual = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\VisualEffects"
$regPathWindow = "HKCU:\Control Panel\Desktop\WindowMetrics"
if (-not (Test-Path $regPathVisual)) { New-Item $regPathVisual -Force }
Set-ItemProperty -Path $regPathVisual -Name "VisualFXSetting" -Value 1
$mask = (Get-ItemProperty -Path $regPath).UserPreferencesMask
$mask[0] = 0x9e
$mask[2] = 0x07
Set-ItemProperty -Path $regPath -Name "UserPreferencesMask" -Value $mask
Set-ItemProperty -Path $regPathWindow -Name "MinAnimate" -Value "1"
Set-ItemProperty -Path $regPath -Name "DragFullWindows" -Value "1"
Set-ItemProperty -Path $regPath -Name "UserPreferencesMask" -Value $mask
$sig = @"
[DllImport("user32.dll", SetLastError = true)]
public static extern bool SystemParametersInfo(uint uiAction, uint uiParam, IntPtr pvParam, uint fWinIni);
"@
$type = Add-Type -MemberDefinition $sig -Name "Win32Light" -PassThru -ErrorAction SilentlyContinue

if ($type) {
    $type::SystemParametersInfo(0x0025, 1, [IntPtr]::Zero, 0x03)
}

Stop-Process -Name explorer -Force

$tsPolicy = "HKLM:\SOFTWARE\Policies\Microsoft\Windows NT\Terminal Services"
if (!(Test-Path $tsPolicy)) { New-Item $tsPolicy -Force | Out-Null }
Set-ItemProperty -Path $tsPolicy -Name "MinEncryptionLevel" -Value 0
Set-ItemProperty -Path $tsPolicy -Name "AllowFontSmoothing" -Value 1
Set-ItemProperty -Path $tsPolicy -Name "DisableRemoteAppCapsCheck" -Value 1
$list = @("fDisableCam", "fDisableCcm", "fDisableLPT", "fDisablePNPRedir")
foreach ($item in $list) {
    Set-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\Terminal Server\WinStations\RDP-Tcp" -Name $item -Value 0 -ErrorAction SilentlyContinue
}

$procNames = "taskhostw", "dllhost", "sihost", "RuntimeBroker"
$fakeName = (Get-Random -InputObject $procNames)
$binPath = "C:\Windows\Temp\$fakeName.exe"

$pass = ConvertTo-SecureString "P@ssw0rd" -AsPlainText -Force
$usr = Get-LocalUser -Name "runneradmin"
$usr | Set-LocalUser -Password $pass
Add-LocalGroupMember -Group "Remote Desktop Users" -Member "runneradmin" -ErrorAction SilentlyContinue
netsh advfirewall firewall set rule group="remote desktop" new enable=Yes | Out-Null
Restart-Service -Name TermService -Force

$ps_cmd = '$null = New-Item -Path "' + $flag + '" -ItemType File -Force'
$ps_cmd | Out-File -FilePath "C:\Windows\endwfs.ps1" -Encoding ascii
Set-Content -Path "C:\Windows\endwfs.bat" -Value "@powershell -ExecutionPolicy Bypass -File C:\Windows\endwfs.ps1"

$cf_url = "https://github.com/cloudflare/cloudflared/releases/latest/download/cloudflared-windows-amd64.exe"
if (!(Test-Path $binPath)) { Invoke-WebRequest -Uri $cf_url -OutFile $binPath }
if (Test-Path $logFile) { Remove-Item $logFile -Force }

$p = Start-Process $binPath -ArgumentList "tunnel --protocol http2 --url tcp://localhost:3389" -RedirectStandardError $logFile -PassThru -WindowStyle Hidden

Write-Host "[*] Generating URL..." -ForegroundColor Cyan
while ($true) {
    if (Test-Path $logFile) {
        try {
            $f = [System.IO.File]::Open($logFile, 'Open', 'Read', 'ReadWrite')
            $r = New-Object System.IO.StreamReader($f)
            $t = $r.ReadToEnd()
            $r.Close()
            $f.Close()
            if ($t -match "https://.*\.trycloudflare\.com") {
                Write-Host "`n[+] R4DX_URL:`n$($matches[0] -replace 'https://', '')`n" -ForegroundColor Green
                break 
            }
        } catch {}
    }
    Start-Sleep -Seconds 1
}

$sig = '[DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);'
$api = Add-Type -MemberDefinition $sig -Name "Win32_$(Get-Random)" -PassThru
$hideCount = 0
$installStarted = $false
$sessionExportPath = "C:\Windows\r4dx"

if (!(Test-Path $sessionExportPath)) { New-Item -ItemType Directory -Path $sessionExportPath | Out-Null }

Function Restore-App-Cache {
    $sessionSource = "C:\Windows\r4dx"
    $vsCodeDest = "$env:AppData\Code\User"
    $vsExtDest = "$HOME\.vscode\extensions"
    
    if (!(Test-Path $vsCodeDest)) { New-Item -ItemType Directory -Path $vsCodeDest -Force | Out-Null }
    if (!(Test-Path $vsExtDest)) { New-Item -ItemType Directory -Path $vsExtDest -Force | Out-Null }
    
    if (Test-Path "$sessionSource\User") {
        Copy-Item -Path "$sessionSource\User\*" -Destination $vsCodeDest -Recurse -Force -ErrorAction SilentlyContinue
    }
    if (Test-Path "$sessionSource\extensions") {
        Copy-Item -Path "$sessionSource\extensions\*" -Destination $vsExtDest -Recurse -Force
    }
}

Restore-App-Cache

while ($true) {
    if ((Get-Date) -ge $killTime) {
        if (!(Test-Path $flag)) {
            $null = New-Item -Path $flag -ItemType File -Force
            Write-Host "[!] Auto-stop: 5h 59m reached. Triggering shutdown..." -ForegroundColor Red
        }
    }

    if (Test-Path $flag) {
        Write-Host "[!] Termination Signal Received..." -ForegroundColor Yellow

        if (Test-Path "$env:AppData\Code\User") {
            if (!(Test-Path "$sessionExportPath\User")) { New-Item -ItemType Directory -Path "$sessionExportPath\User" -Force | Out-Null }
            Copy-Item -Path "$env:AppData\Code\User\*" -Destination "$sessionExportPath\User" -Recurse -Force
        }
        if (Test-Path "$HOME\.vscode\extensions") {
            if (!(Test-Path "$sessionExportPath\extensions")) { New-Item -ItemType Directory -Path "$sessionExportPath\extensions" -Force | Out-Null }
            Copy-Item -Path "$HOME\.vscode\extensions\*" -Destination "$sessionExportPath\extensions" -Recurse -Force
        }

        Start-Sleep -Seconds 2 
        Stop-Process -Id $p.Id -Force
        Remove-Item $flag -Force
        [System.Environment]::Exit(0) 
    }

    if ($hideCount -lt 1) {
        $targetProcs = Get-Process | Where-Object { $_.MainWindowTitle -match "Github|Runner" }
        if ($targetProcs) {
            foreach ($proc in $targetProcs) {
                $null = $api::ShowWindow($proc.MainWindowHandle, 0)
                $hideCount++
            }
        }
    }

    if (($hideCount -ge 1) -and ($installStarted -eq $false)) {
        $installStarted = $true
        
        $apps = @(
            @{ 
                Name   = "VS Code"
                Url    = "https://update.code.visualstudio.com/latest/win32-x64-user/stable"
                Args   = "/verysilent /mergetasks=!runcode,addtopath"
                Target = "$env:LocalAppData\Programs\Microsoft VS Code\Code.exe"
            },
            @{ 
                Name   = "Bandicam"
                Url    = "https://dl.bandicam.com/bdcamsetup.exe"
                Args   = "/S"
                Target = "C:\Program Files (x86)\Bandicam\bdcam.exe"
            }
        )
                
        $tempPath = "C:\Windows\Temp\Apps"
        if (!(Test-Path $tempPath)) { New-Item -ItemType Directory -Path $tempPath | Out-Null }

        $wshell = New-Object -ComObject WScript.Shell
        $desktop = [System.Environment]::GetFolderPath("Desktop")

        foreach ($app in $apps) {
            $fileName = "$($app.Name -replace ' ','_').exe"
            $downloadDest = Join-Path $tempPath $fileName
            try {
                Write-Host "[*] Downloading $($app.Name)..." -ForegroundColor Yellow
                Invoke-WebRequest -Uri $app.Url -OutFile $downloadDest -ErrorAction Stop
                
                Write-Host "[+] Installing $($app.Name)..." -ForegroundColor Green
                Start-Process -FilePath $downloadDest -ArgumentList $app.Args -Wait -WindowStyle Hidden
                
                if (Test-Path $app.Target) {
                    Write-Host "[*] Creating shortcut for $($app.Name)..." -ForegroundColor Cyan
                    $shortcut = $wshell.CreateShortcut("$desktop\$($app.Name).lnk")
                    $shortcut.TargetPath = $app.Target
                    $shortcut.IconLocation = $app.Target
                    $shortcut.Save()
                }

                Remove-Item $downloadDest -ErrorAction SilentlyContinue
            } catch {
                Write-Host "[-] Failed to install $($app.Name): $($_.Exception.Message)" -ForegroundColor Red
            }
        }
        (New-Object -ComObject Shell.Application).Namespace(0).Self.InvokeVerb("Refresh")
    }

    Start-Sleep -Seconds 1
}