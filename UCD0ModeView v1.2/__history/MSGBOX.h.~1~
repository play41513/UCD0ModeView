//---------------------------------------------------------------------------

#ifndef MSGBOXH
#define MSGBOXH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
#include <Keyboard.hpp>
#include <tchar.h>
//---------------------------------------------------------------------------
class TfrmMsg : public TForm
{
__published:	// IDE-managed Components
	TPanel *plWOInfo;
	TPanel *Panel39;
	TEdit *edtSetWorkOrderNumber;
	TPanel *Panel5;
	TEdit *edtSetEmployeeID;
	TBitBtn *btnID_ok;
	TTouchKeyboard *TouchKeyboard;
	TPanel *plSwitch;
	TLabel *Label1;
	TPanel *Panel4;
	TPanel *Panel7;
	TPanel *ckbWIP;
	TPanel *Panel8;
	TPanel *ckbFGI;
	TPanel *Panel10;
	TPanel *Panel40;
	TPanel *Panel41;
	void __fastcall btnID_okClick(TObject *Sender);
	void __fastcall plSwitchClick(TObject *Sender);
	void __fastcall edtSetEmployeeIDKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall edtSetWorkOrderNumberKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall edtSetEmployeeIDExit(TObject *Sender);
	void __fastcall edtSetWorkOrderNumberExit(TObject *Sender);






private:	// User declarations
public:		// User declarations
	__fastcall TfrmMsg(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMsg *frmMsg;
//---------------------------------------------------------------------------
#endif
