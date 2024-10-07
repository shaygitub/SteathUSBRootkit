# STAGER/INSTALLER SHOULD WRITE ALL DRIVER FILES' HARDCODED VALUE INTO C:\Windows\System32\drivers\BootStart
copy C:\Windows\System32\drivers\BootStart\BootStart.sys C:\Windows\System32\drivers\BootStart.sys
# pnputil /add-driver C:\Windows\System32\drivers\BootStart\BootStart.inf /install
sc stop BootStart
sc delete BootStart
sc create BootStart type=kernel start=boot binPath=C:\Windows\System32\drivers\BootStart.sys
sc config BootStart start= boot