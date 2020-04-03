#include <stdio.h>
#include "socketserver.h"
void clientStatus(char* ip, int port, int status)
{
	printf("client:%s:%d: status: %d\n", ip, port, status);
}
void clientMsg(char* ip, int port, char* buffer, int len)
{
	printf("client:%s:%d recv %d bytes [%s]\n", ip, port, len, buffer);
}
int main(void)
{
	register_clientStatus(clientStatus);
	register_clientMsg(clientMsg);
	Server_init("0.0.0.0", 12345);
	return 0;
}
