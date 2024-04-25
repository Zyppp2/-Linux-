 
#include <config.h>                                  /*���߳��������*/
#include <input_manager.h>
#include <string.h>

static PT_InputOpr g_ptInputOprHead;
static T_InputEvent g_tInputEvent;

static pthread_mutex_t g_tMutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_tConVar = PTHREAD_COND_INITIALIZER;

int RegisterInputOpr(PT_InputOpr ptInputOpr)  /*ע�������豸�����������豸�ṹ���������ĩ��*/
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


void ShowInputOpr(void)                          /*��ӡ���������豸����*/
{
	int i = 0;
	PT_InputOpr ptTmp = g_ptInputOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

static void *InputEventTreadFunction(void *pVoid)  /*�����¼��̵߳�ִ�к�������GetInputEvent*/
{
	T_InputEvent tInputEvent;
	
	/* ���庯��ָ�� */
	int (*GetInputEvent)(PT_InputEvent ptInputEvent);
	GetInputEvent = (int (*)(PT_InputEvent))pVoid;

	while (1)
	{
		if(0 == GetInputEvent(&tInputEvent))
		{
			/* �������߳�, ��tInputEvent��ֵ����һ��ȫ�ֱ��� */
			/* �����ٽ���Դǰ���Ȼ�û����� */
			pthread_mutex_lock(&g_tMutex);
			g_tInputEvent = tInputEvent;

			/*  �������߳� */
			pthread_cond_signal(&g_tConVar);

			/* �ͷŻ����� */
			pthread_mutex_unlock(&g_tMutex);
		}
	}

	return NULL;
}

int AllInputDevicesInit(void)            /*��ִ�������������ʼ������ע��������豸*/
{
	PT_InputOpr ptTmp = g_ptInputOprHead;
	int iError = -1;

	while (ptTmp)
	{
		if (0 == ptTmp->DeviceInit())              /*���γ�ʼ�������豸�����ֱ𴴽����߳�*/
		{
			/* �������߳� */
			pthread_create(&ptTmp->tTreadID, NULL, InputEventTreadFunction, ptTmp->GetInputEvent);			
			iError = 0;                             /*�´����̵߳ı�ʶ������ptTmp->tTreadID�У�NULLʹ��Ĭ������*/
		}                                           /*��ں���ʹ��InputEventTreadFunction��ptTmp->GetInputEventΪ����InputEventTreadFunction�����Ĳ���*/ 
		ptTmp = ptTmp->ptNext;
	}
	return iError;
}

int GetInputEvent(PT_InputEvent ptInputEvent)  
{
	/* ���� */
	pthread_mutex_lock(&g_tMutex);                   /*�ڶ�ִ�����������ʹ�߳̽������ߵȴ������ѣ������¼������ת��InputEventTreadFunction����ִ�в���*/
	pthread_cond_wait(&g_tConVar, &g_tMutex);	

	/* �����Ѻ�,�������� */
	*ptInputEvent = g_tInputEvent;                    /*��� GetInputEvent ����������̵߳��ã�����ÿ���̶߳������˲�ͬ�� ptInputEvent ����*/
	pthread_mutex_unlock(&g_tMutex);                  /*��ô�����ֵ�����Ϳ���ȷ��ÿ���̻߳�ȡ�������ݶ�����ȷ�ظ��Ƶ��˸��Ե��ڴ�λ�ã��������໥Ӱ��*/
	return 0;	
}

int InputInit(void)
{
	int iError;
	iError = StdinInit();
	iError |= TouchScreenInit();
	return iError;
}

