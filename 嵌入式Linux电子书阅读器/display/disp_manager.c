  
#include <config.h>                        /*���ݱ���ֵ������ȡ���ĵ�����Ƶ�LCD��Ļ��*/
#include <disp_manager.h>
#include <string.h>

static PT_DispOpr g_ptDispOprHead; /* ������һ��ָ��PT_DispOpr�ṹ���ָ�룬��Ϊ��ʾ���������ͷָ�� */

int RegisterDispOpr(PT_DispOpr ptDispOpr) /* ������������� ��LCD��ִ����ʾ���� ������ע���µ���ʾ���� */
{                                                /* ������������ΪPT_DispOpr��ָ����Ϊ��������ʾҪע����µ���ʾ���� */
	PT_DispOpr ptTmp;   /* ��ʱָ�� */

	if (!g_ptDispOprHead)                       
	{                                           /* ������鿪ʼʱ�����ָ��g_ptDispOprHead�Ƿ�Ϊ�ա����Ϊ��   */                                      
		g_ptDispOprHead   = ptDispOpr;          /* ��ʾ����û���κ���ʾ��������ô���µ���ʾ����ָ��ptDispOpr */
		ptDispOpr->ptNext = NULL;               /* ��ֵ��g_ptDispOprHead����������ʾ������ptNextָ����ΪNULL�� */	                                                                        
	}											/* ��ʾ������ֻ��һ���ڵ�                                      */
	else
	{
		ptTmp = g_ptDispOprHead;                /* �������Ϊ�գ�����Ҫ���������ҵ�β����Ȼ���µ���ʾ������ӵ�����ĩβ       */
		while (ptTmp->ptNext)	                /* ����ʹ��ʱָ��ptTmpָ������ͷg_ptDispOprHead��Ȼ���������ֱ���ҵ����һ���ڵ� */
		{                                       /* ��ѭ��������ptTmpָ���������е����һ���ڵ㡣���ţ����µ���ʾ����ָ��ptDispOpr��ֵ��ptTmp->ptNext */
			ptTmp = ptTmp->ptNext;              /* ���������ӵ������ĩβ��������ptNextָ����ΪNULL����ʾ�ýڵ�Ϊ�����е����һ���ڵ㡣*/
		}
		ptTmp->ptNext	  = ptDispOpr;
		ptDispOpr->ptNext = NULL;
	}

	return 0;
}


void ShowDispOpr(void)                     /* ��ʾ��ע�����ʾ��������Ϣ����������ʾ�����������ÿ����ʾ�����������ź����� */
{                                                /* ������Ա���Է�����˽�ϵͳ�е���ʾ���� */
	int i = 0;
	PT_DispOpr ptTmp = g_ptDispOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_DispOpr GetDispOpr(char *pcName)          /* ����һ������ѡȡ��������������ע�����ʾ�������ҵ��ض����ƵĲ����� */
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

int DisplayInit(void)         /*�����ǳ�ʼ����ʾ�豸�����õ�FBInit()��Fb.c�ļ��ж��壬iError���LCD�Ƿ�ɹ��򿪵Ĵ�����Ϣ��������Ϣ���ݸ���ָ��Main.c �ļ� */
{
	int iError;
	
	iError = FBInit();

	return iError;
}

