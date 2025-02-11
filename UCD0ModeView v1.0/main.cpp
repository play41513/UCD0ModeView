// ---------------------------------------------------------------------------
#include "USBConnect.h"
#include "LOGFile.h"
#include <vcl.h>
#include <conio.h>
#pragma hdrstop

#include "main.h"
#include "Iphlpapi.h"
#include <iostream>

using namespace std;
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
#pragma comment(lib,"Iphlpapi.lib")
/*
停止傳給對方
*/
TfrmMain *frmMain;
bool bLoadDevData[5] = {
	false
};
DWORD dwStep = STEP_REFRESH_USB_DEV;
DWORD dwTimeOut,dwStartTime;
bool bSwitching = false;
bool bNetWorkServer = false;
AnsiString strRecBuf;
// ---------------------------------------------------------------------------
__fastcall TfrmMain::TfrmMain(TComponent* Owner) : TForm(Owner) {
	for (int i = 0; i < 4; i++) {
		TListHUB_Dev[i] = new TStringList;
		TListUSB_Dev[i] = new TStringList;
		TListScreen_Dev[i] = new TStringList;
	}
	TListCheck_DevUSB = new TStringList;

	//
	USBConn = new CUSBConn();
	LOGFile = new CLOGFile();
	LoadINI();
	ShowSettingForm();
}

