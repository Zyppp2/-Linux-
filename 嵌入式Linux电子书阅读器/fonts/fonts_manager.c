#include <config.h>                                 /*根据编码值获得点阵位图*/
#include <fonts_manager.h>
#include <string.h>

static PT_FontOpr g_ptFontOprHead = NULL;

int RegisterFontOpr(PT_FontOpr ptFontOpr)            /*原理同disp_manager.c文件中的RegisterDispOpr函数，用于注册字体操作结构体*/
{
	PT_FontOpr ptTmp;

	if (!g_ptFontOprHead)
	{
		g_ptFontOprHead   = ptFontOpr;
		ptFontOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptFontOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext     = ptFontOpr;
		ptFontOpr->ptNext = NULL;
	}

	return 0;
}


void ShowFontOpr(void)                                 /*原理同disp_manager.c文件中的ShowDispOpr函数，它遍历字体操作器链表，输出每个字体操作器的名称*/
{
	int i = 0;
	PT_FontOpr ptTmp = g_ptFontOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_FontOpr GetFontOpr(char *pcName)                      /*原理同disp_manager.c文件中的GetDispOpr函数，查找函数，用于在已注册的字体操作器中找到特定名称的操作。*/
{
	PT_FontOpr ptTmp = g_ptFontOprHead;
	
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


int FontsInit(void)                                       /*作用是初始化字体管理器，调用的ASCIIInit()，GBKInit()，FreeTypeInit()函数分别在ascii.c,gbk.c,fteetype.c文件中定义，分别对应ASCII,国标以及Unicode编码格式，iError获得点阵位图是否成功获得的错误信息，并将信息传递给总指挥Main.c 文件 */
{
	int iError;
	
	iError = ASCIIInit();
	if (iError)
	{
		DBG_PRINTF("ASCIIInit error!\n");
		return -1;
	}

	iError = GBKInit();
	if (iError)
	{
		DBG_PRINTF("GBKInit error!\n");
		return -1;
	}
	
	iError = FreeTypeInit();
	if (iError)
	{
		DBG_PRINTF("FreeTypeInit error!\n");
		return -1;
	}

	return 0;
}

