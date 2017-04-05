//Charlie Ang
//CSC 3350 Spring 2016
//June 1, 2016
//Lab 6 CLIENT
//This program takes in a command line port number and connects with a server.
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

int main(int argc, char *argv[]) //argv[1] is port number from server //arg[2] is server name 
{
	if (argc == 1)
	{
		printf("Usage: client<port no.> [<server_host>]\n");
		printf("(Must start server first to get port no.)");
		exit(1);
	}
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

	//Client must know or obtain serverName and serverPortNumber...from command line 

	//Determine server-host network address table for serverName from command line 
	int errorNumber;
	struct hostent *host;
	if (argc == 3) //if there is two command line...serverPortNumber and serverName 
	{
		printf("Server host: %s\n", argv[2]);	//print server host name 
		host = gethostbyname(argv[2]);	//second command line 
		if (host == NULL)
		{
			printf("Invalid server name: %s", argv[2]);
			exit(1);
		}
	}
	else //serverHostName not specified...so assumes local machine is running SERVER app
	{
		char serverHostName[HOSTNAMELEN]; //stores serverHostName in this string 
		int result2 = gethostname(serverHostName, HOSTNAMELEN);
		host = gethostbyname(serverHostName);
		printf("Server host: %s\n", serverHostName); //print server host name
		if (result2 != 0)
		{
			errorNumber = GetLastError();	//get error number if result returns non-zero 
			printf("Invalid server name: %d", errorNumber);
			exit(1);
		}
	}



	//Create a SOCKADDR_IN to reference the server
	SOCKADDR_IN theSrvr;
	ZeroMemory(&theSrvr, sizeof(SOCKADDR_IN));
	theSrvr.sin_family = AF_INET;
	int portnum = atoi(argv[1]);
	theSrvr.sin_port = htons((u_short) portnum);	//serverPortNumber is first commandline arg
	CopyMemory(&theSrvr.sin_addr, host->h_addr_list[0], host->h_length);

	//Print serverPortNumber
	printf("Server port number: %d\n", portnum);

	//Create a client socket
	SOCKET ConnSkt = socket(AF_INET, SOCK_STREAM, 0);

	//Client connects to the server
	int result = connect(ConnSkt, (const SOCKADDR*) &theSrvr, sizeof(theSrvr));
	if (result != 0)
	{
		errorNumber = GetLastError();	//get error number if result returns non-zero 
		printf("Making connection request ... Connection error #%d\n", errorNumber);
		exit(1);	//quit program 
	}
	else
	{
		printf("Making connection request ... connected\n");
	}

	//Once a connection is accepted by the server, then client can send/recv data using ConnSkt
	int flags = 0;
	char buffer[MAXBUFFSIZE];

	//Configure socket to NB non-blocking mode 
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
	closesocket(ConnSkt); //close ConnSkt when finished 
	WSACleanup();	//release all socket-related resources 

	return 0;
}