// ---------------------------------------------------------------------------
void __fastcall TfrmMain::TimeServerTimer(TObject *Sender) {
	TimeServer->Enabled = false;
	USBConn->EnumHUB(false, lbxHUB, bLoadDevData[dwModeIndex]);
	USBConn->EnumUSB(false, lbxUSB, bLoadDevData[dwModeIndex]);
	USBConn->getMonitor(false, plScreenInfo, bLoadDevData[dwModeIndex]);
	//
	RefreshTime();
	switch(dwStep)
	{
		case STEP_REFRESH_USB_DEV:
			break;
		case STEP_WAIT_DEV_PLUG_IN:
			if (USBConn->CheckListBoxData(TListCheck_DevUSB, lbxUSB, false))
			{
				//初始化參數,畫面
				dwModeIndex = dwModeInit;
				for (DWORD i = 1; i <= 4; i++) {
					TPanel* plMode = (TPanel*)frmMain->FindComponent
						("plMode" + AnsiString(i));
					plMode->Color = clYellow;
					if(dwModeInit == i)
						plMode1Click(plMode);
				}
				dwNextModeIndex = 0;
				ChangeMsgBarUI(false, 1, clNavy);
				plResult->Color = clCream;
				plResult->Caption = "Testing...";
				LOGFile->ClearLOGContent();
				LOGFile->astrErrorMsg = "";
				//
				if(strRecBuf != "[AS]STATUS_READY" && strRecBuf != "[AS]STATUS?")
					strRecBuf = "";
				if(bNetWorkServer)
					SendNetworkMsg("[AS]STATUS?");
				dwStep = STEP_WAITING_DUT_READY;
				DEBUG("\r\n[STEP_WAITING_DUT_READY]");
				dwTimeOut = GetTickCount() + dwSwitchModeTimeOut[dwModeIndex];
				dwStartTime = GetTickCount();
			}
			else if(strRecBuf.Pos("[AS]TEST_STOP"))
			{
				StopTest(false);
			}
			else if(strRecBuf.Pos("[AS]NO_RESPOND"))
			{
				DEBUG_NETWORD("(!)NERWORK_ABORMAL");
				LOGFile->astrErrorMsg = "(!)NERWORK_ABORMAL";
				dwStep = STEP_RESULT_FAIL;
			}
			break;
		case STEP_WAITING_DUT_READY:
			//確認server、client溝通
			if(bNetWorkServer)
			{
				if(strRecBuf.Pos("[AS]STATUS_READY"))
				{
					strRecBuf = "";
					dwTimeOut = 0;
					bSwitching = true;
					dwStep = STEP_SWITCH_MODE_TIMEOUT;
					break;
				}
			}
			else
			{
				if(strRecBuf.Pos("[AS]STATUS?"))
				{
					strRecBuf = "";
					SendNetworkMsg("[AS]STATUS_READY");

					dwTimeOut = 0;
					bSwitching = true;
					dwStep = STEP_SWITCH_MODE_TIMEOUT;
					DEBUG("[STEP_SWITCH_MODE_TIMEOUT]");
					break;
				}
			}
			if(GetTickCount() > dwTimeOut)
			{
				DEBUG_NETWORD("(!)NERWORK_ABORMAL");
				LOGFile->astrErrorMsg = "(!)NERWORK_ABORMAL";
				SendNetworkMsg("[AS]NO_RESPOND");
				dwStep = STEP_RESULT_FAIL;
			}
			else if(strRecBuf.Pos("[AS]TEST_STOP"))
			{
				DEBUG_NETWORD("(!)TEST_STOP");
				LOGFile->astrErrorMsg = "(!)TEST_STOP";
				dwStep = STEP_RESULT_FAIL;
			}
			else if(strRecBuf.Pos("[AS]NOT_FIND"))
			{
				DEBUG_NETWORD("(!)OTHER_NOT_FIND_DEV");
				LOGFile->astrErrorMsg = "(!)OTHER_NOT_FIND_DEV";
				dwStep = STEP_RESULT_FAIL;
			}
			break;
		case STEP_SWITCH_MODE_TIMEOUT:
			//確認MODE
			if (GetTickCount() > dwTimeOut)
			{
				if(dwModeIndex == dwModeInit &&!CheckPDVersion())
				{   //Check PD-Version
					AnsiString sTemp = "PD版本異常:"+astrNowPDVersion;
					DEBUG("(!)PD_VERSION_FAIL:"+astrNowPDVersion);
					LOGFile->astrErrorMsg = "(!)PD_VERSION_FAIL:"+astrNowPDVersion;
					SendNetworkMsg("[AS]PD_FAIL");
					ERROR_MSG(sTemp);
					dwStep = STEP_RESULT_FAIL;
					break;
				}
				dwStep = STEP_CHECK_MODE;
				DEBUG("[STEP_CHECK_MODE]");
				dwTimeOut = GetTickCount() + dwModeTimeOut[dwModeIndex];
				bModePass  = false;
				bSwitching = false;
			}
			if (!USBConn->CheckListBoxData(TListCheck_DevUSB, lbxUSB, false)) { // 找不到裝置
				for (int i = 0; i < 3; i++) {
					Delay(1000);
					if (!USBConn->CheckListBoxData(TListCheck_DevUSB, lbxUSB, false)) {
						DEBUG("(!)NOT_FIND_DEV");
						SendNetworkMsg("[AS]NOT_FIND");
						LOGFile->astrErrorMsg = "(!)NOT_FIND_DEV";
						dwStep = STEP_RESULT_FAIL;
					}
				}
			}
			break;
		case STEP_CHECK_MODE:
			dwResult = CheckDevMode(dwModeIndex);
			if (dwResult != UNKNOW_DEV_MODE) {
				if (dwModeIndex == 0)
					plMode1Click(FindComponent("plMode" + AnsiString(dwResult)));
				if (plScreenInfo->Caption != "") {
					//辨識到須確認螢幕，跳至確認螢幕case
					plMSGBOX->Visible = true;
					dwScreenResult = RESULT_WAIT_SCREEN;
					dwStep = STEP_CHECK_SCREEN;
					DEBUG("[STEP_CHECK_SCREEN]");
					break;
				}
				//
				SendNetworkMsg("[AS]MODE_PASS");
				bModePass  = true;
			}
			else if (GetTickCount() > dwTimeOut)
			{
				if (dwModeIndex == 0)
				{
					ChangeMsgBarUI(false, 1, clRed);
					LOGFile->astrErrorMsg = "(!)TIMEOUT_FAIL";
					ClearUI();
				}
				else
				{
					LOGFile->AddLOGContent("MODE"+AnsiString(dwModeIndex),"FAIL");
					ChangeMsgBarUI(false, 0, clRed);
				}
				DEBUG("(!)TIMEOUT_FAIL");
				SendNetworkMsg("[AS]MODE_TIMEOUT");
				dwStep = STEP_RESULT_FAIL;
			}
			if(dwStep != STEP_RESULT_FAIL)
				dwStep = STEP_WAITING_TEST_RESULT;
			break;
		case STEP_WAITING_TEST_RESULT:
			//
			if(strRecBuf.Pos("[AS]MODE_PASS"))
			{
				if(bModePass)
				{
					dwTimeOut = 0;
					//目前MODE確認
					TPanel* plMode = (TPanel*)FindComponent
						("plMode" + AnsiString(dwResult));
					plMode->Color = clGreen;
					//確認下個MODE
					dwNextModeIndex = CheckNextMode(dwResult);
					ChangeMsgBarUI(false, 5, clGreen);
					LOGFile->AddLOGContent("MODE"+AnsiString(dwResult),"PASS");
					dwStep = dwNextModeIndex == STEP_DEV_MODE_OVER ?
						STEP_DEV_MODE_OVER : STEP_SWITCH_MODE;
					if(dwStep == STEP_DEV_MODE_OVER)
					{   //最後一個MODE測試完成 寫LOG
						DEBUG("[STEP_DEV_MODE_OVER]");
						if(!LOGFile->writeLOG("PASS",ImgDisk,moDebug))
							MessageBoxA(Handle,"請確認LOG隨身碟狀態","WARNING", MB_ICONEXCLAMATION);
						plResult->Color = clGreen;
						plResult->Caption = "PASS";
						DEBUG(float(GetTickCount()-dwStartTime)/1000);
					}
					else
						DEBUG("[STEP_SWITCH_MODE]");
					break;
				}
			}
			else if(strRecBuf.Pos("[AS]MODE_FAIL"))
			{
				DEBUG_NETWORD("(!)OTHER_RESULT_FAIL");
				LOGFile->astrErrorMsg = "(!)OTHER_RESULT_FAIL";
				dwStep = STEP_RESULT_FAIL;
			}
			else if(strRecBuf.Pos("[AS]PD_FAIL"))
			{
				DEBUG_NETWORD("(!)OTHER_PD_FAIL");
				LOGFile->astrErrorMsg = "(!)OTHER_PD_FAIL";
				dwStep = STEP_RESULT_FAIL;
			}
			else if(strRecBuf.Pos("[AS]MODE_TIMEOUT"))
			{
				DEBUG_NETWORD("(!)OTHER_TEST_TIMEOUT");
				LOGFile->astrErrorMsg = "(!)OTHER_TEST_TIMEOUT";
				dwStep = STEP_RESULT_FAIL;
			}
			else if(strRecBuf.Pos("[AS]NOT_FIND"))
			{
				DEBUG_NETWORD("(!)OTHER_NOT_FIND_DEV");
				LOGFile->astrErrorMsg = "(!)OTHER_NOT_FIND_DEV";
				dwStep = STEP_RESULT_FAIL;
			}
			else if(strRecBuf.Pos("[AS]TEST_STOP"))
			{
				DEBUG_NETWORD("(!)OTHER_TEST_STOP");
				LOGFile->astrErrorMsg = "(!)OTHER_TEST_STOP";
				dwStep = STEP_RESULT_FAIL;
			}
			else if(GetTickCount() > dwTimeOut)
			{
				DEBUG_NETWORD("(!)NERWORK_ABORMAL");
				LOGFile->astrErrorMsg = "(!)NERWORK_ABORMAL";
				dwStep = STEP_RESULT_FAIL;
			}
			else if (!USBConn->CheckListBoxData(TListCheck_DevUSB, lbxUSB, false))
			{ // 找不到裝置
				for (int i = 0; i < 3; i++) {
					Delay(1000);
					if (!USBConn->CheckListBoxData(TListCheck_DevUSB, lbxUSB, false)) {
						DEBUG("(!)NOT_FIND_DEV");
						LOGFile->astrErrorMsg = "(!)NOT_FIND_DEV";
						SendNetworkMsg("[AS]NOT_FIND");
						dwStep = STEP_RESULT_FAIL;
					}
				}
			}
			else if(bModePass)
				break;

			if(dwStep != STEP_RESULT_FAIL)
			{
				if(plMSGBOX->Visible == true)
					dwStep = STEP_CHECK_SCREEN;
				else
					dwStep = STEP_CHECK_MODE;
			}

			break;
		case STEP_SWITCH_MODE:
			// 切換至下一個
			plMode1Click(FindComponent("plMode" + AnsiString(dwNextModeIndex)));
			// dwModeIndex 會變更
			ChangeMsgBarUI(true, 0, clGray);
			strRecBuf = "";
			Delay(200);
			ChangeMsgBarUI(false, 1, clNavy);
			dwTimeOut = GetTickCount() + dwSwitchModeTimeOut[dwModeIndex];
			bSwitching = true;

			dwStep = STEP_SWITCH_MODE_TIMEOUT;
			break;
		case STEP_CHECK_SCREEN:
			if (dwScreenResult == RESULT_SCREEN_PASS) {//PASS
				plMSGBOX->Visible = false;
				if(bKeyBoardSwitch[dwModeIndex])
				{
					DEBUG_NETWORD("STEP_KEYBOARD_SWITCH");
					dwTimeOut = GetTickCount() + dwKeyBoardSwitchTimeOut[dwModeIndex];
					dwStep = STEP_KEYBOARD_SWITCH;
					WAIT_KEYBOARD_MSG("雙擊 左Shift 切換模式");
				}
				else
				{
					bModePass = true;
					SendNetworkMsg("[AS]MODE_PASS");
					dwStep = STEP_WAITING_TEST_RESULT;
                }
			}
			else if (dwScreenResult == RESULT_SCREEN_FAIL) { // FAIL
				DEBUG("(!)SCREEN_FAIL");
				SendNetworkMsg("[AS]MODE_FAIL");
				dwStep = STEP_RESULT_FAIL;
				ChangeMsgBarUI(false, 4, clRed);
				plMSGBOX->Visible = false;
				LOGFile->astrErrorMsg = "(!)SCREEN_FAIL";
				LOGFile->AddLOGContent("MODE"+AnsiString(dwResult),"FAIL");
			}
			else if(strRecBuf.Pos("[AS]TEST_STOP"))
			{
				DEBUG_NETWORD("(!)TEST_STOP");
				LOGFile->astrErrorMsg = "(!)TEST_STOP";
				dwStep = STEP_RESULT_FAIL;
			}
			else if (GetTickCount() > dwTimeOut) { // TIMEOUT
				DEBUG("(!)SCREEN_TIMEOUT_FAIL");
				SendNetworkMsg("[AS]MODE_TIMEOUT");
				dwStep = STEP_RESULT_FAIL;
				plMSGBOX->Visible = false;
				ChangeMsgBarUI(false, 4, clRed);
				LOGFile->astrErrorMsg = "(!)SCREEN_TIMEOUT_FAIL";
				LOGFile->AddLOGContent("MODE"+AnsiString(dwResult),"FAIL");
			}
			if (!USBConn->CheckListBoxData(TListCheck_DevUSB, lbxUSB, false)) { // 找不到裝置
				for (int i = 0; i < 3; i++) {
					Delay(1000);
					if (!USBConn->CheckListBoxData(TListCheck_DevUSB, lbxUSB, false)) {
						DEBUG("(!)NOT_FIND_DEV");
						LOGFile->astrErrorMsg = "(!)NOT_FIND_DEV";
						SendNetworkMsg("[AS]NOT_FIND");
						LOGFile->AddLOGContent("MODE"+AnsiString(dwResult),"FAIL");
						dwStep = STEP_RESULT_FAIL;
						plMSGBOX->Visible = false;
					}
				}
			}
			break;
		case STEP_KEYBOARD_SWITCH:
			if(GetAsyncKeyState(VK_LSHIFT))
			{
				bModePass = true;
				SendNetworkMsg("[AS]MODE_PASS");
				dwStep = STEP_WAITING_TEST_RESULT;
				plErrorMsgWindows->Visible = false;
			}
			else if(strRecBuf.Pos("[AS]TEST_STOP"))
			{
				DEBUG_NETWORD("(!)TEST_STOP");
				LOGFile->astrErrorMsg = "(!)TEST_STOP";
				dwStep = STEP_RESULT_FAIL;
				plErrorMsgWindows->Visible = false;
			}
			else if (GetTickCount() > dwTimeOut)
			{ // TIMEOUT
				DEBUG("(!)KEYBOARD_SWITCH_TIMEOUT_FAIL");
				SendNetworkMsg("[AS]MODE_TIMEOUT");
				dwStep = STEP_RESULT_FAIL;
				plMSGBOX->Visible = false;
				ChangeMsgBarUI(false, 4, clRed);
				LOGFile->astrErrorMsg = "(!)KEYBOARD_SWITCH_TIMEOUT_FAIL";
				LOGFile->AddLOGContent("MODE"+AnsiString(dwResult),"FAIL");
				plErrorMsgWindows->Visible = false;
			}
			else if (!USBConn->CheckListBoxData(TListCheck_DevUSB, lbxUSB, false)) { // 找不到裝置
				for (int i = 0; i < 3; i++) {
					Delay(1000);
					if (!USBConn->CheckListBoxData(TListCheck_DevUSB, lbxUSB, false)) {
						DEBUG("(!)NOT_FIND_DEV");
						LOGFile->astrErrorMsg = "(!)NOT_FIND_DEV";
						SendNetworkMsg("[AS]NOT_FIND");
						LOGFile->AddLOGContent("MODE"+AnsiString(dwResult),"FAIL");
						dwStep = STEP_RESULT_FAIL;
						plMSGBOX->Visible = false;
						plErrorMsgWindows->Visible = false;
					}
				}
			}
			break;
		case STEP_DEV_MODE_OVER:
			if (!USBConn->CheckListBoxData(TListCheck_DevUSB, lbxUSB, false))
			{
				ClearUI();
				strRecBuf = "";
				plResult->Color = clCream;
				plResult->Caption = "Waiting...";
				ChangeMsgBarUI(true, 0, clGray);
				dwStep = STEP_WAIT_DEV_PLUG_IN;
			}
			else if(strRecBuf.Pos("[AS]TEST_STOP"))
			{
				StopTest(false);
			}
			break;
		case STEP_RESULT_FAIL:
			dwTimeOut = 0;
			DEBUG("[STEP_RESULT_FAIL]");
			DEBUG(float(GetTickCount()-dwStartTime)/1000);
			if (dwModeIndex == 0)
				dwModeIndex = 1;
			TPanel* plMode = (TPanel*)FindComponent
				("plMode" + AnsiString(dwModeIndex));
			plMode1Click(plMode);
			plMode->Color = clRed;
			plResult->Color = clRed;
			plResult->Caption = "FAIL";
			dwStep = STEP_DEV_MODE_OVER;
			if(!LOGFile->writeLOG("FAIL",ImgDisk,moDebug))
				MessageBoxA(Handle,"請確認LOG隨身碟狀態","WARNING", MB_ICONEXCLAMATION);
			if(LOGFile->astrErrorMsg == "(!)TEST_STOP")
				StopTest(false);
			break;
	}
	frmMain->Refresh();
	TimeServer->Enabled = true;
}
// ---------------------------------------------------------------------------

