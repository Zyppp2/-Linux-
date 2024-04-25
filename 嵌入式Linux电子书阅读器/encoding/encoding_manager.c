#include <config.h>                          /* ���ļ��л�ȡ����ֵ */
#include <encoding_manager.h>
#include <string.h>
#include <stdlib.h>

static PT_EncodingOpr g_ptEncodingOprHead;

int RegisterEncodingOpr(PT_EncodingOpr ptEncodingOpr)      /*ԭ��ͬdisp_manager.c�ļ��е�RegisterDispOpr����������ע���������������ṹ��*/
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



void ShowEncodingOpr(void)                                    /*ԭ��ͬdisp_manager.c�ļ��е�ShowDispOpr�����������ڱ����������������������ÿ��������������������*/
{
	int i = 0;
	PT_EncodingOpr ptTmp = g_ptEncodingOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_EncodingOpr SelectEncodingOprForFile(unsigned char *pucFileBufHead)    /*�����Ǳ������������������������ļ�����ѡ����ʵ��������������������ظ���������������ָ�롣*/
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
       /*��һ�������������ӵ�һ�����������֧�ֵ������б���*/
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
			ptFontOprCpy->ptNext = ptEncodingOpr->ptFontOprSupportedHead;/*���´�������������� ptFontOprCpy �� ptNext ָ����ԭ�ȱ��������֧�ֵ������б��ͷ����
                                                                           ��������Ϊ��ȷ���´�����������������뵽֧���б��ͷ�����Ա�����Ϊ�µ�ͷ��*/
			ptEncodingOpr->ptFontOprSupportedHead = ptFontOprCpy;        /*��һ�н���������� ptEncodingOpr ������֧���б��ͷ��ָ��ָ�����´�������������� ptFontOprCpy��
                                                                           ԭ�ȵ������б����Ϊ�����������������һ�������������*/
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
			/* ɾ��ͷ�ڵ� */
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
				/* ��������ȡ�����ͷ� */
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

