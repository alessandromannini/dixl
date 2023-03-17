source SDK/sdkenv.sh
$CC dkm.c network.c utils.c datatypes/dataHelper.c FSM/FSMCtrlPOINT.c FSM/FSMCtrlTRACKCIRCUIT.c FSM/FSMInit.c tasks/dixlCommRx.c tasks/dixlCommTx.c tasks/dixlCtrl.c tasks/dixlDiag.c tasks/dixlInit.c tasks/dixlLog.c tasks/dixlPoint.c -o dkm.o
