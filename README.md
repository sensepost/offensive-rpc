To translate the IDL file:

> midl.exe /app_config RemotePrivilegeCall.idl

To compile the client/server:

> cl.exe RemotePrivilege_c.c client.cpp

> cl.exe RemotePrivilege_s.c server.cpp

Don't forget to switch hardcoded IP's in the server.cpp and client.cpp files :)

Happy hacking :) !
