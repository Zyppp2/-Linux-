#include <config.h>                          /* 从文件中获取编码值 */
#include <encoding_manager.h>
#include <string.h>
#include <stdlib.h>

static PT_EncodingOpr g_ptEncodingOprHead;

int RegisterEncodingOpr(PT_EncodingOpr ptEncodingOpr)      /*原理同disp_manager.c文件中的RegisterDispOpr函数，用于注册字体编码操作器结构体*/
{
	PT_EncodingOpr ptTmp;

	if (!g_ptEncodingOprHead)
	{
		g_ptEncodingOprHead   = ptEncodingOpr;
		ptEncodingOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptEncodingOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext	      = ptEncodingOpr;
		ptEncodingOpr->ptNext = NULL;
	}

	return 0;
}



void ShowEncodingOpr(void)                                    /*原理同disp_manager.c文件中的ShowDispOpr函数，它用于遍历字体编码操作器链表，输出每个字体编码操作器的名称*/
{
	int i = 0;
	PT_EncodingOpr ptTmp = g_ptEncodingOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_EncodingOpr SelectEncodingOprForFile(unsigned char *pucFileBufHead)    /*作用是遍历字体编码操作器链表，根据文件内容选择合适的字体编码操作器，并返回该字体编码操作器的指针。*/
{
	PT_EncodingOpr ptTmp = g_ptEncodingOprHead;

	while (ptTmp)
	{	
		if (ptTmp->isSupport(pucFileBufHead))
			return ptTmp;
		else
			ptTmp = ptTmp->ptNext;
	}
	return NULL;
}
       /*将一个字体操作器添加到一个编码操作器支持的字体列表中*/
int AddFontOprForEncoding(PT_EncodingOpr ptEncodingOpr, PT_FontOpr ptFontOpr)  
{
	PT_FontOpr ptFontOprCpy;
	
	if (!ptEncodingOpr || !ptFontOpr)
	{
		return -1;
	}
	else
	{
		ptFontOprCpy = malloc(sizeof(T_FontOpr));
		if (!ptFontOprCpy)
		{
			return -1;                       
		}
		else
		{
			memcpy(ptFontOprCpy, ptFontOpr, sizeof(T_FontOpr));          
			ptFontOprCpy->ptNext = ptEncodingOpr->ptFontOprSupportedHead;/*将新创建的字体操作器 ptFontOprCpy 的 ptNext 指向了原先编码操作器支持的字体列表的头部。
                                                                           这样做是为了确保新创建的字体操作器插入到支持列表的头部，以便它成为新的头部*/
			ptEncodingOpr->ptFontOprSupportedHead = ptFontOprCpy;        /*这一行将编码操作器 ptEncodingOpr 的字体支持列表的头部指针指向了新创建的字体操作器 ptFontOprCpy。
                                                                           原先的字体列表则成为了新字体操作器的下一个字体操作器。*/
			return 0;
		}		
	}
}

int DelFontOprFrmEncoding(PT_EncodingOpr ptEncodingOpr, PT_FontOpr ptFontOpr)
{
	PT_FontOpr ptTmp;
	PT_FontOpr ptPre;
		
	if (!ptEncodingOpr || !ptFontOpr)
	{
		return -1;
	}
	else
	{
		ptTmp = ptEncodingOpr->ptFontOprSupportedHead;
		if (strcmp(ptTmp->name, ptFontOpr->name) == 0)
		{
			/* 删除头节点 */
			ptEncodingOpr->ptFontOprSupportedHead = ptTmp->ptNext;
			free(ptTmp);
			return 0;
		}

		ptPre = ptEncodingOpr->ptFontOprSupportedHead;
		ptTmp = ptPre->ptNext;
		while (ptTmp)
		{
			if (strcmp(ptTmp->name, ptFontOpr->name) == 0)
			{
				/* 从链表里取出、释放 */
				ptPre->ptNext = ptTmp->ptNext;
				free(ptTmp);
				return 0;
			}
			else
			{
				ptPre = ptTmp;
				ptTmp = ptTmp->ptNext; 
			}
		}

		return -1;
	}
}

int EncodingInit(void)
{
	int iError;

	iError = AsciiEncodingInit();
	if (iError)
	{
		DBG_PRINTF("AsciiEncodingInit error!\n");
		return -1;
	}

	iError = Utf16leEncodingInit();
	if (iError)
	{
		DBG_PRINTF("Utf16leEncodingInit error!\n");
		return -1;
	}
	
	iError = Utf16beEncodingInit();
	if (iError)
	{
		DBG_PRINTF("Utf16beEncodingInit error!\n");
		return -1;
	}
	
	iError = Utf8EncodingInit();
	if (iError)
	{
		DBG_PRINTF("Utf8EncodingInit error!\n");
		return -1;
	}

	return 0;
}