void __fastcall TfrmMain::lbxHUBDrawItem(TWinControl *Control, int Index,
	TRect &Rect, TOwnerDrawState State) {
	TListBox *lbx = (TListBox*)Control;
	if (lbx == NULL)
		return;
	bool bPass = lbx->Items->Strings[Index].Pos("[O]") ? true : false;
	if (bPass)
		lbx->Canvas->Font->Color = clBlue;
	else {
		if (lbx->Items->Strings[Index].Pos("[X]"))
			lbx->Canvas->Font->Color = clGrayText;
		else
			lbx->Canvas->Font->Color = clRed;
	}
	lbx->Canvas->TextRect(Rect, 1, Rect.Top + 1, lbx->Items->Strings[Index]);
}

// ---------------------------------------------------------------------------
void __fastcall TfrmMain::plLoadDevInfoClick(TObject *Sender) {
	if (plScreenInfo->Font->Color == clGrayText)
	{
		plScreenInfo->Caption = "";
	}
	TListScreen_Dev[dwModeIndex - 1]->Text = plScreenInfo->Caption;
	plScreenInfo->Hint = plScreenInfo->Caption;
	// 加入額外裝置
	AnsiString strTemp;
	strTemp = StringReplace(lbxHUB->Items->Text, "[E]", "[O]",
		TReplaceFlags() << rfReplaceAll);
	lbxHUB->Items->Text = strTemp;
	// 清空未長出裝置
	for (int i = 0; i < lbxHUB->Count; i++) {
		if (lbxHUB->Items->Strings[i].Pos("[X]")) {
			lbxHUB->Items->Delete(i);
			i -= 1;
		}
	}
	TListHUB_Dev[dwModeIndex - 1]->Text = lbxHUB->Items->Text;
	// 加入額外裝置
	strTemp = StringReplace(lbxUSB->Items->Text, "[E]", "[O]",
		TReplaceFlags() << rfReplaceAll);
	lbxUSB->Items->Text = strTemp;
	// 清空未長出裝置
	for (int i = 0; i < lbxUSB->Count; i++) {
		if (lbxUSB->Items->Strings[i].Pos("[X]")) {
			lbxUSB->Items->Delete(i);
			i -= 1;
		}
	}
	TListUSB_Dev[dwModeIndex - 1]->Text = lbxUSB->Items->Text;
	//
	bLoadDevData[dwModeIndex] = true;
}
// ---------------------------------------------------------------------------

