#ifndef __SOCKETSERVER_H__
#define __SOCKETSERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

//客户端链接状态
#define STATUS_NEW  1	//刚建立链接
#define STATUS_RECV 2	//服务器接收数据中
#define STATUS_SEND 3	//服务器发送数据中
#define STATUS_CON  4	//链接空闲状态
#define STATUS_DISC 5	//刚断开链接

#define SENDBUFF_LEN 1000
#define RECVBUFF_LEN 1000
#define CLIENT_MAX 10

typedef void(* callback_clientStatus)(char* ip, int port, int status);
typedef void(* callback_clientMsg)(char* ip, int port, char* buf, int len);

//客户端连接状态回调函数
extern void register_clientStatus(callback_clientStatus callback);

//客户端消息回调函数
extern void register_clientMsg(callback_clientMsg callback);

//服务器端初始化
extern int Server_init(char* ip, int port);

#ifdef __cplusplus
}
#endif

#endif
