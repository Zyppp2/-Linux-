  
#include <config.h>                        /*根据编码值，将获取到的点阵绘制到LCD屏幕上*/
#include <disp_manager.h>
#include <string.h>

static PT_DispOpr g_ptDispOprHead; /* 定义了一个指向PT_DispOpr结构体的指针，作为显示操作链表的头指针 */

int RegisterDispOpr(PT_DispOpr ptDispOpr) /* 这个函数用于向 在LCD上执行显示操作 的链表注册新的显示操作 */
{                                                /* 函数接受类型为PT_DispOpr的指针作为参数，表示要注册的新的显示操作 */
	PT_DispOpr ptTmp;   /* 临时指针 */

	if (!g_ptDispOprHead)                       
	{                                           /* 函数检查开始时定义的指针g_ptDispOprHead是否为空。如果为空   */                                      
		g_ptDispOprHead   = ptDispOpr;          /* 表示链表还没有任何显示操作，那么将新的显示操作指针ptDispOpr */
		ptDispOpr->ptNext = NULL;               /* 赋值给g_ptDispOprHead，并将该显示操作的ptNext指针设为NULL， */	                                                                        
	}											/* 表示链表中只有一个节点                                      */
	else
	{
		ptTmp = g_ptDispOprHead;                /* 如果链表不为空，则需要遍历链表找到尾部，然后将新的显示操作添加到链表末尾       */
		while (ptTmp->ptNext)	                /* 首先使临时指针ptTmp指向链表头g_ptDispOprHead。然后遍历链表直到找到最后一个节点 */
		{                                       /* 在循环结束后，ptTmp指向了链表中的最后一个节点。接着，将新的显示操作指针ptDispOpr赋值给ptTmp->ptNext */
			ptTmp = ptTmp->ptNext;              /* 即将其链接到链表的末尾，并将其ptNext指针设为NULL，表示该节点为链表中的最后一个节点。*/
		}
		ptTmp->ptNext	  = ptDispOpr;
		ptDispOpr->ptNext = NULL;
	}

	return 0;
}


void ShowDispOpr(void)                     /* 显示已注册的显示操作的信息，它遍历显示操作链表，输出每个显示操作的索引号和名称 */
{                                                /* 开发人员可以方便地了解系统中的显示操作 */
	int i = 0;
	PT_DispOpr ptTmp = g_ptDispOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_DispOpr GetDispOpr(char *pcName)          /* 它是一个查找选取函数，用于在已注册的显示操作中找到特定名称的操作。 */
{
	PT_DispOpr ptTmp = g_ptDispOprHead;
	
	while (ptTmp) 
	{
		if (strcmp(ptTmp->name, pcName) == 0)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

int DisplayInit(void)         /*作用是初始化显示设备，调用的FBInit()在Fb.c文件中定义，iError获得LCD是否成功打开的错误信息，并将信息传递给总指挥Main.c 文件 */
{
	int iError;
	
	iError = FBInit();

	return iError;
}