void __fastcall TfrmMain::plMode1Click(TObject *Sender) {
	TPanel* plMode = (TPanel*)Sender;
	plMode->Color = clYellow;
	dwModeIndex = plMode->Tag;

	for (DWORD i = 1; i <= 4; i++) {
		if (i == dwModeIndex)
			continue;
		TPanel* plOtherMode = (TPanel*)frmMain->FindComponent
			("plMode" + AnsiString(i));
		if (plOtherMode->Color == clYellow)
			plOtherMode->Color = clMoneyGreen;
	}
	AnsiString strTemp;
	strTemp = plScreenInfo->Caption;
	ClearListBox();

	plScreenInfo->Hint = TListScreen_Dev[dwModeIndex - 1]->Text;
	plScreenInfo->Caption = strTemp;
	strTemp = StringReplace(TListHUB_Dev[dwModeIndex - 1]->Text, "[O]", "[X]",
		TReplaceFlags() << rfReplaceAll);
	lbxHUB->Items->Text = strTemp;

	strTemp = StringReplace(TListUSB_Dev[dwModeIndex - 1]->Text, "[O]", "[X]",
		TReplaceFlags() << rfReplaceAll);
	lbxUSB->Items->Text = strTemp;
	USBConn->EnumHUB(false, lbxHUB, bLoadDevData[dwModeIndex]);
	USBConn->EnumUSB(false, lbxUSB, bLoadDevData[dwModeIndex]);
	USBConn->getMonitor(false, plScreenInfo, bLoadDevData[dwModeIndex]);
}

// ---------------------------------------------------------------------------
DWORD TfrmMain::CheckDevMode(int ModeIndex) {
	// ModeIndex : 0 -> Unknow Mode
	bool bPass;
	if (ModeIndex == 0) {
		for (int i = 0; i < 4; i++) {
			TPanel* plMode = (TPanel*)FindComponent
				("plMode" + AnsiString(i + 1));
			plMode1Click(plMode);
			Delay(250);
			frmMain->Refresh();
			bPass = true;
			// -----------------
			if (!USBConn->CheckListBoxData(TListHUB_Dev[i], lbxHUB, true))
				bPass = false;
			else
				ChangeMsgBarUI(false, 2, clGreen);
			// =========================
			if (!USBConn->CheckListBoxData(TListUSB_Dev[i], lbxUSB, true))
				bPass = false;
			else
				ChangeMsgBarUI(false, 3, clGreen);
			// =======================
			if (plScreenInfo->Caption != TListScreen_Dev[i]->Text.Trim())
				bPass = false;
			else if (plScreenInfo->Font->Color != clBlue)
				bPass = false;
			else
				ChangeMsgBarUI(false, 4, clGreen);
			// ============
			if (bPass)
				return i + 1;
			else {
				ChangeMsgBarUI(true, 0, clGreen);
				dwModeIndex = 0;
			}
		}
	}
	else {
		bPass = true;
		if (!USBConn->CheckListBoxData(TListHUB_Dev[ModeIndex - 1], lbxHUB, true))
			bPass = false;
		else
			ChangeMsgBarUI(false, 2, clGreen);
		if (!USBConn->CheckListBoxData(TListUSB_Dev[ModeIndex - 1], lbxUSB, true))
			bPass = false;
		else
			ChangeMsgBarUI(false, 3, clGreen);
		if (plScreenInfo->Caption != TListScreen_Dev[ModeIndex - 1]->Text.Trim()
			)
			bPass = false;
		else if (plScreenInfo->Font->Color != clBlue)
			bPass = false;
		else
			ChangeMsgBarUI(false, 4, clGreen);
		if (bPass)
			return ModeIndex;
	}
	return UNKNOW_DEV_MODE;
}

