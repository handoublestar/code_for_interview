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
static CLIENT_INFO client_info[CLIENT_MAX];

//外部回调接口1
static callback_clientStatus clientstatus = NULL;

//外部回调接口2
static callback_clientMsg clientmsg = NULL;

//服务端状态
static int server_run = 1;

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

//函数名：closeClient
//  功能：关闭指定ip客户端的链接
//  入参：ip 客户端ip，格式为xxx.xxx.xxx.xxx
//返回值：0关闭成功，-1关闭失败
int closeClient(char* ip)
{
	int i;
	int ret = -1;
	if(NULL == ip)
		return ret;
	for(i = 0; i < CLIENT_MAX; i++)
	{
		if(0 == strncmp(ip, client_info[i].client_ip, strlen(ip)))
		{
			if(client_info[i].socketid > 0)
			{
				memset(client_info[i].recvbuffer, 0, RECVBUFF_LEN);
				memset(client_info[i].client_ip, 0, INET_ADDRSTRLEN);
				ret = close(client_info[i].socketid);
				client_info[i].socketid = -1;
			}
		}
	}
	return ret;
}


//函数名：send2Client
//  功能：像指定ip客户端发送消息
//  入参：ip 客户端ip，格式为xxx.xxx.xxx.xxx
//  入参：buf 发送缓冲区首地址
//  入参：len 发送长度
//返回值：>0 发送成功，-1失败
int send2Client(char* ip, char* buf, int len)
{
	int i;
	int ret = -1;
	if((NULL == ip) || (NULL == buf))
		return ret;
	for(i = 0; i < CLIENT_MAX; i ++)
	{
		if(0 == strncmp(ip, client_info[i].client_ip, strlen(ip)))
		{
			if(client_info[i].socketid > 0)
			{
				ret = send(client_info[i].socketid, buf, len, 0);
			}
		}
	}
	return ret;
}

//函数名：broadcast2All
//  功能：广播数据
//  入参：buf 发送缓冲区
//  入参：len 发送缓冲区长度
//返回值：>0 发送成功，值为发送成功的链路数量，-1发送失败
int broadcast2All(char* buf, int len)
{
	int i;
	int ret = -1;
	int count = 0;
	if(NULL == buf)
		return ret;
	for(i = 0; i < CLIENT_MAX; i ++)
	{
		if(client_info[i].socketid > 0)
		{
			ret = send(client_info[i].socketid, buf, len, 0);
			if(ret > 0)
				count++;
		}
	}
	return count;
	
}

//函数名： shutdowns1
//  功能： 关闭服务端
//  入参： 无
//返回值： 无
void shutdowns1(void)
{
	server_run = 0;
}

//用于测试链接关闭,无报文处理
static void link_control()
{
	int i;
	char* closestr = NULL;
	char* sendtostr = NULL;
	char* broadcaststr = NULL;
	char* shutdownstr = NULL;
	for(i = 0; i < CLIENT_MAX; i++)
	{
		if(client_info[i].socketid < 0)
			continue;
		closestr = strstr(client_info[i].recvbuffer, "close");
		sendtostr = strstr(client_info[i].recvbuffer, "sendto");
		broadcaststr = strstr(client_info[i].recvbuffer, "broadcast");
		shutdownstr = strstr(client_info[i].recvbuffer, "shutdown");
		if(NULL != closestr)
		{
			closeClient("192.168.0.102");
		}
		else if(NULL != sendtostr)
		{
			send2Client("192.168.0.102", "sendto test msg", strlen("sendto test msg"));
		}
		else if(NULL != broadcaststr)
		{
			broadcast2All("test broadcast msg", strlen("test broadcast msg"));
		}
		else if(NULL != shutdownstr)
		{
			shutdowns1();
		}
	}	
}

//函数名：Server_init
//  功能：服务器初始化
//  入参：ip   服务器端ip，格式为xxx.xxx.xxx.xxx
//  入参：port 服务器端端口号，0-65535
//返回值：0初始化成功，>0 初始化失败，详见errno
int Server_init(char* ip, int port)
{
	int i = 0;
	int opt = 1;
	int ret = -1;
	int max_socketid = -1;
	int listen_socket = -1;
	int connectedsocket = -1;
	struct timeval timeout = {0, 0};
	fd_set listen_set;
	socklen_t clientaddr_len;
	struct sockaddr_in serveraddr, clientaddr;	
	char recvbuffer[RECVBUFF_LEN];
	char sendbuffer[SENDBUFF_LEN];

	listen_socket = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
	if(listen_socket < 0)
	{
		return errno;
	}
	ret = setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));	
	if(ret < 0)
	{
		return errno;
	}
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
	memset(client_info, -1, sizeof(client_info));
	
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
			if(client_info[i].socketid > 0)
				FD_SET(client_info[i].socketid, &listen_set);
			if(client_info[i].socketid > max_socketid)
				max_socketid = client_info[i].socketid;
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
				if(client_info[i].socketid < 0)
				{
					client_info[i].socketid = connectedsocket;
					break;
				}
			}
			if(clientstatus)
				clientstatus((char *)inet_ntop(AF_INET, &clientaddr.sin_addr, ipstr, sizeof(ipstr)), \
			       		ntohs(clientaddr.sin_port), STATUS_NEW);
		}
		for(i = 0; i < CLIENT_MAX; i++)
		{
			connectedsocket = client_info[i].socketid;
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
					client_info[i].socketid = -1;
				}
				else if(ret < 0)
				{
					if(errno != EWOULDBLOCK)
					{	
						if(clientstatus)
							clientstatus((char *)inet_ntop(AF_INET, &clientaddr.sin_addr, ipstr, sizeof(ipstr)), \
								ntohs(clientaddr.sin_port), STATUS_DISC);
						close(connectedsocket);
						client_info[i].socketid = -1;
					}
				
				}
				else
				{
					printf("server:%s:%d received from %s at PORT %d\n",
						ip, port, inet_ntop(AF_INET, &clientaddr.sin_addr, ipstr, sizeof(ipstr)),
						ntohs(clientaddr.sin_port));
					memcpy(client_info[i].recvbuffer, recvbuffer, ret);
					client_info[i].recvbuffer[ret] = '\0';
					sprintf(client_info[i].client_ip, "%s", inet_ntop(AF_INET, &clientaddr.sin_addr, ipstr, sizeof(ipstr)));
					if(clientmsg)
						clientmsg(client_info[i].client_ip, port, client_info[i].recvbuffer, ret);
					for(i = 0; i < ret; i++)
						sendbuffer[i] = recvbuffer[i] + 1;
					send(connectedsocket, sendbuffer, ret, 0);
					if(clientstatus)
						clientstatus((char *)inet_ntop(AF_INET, &clientaddr.sin_addr, ipstr, sizeof(ipstr)), \
							ntohs(clientaddr.sin_port), STATUS_SEND);
					link_control();
				}
			}
		}
	}
	for(i = 0; i < CLIENT_MAX; i++)
	{
		connectedsocket = client_info[i].socketid;
		if(connectedsocket < 0)
			continue;
		if(FD_ISSET(connectedsocket, &listen_set))
		{
			close(connectedsocket);
		}
	}
	return 0;
}
