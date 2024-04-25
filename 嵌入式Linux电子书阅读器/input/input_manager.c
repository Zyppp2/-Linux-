 
#include <config.h>                                  /*多线程输入管理*/
#include <input_manager.h>
#include <string.h>

static PT_InputOpr g_ptInputOprHead;
static T_InputEvent g_tInputEvent;

static pthread_mutex_t g_tMutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_tConVar = PTHREAD_COND_INITIALIZER;

int RegisterInputOpr(PT_InputOpr ptInputOpr)  /*注册输入设备，即将输入设备结构体加入链表末端*/
{
	PT_InputOpr ptTmp;

	if (!g_ptInputOprHead)
	{
		g_ptInputOprHead   = ptInputOpr;
		ptInputOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptInputOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext	  = ptInputOpr;
		ptInputOpr->ptNext = NULL;
	}

	return 0;
}


void ShowInputOpr(void)                          /*打印所有输入设备名称*/
{
	int i = 0;
	PT_InputOpr ptTmp = g_ptInputOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

static void *InputEventTreadFunction(void *pVoid)  /*输入事件线程的执行函数，从GetInputEvent*/
{
	T_InputEvent tInputEvent;
	
	/* 定义函数指针 */
	int (*GetInputEvent)(PT_InputEvent ptInputEvent);
	GetInputEvent = (int (*)(PT_InputEvent))pVoid;

	while (1)
	{
		if(0 == GetInputEvent(&tInputEvent))
		{
			/* 唤醒主线程, 把tInputEvent的值赋给一个全局变量 */
			/* 访问临界资源前，先获得互斥量 */
			pthread_mutex_lock(&g_tMutex);
			g_tInputEvent = tInputEvent;

			/*  唤醒主线程 */
			pthread_cond_signal(&g_tConVar);

			/* 释放互斥量 */
			pthread_mutex_unlock(&g_tMutex);
		}
	}

	return NULL;
}

int AllInputDevicesInit(void)            /*先执行这个函数，初始化所有注册的输入设备*/
{
	PT_InputOpr ptTmp = g_ptInputOprHead;
	int iError = -1;

	while (ptTmp)
	{
		if (0 == ptTmp->DeviceInit())              /*依次初始化输入设备，并分别创建子线程*/
		{
			/* 创建子线程 */
			pthread_create(&ptTmp->tTreadID, NULL, InputEventTreadFunction, ptTmp->GetInputEvent);			
			iError = 0;                             /*新创建线程的标识符存入ptTmp->tTreadID中，NULL使用默认属性*/
		}                                           /*入口函数使用InputEventTreadFunction，ptTmp->GetInputEvent为传入InputEventTreadFunction函数的参数*/ 
		ptTmp = ptTmp->ptNext;
	}
	return iError;
}

int GetInputEvent(PT_InputEvent ptInputEvent)  
{
	/* 休眠 */
	pthread_mutex_lock(&g_tMutex);                   /*第二执行这个函数，使线程进入休眠等待被唤醒，输入事件传入后转到InputEventTreadFunction函数执行操作*/
	pthread_cond_wait(&g_tConVar, &g_tMutex);	

	/* 被唤醒后,返回数据 */
	*ptInputEvent = g_tInputEvent;                    /*如果 GetInputEvent 函数被多个线程调用，并且每个线程都传递了不同的 ptInputEvent 参数*/
	pthread_mutex_unlock(&g_tMutex);                  /*那么这个赋值操作就可以确保每个线程获取到的数据都被正确地复制到了各自的内存位置，而不会相互影响*/
	return 0;	
}

int InputInit(void)
{
	int iError;
	iError = StdinInit();
	iError |= TouchScreenInit();
	return iError;
}

