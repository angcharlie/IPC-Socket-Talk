//Charlie Ang
//CSC 3350 Spring 2016
//June 1, 2016
//Lab 6 SERVER
//This program establishes a port number and waits for a client to link up.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <winbase.h>
#include <winsock.h>
#include <conio.h>

#define MAXBUFFSIZE 1000 //maximum buff size 
#define HOSTNAMELEN 128

int main(int argc, char *argv[])
{
	//Initialize socket package
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);	//VERSION 2.2
	if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		fprintf(stderr, "Process failed on WinSock startup\n");
		ExitProcess(0);
	}
	//******************************************************

	//getHostName of server
	char serverHostName[HOSTNAMELEN];
	int result = gethostname(serverHostName, HOSTNAMELEN);
	if (result != 0)
	{
		int errorNumber = GetLastError();
		printf("Invalid server name: %d", errorNumber);
		exit(1);
	}

	 //Get server-host network address table 
	struct hostent *host;
	host = gethostbyname(serverHostName);	//second command line 
	printf("Server host: %s\n", serverHostName);	//print server host name 
	if (host == NULL)
	{
		printf("Invalid server name: %s", serverHostName);
		exit(1);
	}

	//Server listener socket for incoming client requests 
	SOCKET skt = socket(AF_INET, SOCK_STREAM, 0);

	u_short port = 0;	//lets bind select a port #
	SOCKADDR_IN Listener;
	//Configure Listener for this server host and port #
	ZeroMemory(&Listener, sizeof(SOCKADDR_IN));
	Listener.sin_family = AF_INET;
	Listener.sin_port = htons((u_short)port);
	CopyMemory(&Listener.sin_addr, host->h_addr_list[0], host->h_length);


	//Bind listener socket to server address and specific port
	int result2 = bind(skt, (const SOCKADDR *)&Listener, sizeof(Listener));
	if (result2 != 0)
	{
		int errorNumber2 = GetLastError();
		printf("Could not bind. Error number: %d", errorNumber2);
		exit(1);
	}


	//What port is being used
	int len = sizeof(Listener);
	getsockname(skt, (SOCKADDR *)&Listener, &len);
	u_short actual_port = ntohs(Listener.sin_port);
	printf("Server port number: %u\n", actual_port);

	//Enable the listener incoming-event mechanism on listener socket
	listen(skt, SOMAXCONN);

	SOCKADDR_IN Client;
	SOCKET ConnSkt;
	//Process clients loop
	while (TRUE)
	{
		int ClientLen = sizeof(SOCKADDR_IN);

		printf("Listening ... ");
		ConnSkt = accept(skt, (SOCKADDR *)&Client, &ClientLen);
		if (!ConnSkt == NULL)
		{
			printf("accepted connection request\n");
			break;
		}
	}

	////Once a connection is accepted by the client, then server can send/recv data using ConnSkt
	int flags = 0;
	char buffer[MAXBUFFSIZE];

	////Configure socket to NB non-blocking mode 
	unsigned long on = 1;
	ioctlsocket(ConnSkt, FIONBIO, &on);

	printf(">");
	//Polling pattern 
	while (TRUE) //userInput != "exit\n"
	{
		if (_kbhit())	//non-blocking input 
		{
			gets_s(buffer, MAXBUFFSIZE);		//input entire line 
			send(ConnSkt, buffer, strlen(buffer) + 1, flags);
			printf(">");
			if (_stricmp(buffer, "exit") == 0)
			{
				break;
			}
		}

		int nRead = recv(ConnSkt, buffer, MAXBUFFSIZE, flags);
		if (nRead > 0)	//nRead is length of received info
		{
			buffer[nRead - 1] = '\0';
			printf("\n<");
			printf("%s", buffer); //only read up until 
			printf("\n>");
			if (_stricmp(buffer, "exit") == 0)
			{
				break;
			}
		}
		Sleep(500);
	}

	closesocket(ConnSkt);	//close connection socket 
	closesocket(skt);	//close listener socket 
	WSACleanup();	//release all socket-related resources 

	return 0;
}