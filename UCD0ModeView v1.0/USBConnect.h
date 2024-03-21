//---------------------------------------------------------------------------

#ifndef USBConnectH
#define USBConnectH

#include <windows.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <Registry.hpp>

#include <vcl.h>
//---------------------------------------------------------------------------
class CUSBConn
{
	private:
		void RefreshListBox(TListBox* lbxUSB,TStringList* strList);
		bool CheckMouKbd(AnsiString strVPID);

	public:
		TStringList *TListInitScreen_Dev , *TListScreen_Dev;
		TStringList *TListInitHUB_Dev,*TListHUB_Dev;
		TStringList *TListInitUSB_Dev,*TListUSB_Dev;
		TStringList *TListUnBan_USB_Dev;

		CUSBConn::CUSBConn(void);
		AnsiString DosCommand(AnsiString sCmdline);
		void EnumHUB(bool bInit,TListBox* lbxUSB,bool bLoadDevData);
		void EnumUSB(bool bInit,TListBox* lbxUSB,bool bLoadDevData);
		void getMonitor(bool bInit,TPanel* plScreen,bool bLoadDevData);
		void AddUnBanVPID(AnsiString strVPID);
		void ClearStringList();
		bool CheckListBoxData(TStringList* strList, TListBox* lbxData,bool bAllTheSame);
		void setRegDriver(AnsiString DevVPID);

};
#endif