void TfrmMain::ClearListBox() {
	lbxHUB->Clear();
	lbxUSB->Clear();
	plScreenInfo->Caption = "";
}

void __fastcall TfrmMain::plStartClick(TObject *Sender) {
	if (plStart->Caption.Pos("Start"))
	{
		if(plNetworkConn->Font->Color == clRed)
		{
			MessageBoxA(Handle," (!) 程式未成功連線","WARNING", MB_ICONEXCLAMATION);
			return;
		}
		else if(bNetWorkServer)
		{
			strRecBuf = "";
			SendNetworkMsg("[AS]SERVER_TEST_START");
			Delay(500);
			if(!strRecBuf.Pos("[AS]CLIENT_TEST_START"))
			{
				MessageBoxA(Handle," (!) Client端無回應 ","WARNING", MB_ICONEXCLAMATION);
				return;
			}
        }
		ClearUI();
		strRecBuf = "";
		plResult->Color = clCream;
		plResult->Caption = "Waiting...";
		ChangeMsgBarUI(true, 0, clGray);
		plStart->Caption = "Stop";
		plLoadDevInfoTitle->Visible = false;
		dwStep = STEP_WAIT_DEV_PLUG_IN;
		DEBUG("[STEP_WAIT_DEV_PLUG_IN]");
		bLoadDevData[0] = false;
		ImgLogo->Enabled = false;
		plStartTitle->Visible = true;
	}
	else {
		StopTest(true);
	}
}

// ---------------------------------------------------------------------------
void TfrmMain::Delay(ULONG iMilliSeconds) {
	ULONG iStart;
	iStart = GetTickCount();
	while (GetTickCount() - iStart <= iMilliSeconds) {
		Sleep(50);
		RefreshTime();
		Application->ProcessMessages();
	}
}
DWORD TfrmMain::CheckNextMode(DWORD dwNowMode) {
	if (plMode1->Color == clGreen && plMode2->Color == clGreen &&
		plMode3->Color == clGreen && plMode4->Color == clGreen)
		return STEP_DEV_MODE_OVER;
	DWORD dwNextMode;
	dwNextMode = dwNowMode + 1 >= 5 ? CHECK_DEV_MODE1 : dwNowMode + 1;
	return dwNextMode;
}

DWORD TfrmMain::SwitchNextMode(DWORD dwMode) {
	if (plMode1->Color == clGreen && plMode1->Color == clGreen &&
		plMode3->Color == clGreen && plMode4->Color == clGreen)
		return STEP_DEV_MODE_OVER;
	DWORD dwNextMode;
	dwNextMode = dwMode + 1 >= 5 ? CHECK_DEV_MODE1 : dwMode + 1;
	return dwNextMode;
}
void TfrmMain::RefreshTime() {
	//
	AnsiString astrSS;
	if (dwTimeOut != 0) {
		if (dwTimeOut > GetTickCount()) {
			astrSS.printf("%.1f", float((dwTimeOut - GetTickCount())) / 1000);
			if (astrSS.ToDouble() < 0.2)
				plModeTime->Caption = "0.0(sec)";
			else
				plModeTime->Caption = astrSS + "(sec)";
		}
	}
	else
		plModeTime->Caption = "0.0(sec)";
}

void TfrmMain::ClearUI() {
	HWND hWnd = FindWindow(NULL,"WARNING");
	if(plErrorMsgWindows->Visible)
		plErrorMsgWindows->Visible = false;
	for (int i = 1; i <= 4; i++) {
		TPanel* plMode = (TPanel*)frmMain->FindComponent
			("plMode" + AnsiString(i));
		plMode->Color = clMoneyGreen;
	}
}

void TfrmMain::LoadINI() {
	TIniFile *ini;
	String fn = ChangeFileExt(Application->ExeName, ".ini");

	if (!FileExists(fn)) {
		return;
	}

	ini = new TIniFile(fn);
	AnsiString strTemp, strTemp2;
	DWORD dwCount = 0;
	//
	frmMain->Caption = ini->ReadString("APP","NAME","UCD0ModeView (ActionStar)");
	astrPDVersion = ini->ReadString("APP","PD_VERSION","");
	dwModeInit		 = ini->ReadInteger("APP", "INIT_MODE", 0);
	bNetWorkServer       = ini->ReadString("APP","NETWORK_CONNECT","SERVER") == "SERVER" ? true : false;
	plStartTitle->Visible = bNetWorkServer;
	//
	dwCount = ini->ReadInteger("UNBAN_VPID", "USBCOUNT", 0);
	for (DWORD j = 0; j < dwCount; j++) {
		strTemp2 = "USB" + AnsiString(j + 1);
		USBConn->AddUnBanVPID(ini->ReadString("UNBAN_VPID",strTemp2,""));
	}
	//
	dwCount = ini->ReadInteger("DEVICE_USB", "USBCOUNT", 0);
	for (DWORD i = 0; i < dwCount; i++) {
		strTemp2 = "USB" + AnsiString(i + 1);
		TListCheck_DevUSB->Add(ini->ReadString("DEVICE_USB", strTemp2, ""));
	}
	//
	for (int i = 0; i < 5; i++) {
		strTemp = "MODE" + AnsiString(i);
		dwSwitchModeTimeOut[i] = ini->ReadInteger(strTemp, "SWITCH_TIMEOUT",
			5000);
		dwModeTimeOut[i] = ini->ReadInteger(strTemp, "WAIT_DEV_TIMEOUT", 5000);
		bKeyBoardSwitch[i]  = ini->ReadBool(strTemp,"KEYBOARD_SWITCH",false);
		dwKeyBoardSwitchTimeOut[i] = ini->ReadInteger(strTemp, "KEYBOARD_SWITCH_TIMEOUT", 5000);
		if (i > 0) {
			TListScreen_Dev[i - 1]->Text = ini->ReadString(strTemp, "SCREEN",
				"");
			TPanel* plMode = (TPanel*)frmMain->FindComponent
				("plMode" + AnsiString(i));
			strTemp2 = ini->ReadString(strTemp, "MODE_NAME", "");
			if (strTemp2 != "")
				plMode->Caption = strTemp2;
			//
			int iSize = ini->ReadInteger(strTemp, "MODE_NAME_SIZE", 0);
			if (iSize != 0)
				plMode->Font->Size = iSize;
			//
			dwCount = ini->ReadInteger(strTemp, "HUBCOUNT", 0);
			for (DWORD j = 0; j < dwCount; j++) {
				strTemp2 = "HUB" + AnsiString(j + 1);
				TListHUB_Dev[i - 1]->Add(ini->ReadString(strTemp, strTemp2,
						""));
				USBConn->setRegDriver(ini->ReadString(strTemp, strTemp2,""));
			}
			dwCount = ini->ReadInteger(strTemp, "USBCOUNT", 0);
			for (DWORD j = 0; j < dwCount; j++) {
				strTemp2 = "USB" + AnsiString(j + 1);
				TListUSB_Dev[i - 1]->Add(ini->ReadString(strTemp, strTemp2,
						""));
				USBConn->setRegDriver(ini->ReadString(strTemp, strTemp2,""));
			}
			//
			if (TListScreen_Dev[i - 1]->Text != "" || TListHUB_Dev[i - 1]
				->Text != "" || TListUSB_Dev[i - 1]->Text != "") {
				bLoadDevData[i] = true;
				plMode1Click(plMode);
			}
		}
	}
	delete ini;
	plMode1Click(plMode1);
}

