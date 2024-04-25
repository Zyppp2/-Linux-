#include <config.h>
#include <debug_manager.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>

#define SERVER_PORT 5678
#define PRINT_BUF_SIZE   (16*1024)

static int g_iSocketServer;
static struct sockaddr_in g_tSocketServerAddr;
static struct sockaddr_in g_tSocketClientAddr;
static int g_iHaveConnected = 0;
static char *g_pcNetPrintBuf;
static int g_iReadPos  = 0;
static int g_iWritePos = 0;

static pthread_t g_tSendTreadID;
static pthread_t g_tRecvTreadID;

static pthread_mutex_t g_tNetDbgSendMutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_tNetDbgSendConVar = PTHREAD_COND_INITIALIZER;

static int isFull(void)                     /*如果环形缓冲区满返回1*/
{
	return (((g_iWritePos + 1) % PRINT_BUF_SIZE) == g_iReadPos);
}

static int isEmpty(void)                    /*如果环形缓冲区为空返回1*/
{
	return (g_iWritePos == g_iReadPos);
}


static int PutData(char cVal)     /*将数据放入环形缓冲区中*/
{
	if (isFull())
		return -1;
	else
	{
		g_pcNetPrintBuf[g_iWritePos] = cVal;
		g_iWritePos = (g_iWritePos + 1) % PRINT_BUF_SIZE;
		return 0;
	}	
}

static int GetData(char *pcVal)   /*从环形缓冲区获取数据*/
{
	if (isEmpty())
		return -1;
	else
	{
		*pcVal = g_pcNetPrintBuf[g_iReadPos];
		g_iReadPos = (g_iReadPos + 1) % PRINT_BUF_SIZE;
		return 0;
	}
}


static void *NetDbgSendTreadFunction(void *pVoid) /*负责实时监视环形缓冲区中的数据，并将其发送给已连接的客户端*/
{
	char strTmpBuf[512];
	char cVal;
	int i;
	int iAddrLen;
	int iSendLen;
	
	while (1)
	{
		/* 平时休眠 */
		pthread_mutex_lock(&g_tNetDbgSendMutex);     /*获取互斥锁后，确保只有一个线程可以进入临界区*/
		pthread_cond_wait(&g_tNetDbgSendConVar, &g_tNetDbgSendMutex);	/*使线程等待并在等待期间释放互斥锁*/
																		/*允许其他线程进入临界区，为了等待期间线程被唤醒后能够重新获取锁并检查条件，同时允许其他线程访问共享资源。*/
		pthread_mutex_unlock(&g_tNetDbgSendMutex); 
		while (g_iHaveConnected && !isEmpty())
		{
			i = 0;

			/* 把环形缓冲区的数据取出来, 最多取512字节 */
			while ((i < 512) && (0 == GetData(&cVal)))
			{
				strTmpBuf[i] = cVal;
				i++;
			}
			
			/* 执行到这里, 表示被唤醒 */
			/* 用sendto函数发送打印信息给客户端 */
			iAddrLen = sizeof(struct sockaddr);
			iSendLen = sendto(g_iSocketServer, strTmpBuf, i, 0,
			                      (const struct sockaddr *)&g_tSocketClientAddr, iAddrLen);

		}

	}
	return NULL;
}

