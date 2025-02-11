//---------------------------------------------------------------------------


#pragma hdrstop

#include "USBConnect.h"
#include "Iphlpapi.h"
//---------------------------------------------------------------------------
#pragma comment(lib,"Iphlpapi.lib")
#pragma package(smart_init)
const char GUID_USB_HUB[] = "{F18A0E88-C30C-11D0-8815-00A0C906BED8}";
const char GUID_USB_DEVICE[] = "{A5DCBF10-6530-11D2-901F-00C04FB951ED}";

CUSBConn::CUSBConn(void)
{
	TListInitUSB_Dev = new TStringList;
	TListUSB_Dev     = new TStringList;
	TListInitHUB_Dev = new TStringList;
	TListHUB_Dev     = new TStringList;
	TListInitScreen_Dev  = new TStringList;
	TListScreen_Dev      = new TStringList;
	TListUnBan_USB_Dev = new TStringList;
}
void CUSBConn::AddUnBanVPID(AnsiString strVPID)
{
	if(strVPID != "")
		TListUnBan_USB_Dev->Add(strVPID);
}
void CUSBConn::EnumHUB(bool bInit,TListBox* lbxUSB,bool bLoadDevData)
{
	if(bInit)
		TListInitUSB_Dev->Clear();
	TListHUB_Dev->Clear();
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD i,j;
	AnsiString SS,USBPath;
	PSP_DEVICE_INTERFACE_DETAIL_DATA   pDetail   =NULL;
	GUID GUID_HUB =StringToGUID(GUID_USB_HUB);
	//--------------------------------------------------------------------------
	//   獲取設備資訊
	hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_HUB,
	0,   //   Enumerator
	0,
	DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );
	if   (hDevInfo   ==   INVALID_HANDLE_VALUE){
		//DEBUG("ERROR - SetupDiGetClassDevs()"); //   查詢資訊失敗
	}
	else{
	//--------------------------------------------------------------------------
		SP_DEVICE_INTERFACE_DATA   ifdata;
		DeviceInfoData.cbSize   =   sizeof(SP_DEVINFO_DATA);
		for (i=0;SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData);i++)	//   枚舉每個USB設備
		{
			ifdata.cbSize   =   sizeof(ifdata);
			if (SetupDiEnumDeviceInterfaces(								//   枚舉符合該GUID的設備介面
			hDevInfo,           //   設備資訊集控制碼
			NULL,                         //   不需額外的設備描述
			(LPGUID)&GUID_HUB,//GUID_CLASS_USB_DEVICE,                     //   GUID
			(ULONG)i,       //   設備資訊集�堛熙]備序號
			&ifdata))                 //   設備介面資訊
			{
				ULONG predictedLength   =   0;
				ULONG requiredLength   =   0;
				//   取得該設備介面的細節(設備路徑)
				SetupDiGetInterfaceDeviceDetail(hDevInfo,         //   設備資訊集控制碼
					&ifdata,          //   設備介面資訊
					NULL,             //   設備介面細節(設備路徑)
					0,         	      //   輸出緩衝區大小
					&requiredLength,  //   不需計算輸出緩衝區大小(直接用設定值)
					NULL);            //   不需額外的設備描述
				//   取得該設備介面的細節(設備路徑)
				predictedLength=requiredLength;
				pDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)::GlobalAlloc(LMEM_ZEROINIT,   predictedLength);
				pDetail->cbSize   =   sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

				if(SetupDiGetInterfaceDeviceDetail(hDevInfo,         //   設備資訊集控制碼
					&ifdata,             //   設備介面資訊
					pDetail,             //   設備介面細節(設備路徑)
					predictedLength,     //   輸出緩衝區大小
					&requiredLength,     //   不需計算輸出緩衝區大小(直接用設定值)
					NULL))               //   不需額外的設備描述
				{
					//
					try
					{
						char   ch[512];
						for(j=0;j<predictedLength;j++)
						{
						ch[j]=*(pDetail->DevicePath+8+j);
						}
						SS=ch;
						SS = SS.SubString(1,SS.Pos("{")-2);
						if(SS.Pos("vid"))
						{
							SS = SS.UpperCase();
							if(bInit){
								TListInitUSB_Dev->Add(SS);
							}
							else if(!TListInitUSB_Dev->Text.Pos(SS))
							{
								SS = "[HUB]"+SS.SubString(1,SS.Pos("#")-1);
								if(!bLoadDevData)
									TListHUB_Dev->Add(SS+"[O]");
								else
									TListHUB_Dev->Add(SS);
							}
						}
					}
					catch(...)
					{
						//
					}
				}
				GlobalFree(pDetail);
			}
		}
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	if(bInit)
		return;
	AnsiString  strTemp;
	if(!bLoadDevData)
	{
		lbxUSB->Items->Text = TListHUB_Dev->Text;
	}
	else
		RefreshListBox(lbxUSB,TListHUB_Dev);
}
void CUSBConn::EnumUSB(bool bInit,TListBox* lbxUSB,bool bLoadDevData)
{
	if(bInit)
		TListInitHUB_Dev->Clear();
	TListUSB_Dev->Clear();
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD i,j;
	AnsiString SS,USBPath;
	PSP_DEVICE_INTERFACE_DETAIL_DATA   pDetail   =NULL;
	GUID GUID_USB =StringToGUID(GUID_USB_DEVICE);
	//--------------------------------------------------------------------------
	//   獲取設備資訊
	hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_USB,
	0,   //   Enumerator
	0,
	DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );
	if   (hDevInfo   ==   INVALID_HANDLE_VALUE){
		//DEBUG("ERROR - SetupDiGetClassDevs()"); //   查詢資訊失敗
	}
	else{
	//--------------------------------------------------------------------------
		SP_DEVICE_INTERFACE_DATA   ifdata;
		DeviceInfoData.cbSize   =   sizeof(SP_DEVINFO_DATA);
		for (i=0;SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData);i++)	//   枚舉每個USB設備
		{
			ifdata.cbSize   =   sizeof(ifdata);
			if (SetupDiEnumDeviceInterfaces(								//   枚舉符合該GUID的設備介面
			hDevInfo,           //   設備資訊集控制碼
			NULL,                         //   不需額外的設備描述
			(LPGUID)&GUID_USB,//GUID_CLASS_USB_DEVICE,                     //   GUID
			(ULONG)i,       //   設備資訊集�堛熙]備序號
			&ifdata))                 //   設備介面資訊
			{
				ULONG predictedLength   =   0;
				ULONG requiredLength   =   0;
				//   取得該設備介面的細節(設備路徑)
				SetupDiGetInterfaceDeviceDetail(hDevInfo,         //   設備資訊集控制碼
					&ifdata,          //   設備介面資訊
					NULL,             //   設備介面細節(設備路徑)
					0,         	      //   輸出緩衝區大小
					&requiredLength,  //   不需計算輸出緩衝區大小(直接用設定值)
					NULL);            //   不需額外的設備描述
				//   取得該設備介面的細節(設備路徑)
				predictedLength=requiredLength;
				pDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)::GlobalAlloc(LMEM_ZEROINIT,   predictedLength);
				pDetail->cbSize   =   sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

				if(SetupDiGetInterfaceDeviceDetail(hDevInfo,         //   設備資訊集控制碼
					&ifdata,             //   設備介面資訊
					pDetail,             //   設備介面細節(設備路徑)
					predictedLength,     //   輸出緩衝區大小
					&requiredLength,     //   不需計算輸出緩衝區大小(直接用設定值)
					NULL))               //   不需額外的設備描述
				{
					try
					{
						char   ch[512];
						for(j=0;j<predictedLength;j++)
						{
						ch[j]=*(pDetail->DevicePath+8+j);
						}
						SS="\\\\?\\USB#"+AnsiString(ch);
						SS=ch;
						SS = SS.SubString(1,SS.Pos("{")-2);
						if(SS.Pos("vid"))
						{
							SS = SS.UpperCase();
							if(bInit)
							{
								TListInitUSB_Dev->Add(SS);
							}
							else if(!TListInitUSB_Dev->Text.Pos(SS))
							{
								if(!CheckMouKbd(SS.SubString(1,SS.Pos("#")-1)))
								{
									SS = "[USB]"+SS.SubString(1,SS.Pos("#")-1);
									if(!bLoadDevData)
										TListUSB_Dev->Add(SS+"[O]");
									else
										TListUSB_Dev->Add(SS);
								}
							}
						}
					}
					catch(...)
					{
						//DEBUG("列舉失敗");
					}
				}
				GlobalFree(pDetail);
			}
		}
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	if(bInit)
		return;
	/*
	TListUSB_Dev  : 僅記錄已儲存的裝置
	lbxUSB        : 目前顯示已儲存的裝置及上筆額外裝置
	*/
	AnsiString  strTemp;
	if(!bLoadDevData)
	{
		lbxUSB->Items->Text = TListUSB_Dev->Text;
	}
	else
		RefreshListBox(lbxUSB,TListUSB_Dev);
}
void CUSBConn::getMonitor(bool bInit,TPanel* plScreen,bool bLoadDevData)
{
	AnsiString DeviceID,DeviceDetail;
	TListScreen_Dev->Clear();
	DISPLAY_DEVICE dd;
	TDeviceModeA lpDevMode;
	dd.cb = sizeof(dd);
	DWORD dev = 0;
	int MONITORAMOUNT = 0;
	while (EnumDisplayDevices(0, dev, &dd, 0))
	{
		DISPLAY_DEVICE ddMon;
		ZeroMemory(&ddMon, sizeof(ddMon));
		ddMon.cb = sizeof(ddMon);
		DWORD devMon = 0;
		while (EnumDisplayDevices(dd.DeviceName, devMon, &ddMon, 0))
		{
			DeviceDetail = "";
			if (ddMon.StateFlags & DISPLAY_DEVICE_ACTIVE &&
			!(ddMon.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
			{
				DeviceID = ddMon.DeviceID;
				DeviceID = DeviceID.SubString(9,DeviceID.Pos("\\")-1);
				EnumDisplaySettings(dd.DeviceName,ENUM_CURRENT_SETTINGS,&lpDevMode);

				/*DeviceDetail = "["+DeviceID+"]   "
							+(AnsiString)lpDevMode.dmPelsWidth + "x"+ (AnsiString)lpDevMode.dmPelsHeight
							+"  -  "+lpDevMode.dmDisplayFrequency+"Hz";*/
				DeviceDetail = "["+DeviceID+"]   "
							+(AnsiString)lpDevMode.dmPelsWidth + "x"+ (AnsiString)lpDevMode.dmPelsHeight;
				MONITORAMOUNT++;
				if(bInit)
				{
					TListInitScreen_Dev->Add(DeviceDetail);
				}
				else if(!TListInitScreen_Dev->Text.Pos(DeviceDetail))
				{
					TListScreen_Dev->Add(DeviceDetail);
					if(!bLoadDevData)
						plScreen->Hint    = DeviceDetail;
					plScreen->Caption = DeviceDetail;
				}
			}
			devMon++;
			ZeroMemory(&ddMon, sizeof(ddMon));
			ddMon.cb = sizeof(ddMon);
		}
		ZeroMemory(&dd, sizeof(dd));
		dd.cb = sizeof(dd);
		dev++;
	}
	if(bLoadDevData)
	{
		if(TListScreen_Dev->Text.Pos(plScreen->Hint))
			plScreen->Font->Color = clBlue;
		else if(TListScreen_Dev->Count)
			plScreen->Font->Color = clRed;
		else
		{
			plScreen->Caption = plScreen->Hint;
			plScreen->Font->Color = clGrayText;
		}
		if(plScreen->Hint == "")
		{
			if(TListScreen_Dev->Count == 0)
			{
				plScreen->Caption = "";
				plScreen->Font->Color = clBlue;
			}
		}
	}
}
void CUSBConn::ClearStringList()
{
	TListHUB_Dev->Clear();
	TListUSB_Dev->Clear();
}
void CUSBConn::RefreshListBox(TListBox* lbxUSB,TStringList* strList)
{
	//lbxUSB  儲存的裝置資訊
	//strList 目前所有新長出的裝置
	AnsiString strTemp;
	bool 	   bSame;
	for(int i = 0 ; i < lbxUSB->Count ; i++)
	{
		bSame   = false;
		strTemp = lbxUSB->Items->Strings[i];
		for(int j = 0 ; j < strList->Count ; j++)
		{
			if(strTemp.Pos(strList->Strings[j]))
			{
				bSame = true;
				if(!strTemp.Pos("[E]"))
				{
					if(strTemp.Pos("[X]"))
					{
						strTemp = strTemp.SubString(1,strTemp.Length()-3);
						strTemp = strTemp+"[O]";
						lbxUSB->Items->Strings[i] = strTemp;
						strList->Delete(j);
					}
					else
						strList->Delete(j);
				}
				else
				{
                 	strList->Delete(j);
                }
			}
		}
		//
		if(!bSame)
		{//找不到裝置
			if(strTemp.Pos("[E]"))
			{
				lbxUSB->Items->Delete(i);
				i -= 1;
			}
			else
			{
				if(!strTemp.Pos("[X]"))
				{
					strTemp = strTemp.SubString(1,strTemp.Length()-3);
					lbxUSB->Items->Strings[i] = strTemp+"[X]";
				}
			}
		}
	}
	for(int i = 0 ; i < strList->Count ; i++)
	{
		strTemp = strList->Strings[i];
		lbxUSB->Items->Add(strTemp+"[E]");
    }
}
AnsiString CUSBConn::DosCommand(AnsiString sCmdline)
{
	PROCESS_INFORMATION proc = {0}; //關於進程資訊的一個結構
	long ret;
	bool sPipe;
	STARTUPINFOA start = {0};
	SECURITY_ATTRIBUTES sa = {0};
	HANDLE hReadPipe ;
	HANDLE hWritePipe;
	AnsiString sOutput;
	AnsiString sBuffer;
	unsigned long lngBytesRead;
	char cBuffer[256];
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor=0;
	sa.bInheritHandle = TRUE;
	sPipe=::CreatePipe(&hReadPipe, &hWritePipe,&sa, 0); //創建管道
	if (!sPipe)
	{
		sOutput="CreatePipe failed. Error: "+AnsiString(GetLastError());
		//memoMsg->Lines->Add("CreatePipe failed. Error: "+AnsiString(GetLastError()));
		return sOutput;
	}
	start.cb = sizeof(STARTUPINFOA);
	start.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	start.hStdOutput = hWritePipe;
	start.hStdError = hWritePipe;
	start.wShowWindow = SW_HIDE;
	sBuffer = sCmdline;
	ret =::CreateProcessA(0, sBuffer.c_str(), &sa, &sa, TRUE, NORMAL_PRIORITY_CLASS, 0, 0, &start, &proc);
	if (ret == 0)
	{
		sOutput="Bad command or filename";
		//memoMsg->Lines->Add("Bad command or filename");
		return sOutput;
	}
	::CloseHandle(hWritePipe);
	DWORD dw = WaitForSingleObject(proc.hProcess, 60000);
	if(dw == WAIT_TIMEOUT)
	{
		::CloseHandle(proc.hProcess);
		::CloseHandle(proc.hThread);
		::CloseHandle(hReadPipe);
		return "";
	}
	do
	{
	memset(cBuffer,'\0',sizeof(cBuffer));
	ret = ::ReadFile(hReadPipe, &cBuffer, 255, &lngBytesRead, 0);
	//替換字串
	for(unsigned long i=0; i< lngBytesRead; i++){
		if(cBuffer[i] == '\0'){
			cBuffer[i] = ' ';
		}else if(cBuffer[i] == '\n'){
			cBuffer[i] = ' ';
		}
	}
	sBuffer=StrPas(cBuffer);
	sOutput = sOutput+sBuffer;
	Application->ProcessMessages();

	} while (ret != 0 );
	::CloseHandle(proc.hProcess);
	::CloseHandle(proc.hThread);
	::CloseHandle(hReadPipe);
	return sOutput;
}
bool CUSBConn::CheckListBoxData(TStringList* strList, TListBox* lbxData,
	bool bAllTheSame) {
	bool bSame = false;
	if (strList->Count != lbxData->Count && bAllTheSame)
		return false;
	for (int i = 0; i < strList->Count; i++) {
		bSame = false;
		for (int j = 0; j < lbxData->Count; j++) {
			if (strList->Strings[i] == lbxData->Items->Strings[j]) {
				bSame = true;
				break;
			}
			if (!bAllTheSame) { // [E]判斷也為true
				if (strList->Strings[i] == lbxData->Items->Strings[j].SubString
					(1, lbxData->Items->Strings[j].Length() - 3)) {
					bSame = true;
					break;
				}
			}
		}
		if (bSame == false)
			return false;
	}
	return true;
}
bool CUSBConn::CheckMouKbd(AnsiString strVPID)
{
	if(TListUnBan_USB_Dev->Text.Pos(strVPID))
		return false;
	bool bHave = false;
	AnsiString temppath,SS;
	DWORD dwCount = 0;
	TRegistry *reg = new TRegistry();
	TStringList* itemSet = new TStringList();
	reg->RootKey = HKEY_LOCAL_MACHINE;
	//
	temppath = "SYSTEM\\CurrentControlSet\\Services\\mouhid\\Enum";
	reg->OpenKey(temppath.c_str(), false);
	reg->GetValueNames(itemSet);
	for(int i=0;i<itemSet->Count;i++)
	{
		if(itemSet->Strings[i] == "Count")
		{
			dwCount = ((AnsiString)reg->ReadInteger(itemSet->Strings[i])).ToInt();
			break;
		}
	}
	for(DWORD i = 0 ; i < dwCount ; i++)
	{
		SS = (AnsiString)reg->ReadString(AnsiString(i));
		if(SS.Pos(strVPID))
		{
			bHave = true;
			break;
		}
	}
	reg->CloseKey();
	//
	if(!bHave)
	{
		temppath = "SYSTEM\\CurrentControlSet\\Services\\kbdhid\\Enum";
		reg->OpenKey(temppath.c_str(), false);
		reg->GetValueNames(itemSet);
		for(int i=0;i<itemSet->Count;i++)
		{
			if(itemSet->Strings[i] == "Count")
			{
				dwCount = ((AnsiString)reg->ReadInteger(itemSet->Strings[i])).ToInt();
				break;
			}
		}
		for(DWORD i = 0 ; i < dwCount ; i++)
		{
			SS = (AnsiString)reg->ReadString(AnsiString(i));
			if(SS.Pos(strVPID))
			{
				bHave = true;
				break;
			}
		}
	}
	//
	reg->CloseKey();
	delete itemSet;
	delete reg;

	return bHave;
}
void CUSBConn::setRegDriver(AnsiString DevVPID)
{
	TRegistry *reg = new TRegistry();
	reg->RootKey = HKEY_LOCAL_MACHINE;
	AnsiString temppath = "SYSTEM\\CurrentControlSet\\Control\\usbflags";
	reg->OpenKey(temppath.c_str(), false);
	AnsiString tempIgnoreHWSerNum = "IgnoreHWSerNum"+DevVPID.SubString(10,4)+DevVPID.SubString(19,4);
	if(!reg->ValueExists (tempIgnoreHWSerNum))
	{
		BYTE buf[1];
		buf[0] = 0x01;
		reg->WriteBinaryData(tempIgnoreHWSerNum,buf,1);
	}
	reg->CloseKey();
	delete reg;
}