void TfrmMain::SaveINI() {
	String fn = ChangeFileExt(Application->ExeName, ".ini");
	TIniFile *ini = new TIniFile(fn);
	//
	AnsiString strTemp, strTemp2;
	for (int i = 0; i < 5; i++) {
		strTemp = "MODE" + AnsiString(i);
		ini->WriteInteger(strTemp, "SWITCH_TIMEOUT", dwSwitchModeTimeOut[i]);
		ini->WriteInteger(strTemp, "WAIT_DEV_TIMEOUT", dwModeTimeOut[i]);
		if (i > 0) {
			ini->WriteString(strTemp, "SCREEN",
				TListScreen_Dev[i - 1]->Text.Trim());
			ini->WriteString(strTemp, "HUBCOUNT", TListHUB_Dev[i - 1]->Count);
			for (int j = 0; j < TListHUB_Dev[i - 1]->Count; j++) {
				strTemp2 = "HUB" + AnsiString(j + 1);
				ini->WriteString(strTemp, strTemp2,
					TListHUB_Dev[i - 1]->Strings[j]);
			}
			ini->WriteString(strTemp, "USBCOUNT", TListUSB_Dev[i - 1]->Count);
			for (int j = 0; j < TListUSB_Dev[i - 1]->Count; j++) {
				strTemp2 = "USB" + AnsiString(j + 1);
				ini->WriteString(strTemp, strTemp2,
					TListUSB_Dev[i - 1]->Strings[j]);
			}
		}
	}
	//
	delete ini;
}

void __fastcall TfrmMain::FormClose(TObject *Sender, TCloseAction &Action)
{
	StopTest(true);
	SaveINI();
}
// ---------------------------------------------------------------------------

void __fastcall TfrmMain::plScreenOKClick(TObject *Sender) {
	TPanel* plScreen = (TPanel*)Sender;
	dwScreenResult = plScreen->Name.Pos("OK")
		? RESULT_SCREEN_PASS : RESULT_SCREEN_FAIL;
}

void __fastcall TfrmMain::popLoadDevClick(TObject *Sender) {
	plLoadDevInfoTitle->Visible = true;
}
// ---------------------------------------------------------------------------

void __fastcall TfrmMain::popINIT_USBClick(TObject *Sender) {
	TimeServer->Enabled = false;
	Delay(300);
	USBConn->EnumHUB(true, lbxHUB, false);
	USBConn->EnumUSB(true, lbxUSB, false);
	USBConn->getMonitor(true, plScreenInfo, false);
	TimeServer->Enabled = true;
}
// ---------------------------------------------------------------------------

void __fastcall TfrmMain::plLoadDevInfoMouseUp(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y) {
	TPanel* pl = (TPanel*)frmMain->FindComponent
		(((TPanel*)Sender)->Name + "Title");
	pl->Left--;
	pl->Top--;
	pl->Width++;
	pl->Height++;
}
// ---------------------------------------------------------------------------

