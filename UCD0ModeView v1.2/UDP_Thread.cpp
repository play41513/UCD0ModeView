//---------------------------------------------------------------------------


#pragma hdrstop

#include "UDP_Thread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
__fastcall hUDP_Thread::hUDP_Thread(bool CreateSuspended)
: TThread(CreateSuspended)
{
}
//---------------------------------------------------------------------------
void __fastcall hUDP_Thread::Execute(void)
{
	timeval timeout;
	int nRet;
	char message[200];
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);


	char broadcast = 'a';
	int sUDP_Connect_sd=-1;
	struct sockaddr_in udp_broadcast_to;
	struct sockaddr_in udp_broadcast_fr;
	int client_addr_len = sizeof(struct sockaddr_in);
	//建立 socket
	sUDP_Connect_sd=socket(AF_INET,SOCK_DGRAM,0);
	if(sUDP_Connect_sd < 0)
	{
		//DEBUG("UDP Broadcast Failed!");
		exit(0);
	}
	//socket發送的數據具有廣播特性
	if (setsockopt(sUDP_Connect_sd, SOL_SOCKET, SO_BROADCAST, &broadcast,sizeof(broadcast)) == -1)
	{
		//DEBUG("setsockopt(SO_BROADCAST)");
		exit(1);
	}

	SOCKADDR_IN addr;
	// 清空,將資料設為 0
	memset((UINT8*)&udp_broadcast_to,0,sizeof(struct sockaddr_in));
	memset((UINT8*)&udp_broadcast_fr,0,sizeof(struct sockaddr_in));

	//設定目標位址資訊
	udp_broadcast_to.sin_family = AF_INET;
	udp_broadcast_to.sin_port = ntohs(9090);
	udp_broadcast_to.sin_addr.s_addr =inet_addr("255.255.255.255");

	//設定己方位址資訊
	udp_broadcast_fr.sin_family = AF_INET;

	//傳送封包
	char buf[512];
	sprintf(buf,"[AS]CONNECT");
	sendto(sUDP_Connect_sd,buf,strlen(buf)+1, 0,
		(struct sockaddr*) & udp_broadcast_to,sizeof(struct sockaddr));

	closesocket(sUDP_Connect_sd);
	WSACleanup();
}
//---------------------------------------------------------------------------
