//socket()
#include <sys/types.h>
#include <sys/socket.h>

//timeval
#include <sys/time.h>

//sockaddr_in
#include <netinet/in.h>
#include <arpa/inet.h>

//close
#include <unistd.h>

//errno
#include <errno.h>

//memset()
#include <string.h>

//printf()
#include <stdio.h>

//self include
#include "socketserver.h"

//客户端链接套接字
static int client_sockets[CLIENT_MAX];

//外部回调接口1
static callback_clientStatus clientstatus = NULL;

//外部回调接口2
static callback_clientMsg clientmsg = NULL;

//用于引入外部接口地址
void register_clientStatus(callback_clientStatus callback)
{
	clientstatus = callback;
}

//用于引入外部接口地址
void register_clientMsg(callback_clientMsg callback)
{
	clientmsg = callback;
}

//函数名：Server_init
//  功能：服务器初始化
//  入参：ip   服务器端ip，格式为xxx.xxx.xxx.xxx
//  入参：port 服务器端端口号，0-65535
//返回值：0初始化成功，>0 初始化失败，详见errno
int Server_init(char* ip, int port)
{
	int i = 0;
	int listen_socket = -1;
	int ret = -1;
	int opt = 1;
	int connectedsocket = -1;
	int max_socketid = -1;
	int server_run = 1;
	struct timeval timeout = {0, 0};
	
	fd_set listen_set;
	struct sockaddr_in serveraddr, clientaddr;
	
	socklen_t clientaddr_len;
	
	char recvbuffer[RECVBUFF_LEN];
	char sendbuffer[SENDBUFF_LEN];

	listen_socket = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
	if(listen_socket < 0)
	{
		return errno;
	}
	ret = setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));	
	if(NULL == ip)
		return EFAULT;
	memset((void*)&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	if(!inet_aton(ip, &serveraddr.sin_addr))
		return EFAULT;
	serveraddr.sin_port = htons(port);
	
	ret = bind(listen_socket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if(ret < 0)
	{
		return errno;
	}
	ret = listen(listen_socket, CLIENT_MAX);
	memset(client_sockets, -1, sizeof(client_sockets));
	
	clientaddr_len = sizeof(clientaddr);
	timeout.tv_sec = 300;
	timeout.tv_usec = 0;
	while(1 == server_run)
	{
		char ipstr[INET_ADDRSTRLEN];
		FD_ZERO(&listen_set);
		FD_SET(listen_socket, &listen_set);
		max_socketid = listen_socket;
		for(i = 0; i < CLIENT_MAX; i++)
		{
			if(client_sockets[i] > 0)
				FD_SET(client_sockets[i], &listen_set);
			if(client_sockets[i] > max_socketid)
				max_socketid = client_sockets[i];
		}
		ret = select(max_socketid + 1, &listen_set, NULL, NULL, &timeout);
		if(ret < 0)
		{
			server_run = 0;
			break;
		}
		else if(0 == ret)
		{
			server_run = 0;
			break;
		}
		if(FD_ISSET(listen_socket, &listen_set))
		{
			connectedsocket = accept(listen_socket, (struct sockaddr *)&clientaddr, &clientaddr_len);
			if((connectedsocket < 0) && (EWOULDBLOCK != errno))
			{	
				server_run = 0;
				break;
			}
			for(i = 0; i < CLIENT_MAX; i++)
			{
				if(client_sockets[i] < 0)
				{
					client_sockets[i] = connectedsocket;
					break;
				}
			}
			if(clientstatus)
				clientstatus((char *)inet_ntop(AF_INET, &clientaddr.sin_addr, ipstr, sizeof(ipstr)), \
			       		ntohs(clientaddr.sin_port), STATUS_NEW);
		}
		for(i = 0; i < CLIENT_MAX; i++)
		{
			connectedsocket = client_sockets[i];
			if(connectedsocket < 0)
				continue;
			if(FD_ISSET(connectedsocket, &listen_set))
			{
				if(clientstatus)
					clientstatus((char *)inet_ntop(AF_INET, &clientaddr.sin_addr, ipstr, sizeof(ipstr)), \
						ntohs(clientaddr.sin_port), STATUS_RECV);
				ret = recv(connectedsocket, recvbuffer, sizeof(recvbuffer), 0);
				if(0 == ret)
				{
					if(clientstatus)
					clientstatus((char *)inet_ntop(AF_INET, &clientaddr.sin_addr, ipstr, sizeof(ipstr)),
						ntohs(clientaddr.sin_port), STATUS_DISC);
					close(connectedsocket);
					client_sockets[i] = -1;
				}
				else if(ret < 0)
				{
					if(errno != EWOULDBLOCK)
					{	
						if(clientstatus)
							clientstatus((char *)inet_ntop(AF_INET, &clientaddr.sin_addr, ipstr, sizeof(ipstr)), \
								ntohs(clientaddr.sin_port), STATUS_DISC);
						close(connectedsocket);
						client_sockets[i] = -1;
					}
				
				}
				else
				{
					printf("server:%s:%d received from %s at PORT %d\n",
						ip, port, inet_ntop(AF_INET, &clientaddr.sin_addr, ipstr, sizeof(ipstr)),
						ntohs(clientaddr.sin_port));
					for(i = 0; i < ret; i++)
						sendbuffer[i] = recvbuffer[i] + 1;
					send(connectedsocket, sendbuffer, ret, 0);
					if(clientstatus)
						clientstatus((char *)inet_ntop(AF_INET, &clientaddr.sin_addr, ipstr, sizeof(ipstr)), \
							ntohs(clientaddr.sin_port), STATUS_SEND);
				}
			}
		}
	}
	for(i = 0; i < CLIENT_MAX; i++)
	{
		connectedsocket = client_sockets[i];
		if(connectedsocket < 0)
			continue;
		if(FD_ISSET(connectedsocket, &listen_set))
		{
			close(connectedsocket);
		}
	}
	return 0;
}