void __fastcall TfrmMain::plLoadDevInfoMouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y) {
	TPanel* pl = (TPanel*)frmMain->FindComponent
		(((TPanel*)Sender)->Name + "Title");
	pl->Left++;
	pl->Top++;
	pl->Width--;
	pl->Height--;
}
// ---------------------------------------------------------------------------
void TfrmMain::ChangeMsgBarUI(bool bAllClear, DWORD dwIndex, TColor uiColor) {
	if (bAllClear || dwIndex == 0)
	{
		for (int i = 1; i <= 5; i++)
		{
			TPanel* plStatusMsg = (TPanel*)frmMain->FindComponent
				("plStatusMsg" + AnsiString(i));
			TImage* ImgLight = (TImage*)frmMain->FindComponent
				("ImgLight" + AnsiString(i));
			if(bAllClear)
			{
				plStatusMsg->Font->Color = clGray;
				ImgLight->Picture->LoadFromFile("Image\\gray.png");
			}
			else
			{
				if(plStatusMsg->Font->Color == clGray)
				{
					plStatusMsg->Font->Color = clRed;
					ImgLight->Picture->LoadFromFile("Image\\Red.png");
					LOGFile->astrErrorMsg = "(!)"+plStatusMsg->Hint+"_FAIL";
					break;
                }
			}
		}
	}
	else
	{
		TPanel* plStatusMsg = (TPanel*)frmMain->FindComponent
			("plStatusMsg" + AnsiString(dwIndex));
		TImage* ImgLight = (TImage*)frmMain->FindComponent
			("ImgLight" + AnsiString(dwIndex));
		if (uiColor == clNavy)
		{
			ImgLight1->Picture->LoadFromFile("Image\\Green.png");
			plStatusMsg->Font->Color = clNavy;
		}
		else if (uiColor == clGreen) {
			if (dwIndex == 2) {
				if(plStatusMsg1->Font->Color != clNavy)
				{
					plStatusMsg1->Font->Color = clNavy;
					ImgLight1->Picture->LoadFromFile("Image\\Green.png");
				}
			}
			if(plStatusMsg->Font->Color != clNavy)
			{
				plStatusMsg->Font->Color = clNavy;
				ImgLight->Picture->LoadFromFile("Image\\Green.png");
			}
		}
		else if (uiColor == clRed) {
			plStatusMsg->Font->Color = clRed;
			ImgLight->Picture->LoadFromFile("Image\\Red.png");
		}
	}
	Delay(200);
	frmMain->Refresh();
}
void __fastcall TfrmMain::popListBoxClick(TObject *Sender)
{
	if(plMSG->Visible)
		plMSG->Visible = false;
	else
		plMSG->Visible = true;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::plResultDblClick(TObject *Sender)
{
	if(plResult->Height > 120)
	{
		plResult->Height = 100;
	}
	else
	{
		plResult->Height = 486;
    }
}
//---------------------------------------------------------------------------
void TfrmMain::ShowSettingForm()
{
	if(frmMsg==NULL)  frmMsg = new TfrmMsg(Application);

	frmMsg->edtSetWorkOrderNumber->Text = edtSetWorkOrderNumber->Caption;
	frmMsg->edtSetEmployeeID->Text = edtSetEmployeeID->Caption.Trim();
	if(!bNetWorkServer)
	{
		frmMsg->edtSetWorkOrderNumber->Enabled = false;
		frmMsg->edtSetEmployeeID->Enabled = false;
		frmMsg->lbTitle->Caption = "按下OK開啟程式";
	}


	if(frmMsg->ShowModal()== mrOk)
	{
		edtSetWorkOrderNumber->Caption  = frmMsg->edtSetWorkOrderNumber->Text;
		edtSetEmployeeID->Caption = frmMsg->edtSetEmployeeID->Text;
		//
		LOGFile->astrWorkOrderNumber = edtSetWorkOrderNumber->Caption;
		LOGFile->astrEmployeeID		 = edtSetEmployeeID->Caption;
		LOGFile->FindLogFile(ImgDisk);
		//
		USBConn->EnumHUB(true, lbxHUB, false);
		USBConn->EnumUSB(true, lbxUSB, false);
		USBConn->getMonitor(true, plScreenInfo, false);
		dwModeIndex = 1;
		plMSGBOX->Width = 497;
		ImgDisk->Visible = frmMsg->ImgDisk->Visible;
		//
		TimeServer->Enabled = true;
		TimerUDP->Enabled = true;
		//
		delete frmMsg;
		frmMsg = NULL;
	}
	else
	{
		delete frmMsg;
		frmMsg = NULL;
		Close();
	}
}
bool TfrmMain::CheckPDVersion()
{
	if(astrPDVersion == "")
		return true;
	astrNowPDVersion = "";
	astrNowPDVersion = USBConn->DosCommand("GL_FW_Version.exe -pd");
	if(astrNowPDVersion.Pos(astrPDVersion))
		return true;
	return false;
}
//---------------------------------------------------------------------------
void TfrmMain::SendNetworkMsg(AnsiString Text)
{
	DEBUG_NETWORD("[傳送訊息]"+Text);
	if(bNetWorkServer)
	{
		if(ServerSocket->Socket->ActiveConnections)
			ServerSocket->Socket->Connections[0]->SendText(Text);
	}
	else
		ClientSocket->Socket->SendText(Text);
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::ServerSocketClientRead(TObject *Sender, TCustomWinSocket *Socket)

{
	AnsiString RecBuf = Socket->ReceiveText();
	DEBUG_NETWORD("[收到訊息]"+RecBuf);
	strRecBuf = RecBuf;
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::ClientSocketRead(TObject *Sender, TCustomWinSocket *Socket)

{
	AnsiString RecBuf = Socket->ReceiveText();
	DEBUG_NETWORD("[收到訊息]"+RecBuf);
	strRecBuf = RecBuf;
	if(RecBuf.Pos("[AS]Successfully Connected"))
	{
		plNetworkConn->Font->Color = clBlue;
		plNetworkConn->Caption = "程式連線中...";
	}
	else if(RecBuf.Pos("[AS]SERVER_TEST_START"))
	{
		plStart->Caption = "Start";
		plStartClick(NULL);
		SendNetworkMsg("[AS]CLIENT_TEST_START");
	}
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::ClientSocketConnect(TObject *Sender, TCustomWinSocket *Socket)
{
	plNetworkConn->Font->Color = clBlue;
	plNetworkConn->Caption = "程式連線中...";
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::ClientSocketDisconnect(TObject *Sender, TCustomWinSocket *Socket)
{
	plNetworkConn->Font->Color = clRed;
	DEBUG_NETWORD("[Server端中斷連線]");
	plNetworkConn->Caption = "與Server連線異常 請檢查線路 !";
	StopTest(false);
	TimerUDP->Enabled = true;
}
void __fastcall TfrmMain::ServerSocketClientConnect(TObject *Sender, TCustomWinSocket *Socket)

{
	plNetworkConn->Caption = "程式連線中...";
	plNetworkConn->Font->Color = clBlue;
	ServerSocket->Socket->Connections[0]->SendText("[AS]Successfully Connected");
    TimerUDP->Enabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::ClientSocketError(TObject *Sender, TCustomWinSocket *Socket,
		  TErrorEvent ErrorEvent, int &ErrorCode)
{
	ErrorCode = 0;
	DEBUG_NETWORD("[Server端連線異常]");
	plNetworkConn->Font->Color = clRed;
	plNetworkConn->Caption = "與Server連線異常 請檢查線路 !";
	StopTest(false);
	TimerUDP->Enabled = true;
}
void __fastcall TfrmMain::ServerSocketClientDisconnect(TObject *Sender, TCustomWinSocket *Socket)
{
	DEBUG_NETWORD("[Client端中斷連線]");
	plNetworkConn->Font->Color = clRed;
	plNetworkConn->Caption = "Client端中斷連線 請重新嘗試連線";
	StopTest(false);
	TimerUDP->Enabled = true;
}
void TfrmMain::getServerIP() // 抓網卡IP
{
	AnsiString Server_IP = "";
	int IpCount = 0;
	ULONG ulLen1 = 8192;
	BYTE *pbBuf1 = new BYTE[ulLen1];
	IP_ADAPTER_INFO *pipai = NULL;
	IP_ADDR_STRING *pipas = NULL;
	bool bHave = false;

	if (ERROR_SUCCESS == GetAdaptersInfo((IP_ADAPTER_INFO*)pbBuf1, &ulLen1)) {
		pipai = (IP_ADAPTER_INFO*)pbBuf1;
		do {
			// 取得網卡名稱並儲存
			//(AnsiString)pipai->Description;

			// 取得網卡IP並儲存
			pipas = &(pipai->IpAddressList);
			while (pipas != NULL) {
				if (AnsiString(pipas->IpAddress.String).Pos("169.254")) {
					bHave = true;
					Server_IP = AnsiString(pipas->IpAddress.String);
					if(!ServerSocket->Active)
					{
						ServerSocket->Active = true;
						DEBUG_NETWORD("TCP-Server開啟中");
					}
				}
				pipas = pipas->Next;
			}

			pipai = pipai->Next;
			IpCount++;
		}
		while (pipai != NULL);
	}
	delete pbBuf1;
	if(bHave == false)
	{
		if(ServerSocket->Active != false)
		{
			ServerSocket->Active = false;
			DEBUG_NETWORD("TCP-Server關閉");
			plNetworkConn->Font->Color = clRed;
			plNetworkConn->Caption = "與Client連線異常 請檢查線路 !";
		}
	}
}
void __fastcall TfrmMain::TimerUDPTimer(TObject *Sender)
{
	TimerUDP->Enabled = false;
	if(bNetWorkServer)
		getServerIP();
	bool bPass = false;
	char buf[512];
	int nbytes; // the number of read
	int udp_addr_len = sizeof(struct sockaddr_in);
	WSAData wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);

	// 設置地址
	SOCKADDR_IN udp_addr;

	udp_addr.sin_addr.s_addr = INADDR_ANY;
	udp_addr.sin_family = AF_INET;
	udp_addr.sin_port = ntohs(9080);

	// 創建socket
	SOCKET sUDPConnect; //operating if a UDP connection was found
	sUDPConnect = socket(AF_INET, SOCK_DGRAM, NULL);

	unsigned long iMode=1;
	ioctlsocket(sUDPConnect,FIONBIO, (u_long FAR*) &iMode);
	//綁定
	if(bNetWorkServer)
	{
		char broadcast = 'a';
		if (setsockopt(sUDPConnect, SOL_SOCKET, SO_BROADCAST, &broadcast,sizeof(broadcast)) == -1)
		{
			//DEBUG("setsockopt(SO_BROADCAST)");
		}
		else
		{
			SOCKADDR_IN addr;
			// 清空,將資料設為 0
			memset((UINT8*)&udp_addr,0,sizeof(struct sockaddr_in));
			//memset((UINT8*)&udp_broadcast_fr,0,sizeof(struct sockaddr_in));

			//設定目標位址資訊
			udp_addr.sin_family = AF_INET;
			udp_addr.sin_port = ntohs(9080);
			udp_addr.sin_addr.s_addr =inet_addr("255.255.255.255");

			//設定己方位址資訊
			//udp_broadcast_fr.sin_family = AF_INET;

			//傳送封包
			if(ServerSocket->Active)
			{
				AnsiString strSS = "[AS]CONNECT,"+edtSetWorkOrderNumber->Caption+","+edtSetEmployeeID->Caption;
				sprintf(buf,strSS.c_str());
				sendto(sUDPConnect,buf,strlen(buf)+1, 0,
				(struct sockaddr*) & udp_addr,sizeof(struct sockaddr));
			}
		}
	}
	else
	{

		bind(sUDPConnect, (SOCKADDR*)&udp_addr, sizeof(udp_addr));
		Delay(1000);
		if ((nbytes = recvfrom(sUDPConnect, buf,512, 0, (struct sockaddr*)&udp_addr,&udp_addr_len )) <0)
		{
			bPass = false;
		}
		else
		{
			printf("Received a datagram: %s", buf);
			if(AnsiString(buf).Pos("[AS]CONNECT"))
			{
				AnsiString strSS = AnsiString(buf);
				strSS.Delete(1,strSS.Pos(","));
				edtSetWorkOrderNumber->Caption = strSS.SubString(1,strSS.Pos(",")-1);
				strSS.Delete(1,strSS.Pos(","));
				edtSetEmployeeID->Caption      = strSS;
				LOGFile->astrWorkOrderNumber = edtSetWorkOrderNumber->Caption;
				LOGFile->astrEmployeeID		 = edtSetEmployeeID->Caption;
				LOGFile->FindLogFile(ImgDisk);

				AnsiString IP = (AnsiString)inet_ntoa(udp_addr.sin_addr);
				closesocket(sUDPConnect);
				WSACleanup();
				ClientSocket->Address = IP;
				ClientSocket->Active = true;
				bPass = true;
			}
		}
	}
	if(bPass)
	{
		TimerUDP->Enabled = false;
	}
	else
	{
		TimerUDP->Enabled = true;
		closesocket(sUDPConnect);
		WSACleanup();
	}
}
void TfrmMain::StopTest(bool bSend)
{
	DEBUG("[STOP_TEST]");
	plStart->Caption = "Start";
	dwStep = STEP_REFRESH_USB_DEV;
	plMSGBOX->Visible = false;
	dwTimeOut = 0;
	ImgLogo->Enabled = true;
	strRecBuf = "";
	if(bSend)
		SendNetworkMsg("[AS]TEST_STOP");
	plStartTitle->Visible = bNetWorkServer;
}
void __fastcall TfrmMain::ServerSocketClientError(TObject *Sender, TCustomWinSocket *Socket,
		  TErrorEvent ErrorEvent, int &ErrorCode)
{
	ErrorCode = 0;
	StopTest(false);
}
//---------------------------------------------------------------------------