static void *NetDbgRecvTreadFunction(void *pVoid)
{
	socklen_t iAddrLen;                       /*处理接收到的客户端消息，根据消息内容执行相应的操作*/
	int iRecvLen;
	char ucRecvBuf[1000];
	struct sockaddr_in tSocketClientAddr;

	while (1)
	{
		iAddrLen = sizeof(struct sockaddr);
		DBG_PRINTF("in NetDbgRecvTreadFunction\n");
		iRecvLen = recvfrom(g_iSocketServer, ucRecvBuf, 999, 0, (struct sockaddr *)&tSocketClientAddr, &iAddrLen);
		
		if (iRecvLen > 0)	
		{
			ucRecvBuf[iRecvLen] = '\0';
			DBG_PRINTF("netprint.c get msg: %s\n", ucRecvBuf);
			
			/* 解析数据:
			 * setclient            : 设置接收打印信息的客户端
			 * dbglevel=0,1,2...    : 修改打印级别
			 * stdout=0             : 关闭stdout打印
			 * stdout=1             : 打开stdout打印
			 * netprint=0           : 关闭netprint打印
			 * netprint=1           : 打开netprint打印
			 */
			if (strcmp(ucRecvBuf, "setclient")  == 0)      /*设置客户端地址*/
			{
				g_tSocketClientAddr = tSocketClientAddr;
				g_iHaveConnected = 1;
			}
			else if (strncmp(ucRecvBuf, "dbglevel=", 9) == 0)  /*修改调试级别*/
			{
				SetDbgLevel(ucRecvBuf);
			}
			else
			{
			SetDbgChanel(ucRecvBuf);   /*设置特定调试通道的可用性*/
			}
		}
		
	}
	return NULL;
}


static int NetDbgInit(void)
{
	/* socket初始化 */
	int iRet;
	
	g_iSocketServer = socket(AF_INET, SOCK_DGRAM, 0);   /*创建套接字，用于网络通信，类似于普通的文件描述符*/
	if (-1 == g_iSocketServer)
	{
		printf("socket error!\n");
		return -1;
	}

	g_tSocketServerAddr.sin_family      = AF_INET;      /*设置服务器端套接字地址结构体的各个字段*/
	g_tSocketServerAddr.sin_port        = htons(SERVER_PORT);  /* host to net, short */
 	g_tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;
	memset(g_tSocketServerAddr.sin_zero, 0, 8);
	
	iRet = bind(g_iSocketServer, (const struct sockaddr *)&g_tSocketServerAddr, sizeof(struct sockaddr));
	if (-1 == iRet)                                    /*使用bind函数将套接字绑定到服务器的 IP 地址和端口号上*/
	{                                                  /*以便服务器能够监听来自客户端的连接请求*/
		printf("bind error!\n");
		return -1;
	}

	g_pcNetPrintBuf = malloc(PRINT_BUF_SIZE);
	if (NULL == g_pcNetPrintBuf)
	{
		close(g_iSocketServer);
		return -1;
	}


	/* 创建netprint发送线程: 它用来发送打印信息给客户端 */
	pthread_create(&g_tSendTreadID, NULL, NetDbgSendTreadFunction, NULL);			
	
	/* 创建netprint接收线程: 用来接收控制信息,比如修改打印级别,打开/关闭打印 */
	pthread_create(&g_tRecvTreadID, NULL, NetDbgRecvTreadFunction, NULL);			

	return 0;	
}

static int NetDbgExit(void)      /*关闭套接字和释放分配的内存空间*/
{
	/* 关闭socket,... */
	close(g_iSocketServer);
	free(g_pcNetPrintBuf);
}

static int NetDbgPrint(char *strData)
{
	/* 把数据放入环形缓冲区 */
	int i;
	
	for (i = 0; i < strlen(strData); i++)
	{
		if (0 != PutData(strData[i]))
			break;
	}
	
	/* 如果已经有客户端连接了, 就把数据通过网络发送给客户端 */
	/* 唤醒netprint的发送线程 */
	pthread_mutex_lock(&g_tNetDbgSendMutex);
	pthread_cond_signal(&g_tNetDbgSendConVar);
	pthread_mutex_unlock(&g_tNetDbgSendMutex);

	return i;
	
}


static T_DebugOpr g_tNetDbgOpr = {
	.name       = "netprint",
	.isCanUse   = 1,
	.DebugInit  = NetDbgInit,
	.DebugExit  = NetDbgExit,
	.DebugPrint = NetDbgPrint,
};

int NetPrintInit(void)
{
	return RegisterDebugOpr(&g_tNetDbgOpr);
}

