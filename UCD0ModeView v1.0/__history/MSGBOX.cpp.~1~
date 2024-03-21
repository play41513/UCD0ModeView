//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "MSGBOX.h"


//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMsg *frmMsg;
//---------------------------------------------------------------------------
__fastcall TfrmMsg::TfrmMsg(TComponent* Owner)
	: TForm(Owner)
{

}
//---------------------------------------------------------------------------
void __fastcall TfrmMsg::btnID_okClick(TObject *Sender)
{
	if(edtSetWorkOrderNumber->Text=="")
		MessageBoxA(NULL,"請輸入完整資訊","WARNING", MB_ICONEXCLAMATION);
	else if(edtSetEmployeeID->Text=="")
		MessageBoxA(NULL,"請輸入完整資訊","WARNING", MB_ICONEXCLAMATION);
	else
		ModalResult = mrOk;
}
//---------------------------------------------------------------------------
void __fastcall TfrmMsg::plSwitchClick(TObject *Sender)
{
	if(plSwitch->Caption=="V")
	{
		plSwitch->Caption = "^";
		frmMsg->Height = 452;
	}
	else
	{
		plSwitch->Caption = "V";
		frmMsg->Height = 269;
    }
}
//---------------------------------------------------------------------------

void __fastcall TfrmMsg::edtSetEmployeeIDKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
	if(Key==13)
	{
		bool bPASS = true;
		TEdit* edt= edtSetEmployeeID;
		if(edt->Text.Length()== 6)
		{
		//
			TCHAR HexChar[15];

			edt->Text = edt->Text.UpperCase();// 把字串轉成全大寫
			_tcscpy_s(HexChar, 15, AnsiString(edt->Text).c_str());
			for(int i=0;i < edt->Text.Length();i++)
			{
				if(i==0)
				{
					if(HexChar[i] >= 0x30 && HexChar[i] <= 0x39)//數字
					{
						bPASS = false;
						break;
					}
				}
				else
				{
					if(HexChar[i] < 0x30 || HexChar[i] > 0x39)//非數字
					{
						bPASS = false;
						break;
					}
				}
			}
		//
		}
		else if(edt->Text.Length()== 7)
		{
			TCHAR HexChar[15];

			edt->Text = edt->Text.UpperCase();// 把字串轉成全大寫
			_tcscpy_s(HexChar, 15, AnsiString(edt->Text).c_str());
			for(int i=0;i < edt->Text.Length();i++)
			{
				if(i==0 || i==1)
				{
					if(HexChar[i] >= 0x30 && HexChar[i] <= 0x39)//數字
					{
						bPASS = false;
						break;
					}
				}
				else
				{
					if(HexChar[i] < 0x30 || HexChar[i] > 0x39)//非數字
					{
						bPASS = false;
						break;
					}
				}
			}
		}
		else
			bPASS = false;

		if(bPASS)
			btnID_ok->SetFocus();
		else
			edtSetEmployeeID->Text = "";
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmMsg::edtSetWorkOrderNumberKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift)
{
	if(Key==13)
		edtSetEmployeeID->SetFocus();
}
//---------------------------------------------------------------------------





void __fastcall TfrmMsg::FormShow(TObject *Sender)
{
	/*if(frmMsg->Tag)
	{
		frmMsg->plWOInfo->Enabled = false;
		frmMsg->plWOInfo->Height  = 0;
		frmMsg->plSwitch->Enabled = false;
		frmMsg->plSwitch->Height  = 0;
		frmMsg->TouchKeyboard->Top = 182;
		frmMsg->Height = 160;
	}*/
}
//---------------------------------------------------------------------------


void __fastcall TfrmMsg::edtSetEmployeeIDExit(TObject *Sender)
{
	edtSetEmployeeID->Text = edtSetEmployeeID->Text.UpperCase();

	bool bPASS = true;
	TEdit* edt= edtSetEmployeeID;
	if(edt->Text.Length()== 6)
	{
	//
		TCHAR HexChar[15];

		edt->Text = edt->Text.UpperCase();// 把字串轉成全大寫
		_tcscpy_s(HexChar, 15, AnsiString(edt->Text).c_str());
		for(int i=0;i < edt->Text.Length();i++)
		{
			if(i==0)
			{
				if(HexChar[i] >= 0x30 && HexChar[i] <= 0x39)//數字
				{
					bPASS = false;
					break;
				}
			}
			else
			{
				if(HexChar[i] < 0x30 || HexChar[i] > 0x39)//非數字
				{
					bPASS = false;
					break;
				}
			}
		}
	//
	}
	else if(edt->Text.Length()== 7)
	{
		TCHAR HexChar[15];

		edt->Text = edt->Text.UpperCase();// 把字串轉成全大寫
		_tcscpy_s(HexChar, 15, AnsiString(edt->Text).c_str());
		for(int i=0;i < edt->Text.Length();i++)
		{
			if(i==0 || i==1)
			{
				if(HexChar[i] >= 0x30 && HexChar[i] <= 0x39)//數字
				{
					bPASS = false;
					break;
				}
			}
			else
			{
				if(HexChar[i] < 0x30 || HexChar[i] > 0x39)//非數字
				{
					bPASS = false;
					break;
				}
			}
		}
	}
	else
		bPASS = false;
	if(!bPASS)
		edt->Text = "";
}
//---------------------------------------------------------------------------

void __fastcall TfrmMsg::edtSetWorkOrderNumberExit(TObject *Sender)
{
	edtSetWorkOrderNumber->Text = edtSetWorkOrderNumber->Text.UpperCase();
	AnsiString strTemp = "";
	if(edtSetWorkOrderNumber->Text.Length()== 15)
	{
		strTemp = edtSetWorkOrderNumber->Text.SubString(4,1);
		if(!strTemp.Pos("-"))
		{
			edtSetWorkOrderNumber->Text = "";
			edtSetWorkOrderNumber->SetFocus();
		}
		strTemp = edtSetWorkOrderNumber->Text.SubString(5,11);
		if(strTemp.Pos("-"))
		{
			edtSetWorkOrderNumber->Text = "";
			edtSetWorkOrderNumber->SetFocus();
		}
	}
	else
	{
		edtSetWorkOrderNumber->Text = "";
		edtSetWorkOrderNumber->SetFocus();
	}
}
//---------------------------------------------------------------------------

