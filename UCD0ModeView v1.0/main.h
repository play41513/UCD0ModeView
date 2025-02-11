//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <pngimage.hpp>
#include "inifiles.hpp"
#include <Menus.hpp>
#include "MSGBOX.h"
#include <ScktComp.hpp>
#include <ScktComp.hpp>

#include "UDP_Thread.h"
#include <Sockets.hpp>

#define STEP_CHECK_MODE				99
#define STEP_SWITCH_MODE_TIMEOUT	1
#define STEP_CHECK_SCREEN			2
#define STEP_WAITING_DUT_READY		3
#define STEP_WAITING_TEST_RESULT	4
#define STEP_REFRESH_USB_DEV		5
#define STEP_WAIT_DEV_PLUG_IN		6
#define STEP_SWITCH_MODE 			7
#define STEP_DEV_MODE_OVER			8
#define STEP_RESULT_FAIL			9
#define STEP_KEYBOARD_SWITCH		10

#define RESULT_SCREEN_PASS			1
#define RESULT_SCREEN_FAIL			2
#define RESULT_SCREEN_TIMEOUT		3
#define RESULT_WAIT_SCREEN			4


#define CHECK_DEV_MODE1			1
#define CHECK_DEV_MODE2			2
#define CHECK_DEV_MODE3			3
#define CHECK_DEV_MODE4			4
#define UNKNOW_DEV_MODE			5

#define DEBUG(String) moDebug->Lines->Add(String)
#define DEBUG_NETWORD(String) moDebug->Lines->Add(String)
#define ERROR_MSG(String) plErrorMsg->Caption = String; \
						  plErrorMsgTitle->Caption = "WARNING";\
						  plErrorMsgTitle->Color = clRed;\
						  plErrorMsg->Font->Size = 12;\
						  plErrorMsgWindows->Visible = true

#define WAIT_KEYBOARD_MSG(String) plErrorMsg->Caption = String; \
								  plErrorMsgTitle->Caption = "Massage";\
								  plErrorMsgTitle->Color = clTeal;\
								  plErrorMsg->Font->Size = 20;\
								  plErrorMsg->Font->Color = clBlue;\
								  plErrorMsgWindows->Visible = true
//---------------------------------------------------------------------------
class TfrmMain : public TForm
{
__published:	// IDE-managed Components
	TPanel *Panel1;
	TPanel *Panel2;
	TPanel *plMessageBox1;
	TPanel *Panel4;
	TPanel *Panel9;
	TPanel *plUSB;
	TPanel *Panel11;
	TPanel *Panel12;
	TPanel *Panel13;
	TPanel *Panel14;
	TListBox *lbxHUB;
	TTimer *TimeServer;
	TListBox *lbxUSB;
	TPanel *plScreenInfo;
	TPanel *Panel10;
	TImage *ImgLogo;
	TPanel *plLoadDevInfoTitle;
	TPanel *plLoadDevInfo;
	TPanel *plStartTitle;
	TPanel *plStart;
	TPanel *Panel5;
	TMemo *moDebug;
	TPanel *plTimeTitle;
	TPanel *plMSGBOX;
	TPanel *Panel8;
	TPanel *plScreenOKTitle;
	TPanel *plScreenOK;
	TPanel *plScreenFailTitle;
	TPanel *plScreenFail;
	TPanel *plModeTime;
	TPanel *plMode1Title;
	TPanel *plMode1;
	TPanel *plMode2Title;
	TPanel *plMode2;
	TPanel *plMode3Title;
	TPanel *plMode3;
	TPanel *plMode4Title;
	TPanel *plMode4;
	TPanel *Panel20;
	TPanel *Panel22;
	TPopupMenu *PopupMenu;
	TMenuItem *popLoadDev;
	TMenuItem *popINIT_USB;
	TPanel *plMSG;
	TPanel *Panel6;
	TPanel *Panel16;
	TPanel *Panel17;
	TPanel *Panel33;
	TPanel *Panel38;
	TPanel *Panel43;
	TPanel *plStatusMsg2;
	TPanel *Panel27;
	TImage *ImgLight1;
	TPanel *plStatusMsg1;
	TImage *ImgLight2;
	TImage *ImgLight3;
	TPanel *plStatusMsg3;
	TImage *ImgLight4;
	TPanel *plStatusMsg4;
	TImage *ImgLight5;
	TPanel *plStatusMsg5;
	TPanel *Panel15;
	TPanel *Panel19;
	TPanel *Panel25;
	TMenuItem *popListBox;
	TPanel *plResult;
	TPanel *Panel7;
	TPanel *Panel3;
	TPanel *Panel18;
	TPanel *edtSetEmployeeID;
	TPanel *edtSetWorkOrderNumber;
	TImage *ImgDisk;
	TClientSocket *ClientSocket;
	TServerSocket *ServerSocket;
	TTimer *TimerUDP;
	TPanel *plNetworkConn;
	TPanel *plErrorMsgWindows;
	TPanel *plErrorMsgTitle;
	TPanel *plErrorMsg;
	TPanel *Panel21;
	TTimer *Timer1;
	void __fastcall TimeServerTimer(TObject *Sender);
	void __fastcall lbxHUBDrawItem(TWinControl *Control, int Index, TRect &Rect, TOwnerDrawState State);
	void __fastcall plLoadDevInfoClick(TObject *Sender);
	void __fastcall plMode1Click(TObject *Sender);
	void __fastcall plStartClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall plScreenOKClick(TObject *Sender);
	void __fastcall popLoadDevClick(TObject *Sender);
	void __fastcall popINIT_USBClick(TObject *Sender);
	void __fastcall plLoadDevInfoMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall plLoadDevInfoMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall popListBoxClick(TObject *Sender);
	void __fastcall plResultDblClick(TObject *Sender);
	void __fastcall ServerSocketClientDisconnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ServerSocketClientRead(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ClientSocketConnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ClientSocketDisconnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ClientSocketError(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
          int &ErrorCode);
	void __fastcall ClientSocketRead(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall TimerUDPTimer(TObject *Sender);
	void __fastcall ServerSocketClientConnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ServerSocketClientError(TObject *Sender, TCustomWinSocket *Socket,
          TErrorEvent ErrorEvent, int &ErrorCode);


private:	// User declarations
	CUSBConn * USBConn;
	CLOGFile * LOGFile;

	bool    bModePass,bKeyBoardSwitch[5];
	DWORD	dwModeIndex,dwNextModeIndex,dwResult,dwModeInit;
	DWORD   dwSwitchModeTimeOut[5],dwModeTimeOut[5],dwKeyBoardSwitchTimeOut[5];
	DWORD	dwScreenResult;
	AnsiString astrPDVersion,astrNowPDVersion;

	void ClearListBox();
	void RefreshTime();
	void ClearUI();
	void LoadINI();
	void SaveINI();
	void ShowSettingForm();
	void Delay(ULONG iMilliSeconds);
	void ChangeMsgBarUI(bool bAllClear,DWORD dwIndex,TColor uiColor);
	void SendNetworkMsg(AnsiString Text);
	void getServerIP();
	void StopTest(bool bSend);

	DWORD CheckNextMode(DWORD dwNowMode);
	DWORD SwitchNextMode(DWORD dwMode);
	DWORD CheckDevMode(int ModeIndex);

	bool CheckPDVersion();

public:		// User declarations
	__fastcall TfrmMain(TComponent* Owner);
	TStringList *TListCheck_DevUSB;
	TStringList *TListHUB_Dev[4];
	TStringList *TListUSB_Dev[4];
	TStringList *TListScreen_Dev[4];
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
