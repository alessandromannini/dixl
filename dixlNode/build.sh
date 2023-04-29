source SDK/sdkenv.sh
$CC -dkm dkm.c includes/ntp.c includes/network.c includes/utils.c includes/hw.c datatypes/dataHelper.c FSM/FSMCtrlPOINT.c FSM/FSMCtrlTRACKCIRCUIT.c FSM/FSMInit.c tasks/dixlCommRx.c tasks/dixlCommTx.c tasks/dixlCtrl.c tasks/dixlDiag.c tasks/dixlInit.c tasks/dixlLog.c tasks/dixlPoint.c tasks/dixlSensor.c -o dkm.o  -v
