#ifndef __SOCKETSERVER_H__
#define __SOCKETSERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <netinet/in.h>

//客户端链接状态
#define STATUS_NEW  1	//刚建立链接
#define STATUS_RECV 2	//服务器接收数据中
#define STATUS_SEND 3	//服务器发送数据中
#define STATUS_CON  4	//链接空闲状态
#define STATUS_DISC 5	//刚断开链接

#define SENDBUFF_LEN 1000
#define RECVBUFF_LEN 1000
#define CLIENT_MAX 10
#define LIST_MAX 10000
#define LIST_TIMEOUT 10000
//客户端链接信息
typedef struct client_info
{
	int socketid;
	char client_ip[INET_ADDRSTRLEN];
	char recvbuffer[RECVBUFF_LEN];
	char sendbuffer[RECVBUFF_LEN];
} CLIENT_INFO;

//全局list
typedef struct list
{
	char data;
	struct list* next;
} LIST;


typedef void(* callback_clientStatus)(char* ip, int port, int status);
typedef void(* callback_clientMsg)(char* ip, int port, char* buf, int len);

//客户端连接状态回调函数
extern void register_clientStatus(callback_clientStatus callback);

//客户端消息回调函数
extern void register_clientMsg(callback_clientMsg callback);

//关闭指定ip客户端
extern int closeClient(char* ip);

//发送到指定ip客户端
extern int send2Client(char* ip, char* buf, int len);

//向所有链接广播
extern int broadcast2All(char* buf, int len);

//关闭服务器
extern void shutdowns1(void);

//服务器端初始化
extern int Server_init(char* ip, int port);

#ifdef __cplusplus
}
#endif

#endif
