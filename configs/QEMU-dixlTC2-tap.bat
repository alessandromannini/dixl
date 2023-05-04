cd D:\lale\documents\Universita\tesi\VxWorks\wrsdk-vxworks7-win-qemu\vxsdk\bsps\itl_generic_3_0_0_1
d:
"c:\Program Files\qemu\qemu-system-x86_64.exe" -m 512M -kernel vxWorks -m 512M -kernel vxWorks -net nic,netdev=mynet0,macaddr=52:54:00:12:34:57 -netdev tap,id=mynet0,ifname=tap_qemu2 -display none -serial stdio -monitor none -append "bootline:fs(0,0)host:vxWorks e=192.168.173.85 g=192.168.173.1 u=target pw=vxTarget o=gei0"
pause
