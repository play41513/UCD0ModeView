//---------------------------------------------------------------------------
#include <Classes.hpp>
#ifndef UDP_ThreadH
#define UDP_ThreadH
#include <stdio.h>
#include <winsock.h>
#pragma comment(lib, "wsock32.lib")
//---------------------------------------------------------------------------
class hUDP_Thread : public TThread
{
protected:
	void __fastcall Execute(void);
public:
	__fastcall hUDP_Thread(bool CreateSuspended);
};
#endif
