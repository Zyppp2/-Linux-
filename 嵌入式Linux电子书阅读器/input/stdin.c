
#include <input_manager.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>


static int StdinDevInit(void)         /*stdin�豸��ʼ��   */
{
    struct termios tTTYState;               /*termios�������ն��豸���ԵĽṹ��*/
 
    //get the terminal state
    tcgetattr(STDIN_FILENO, &tTTYState);    /*tcgetattr()�������ڻ�ȡ�ն��豸�����ԣ������䱣�浽tTTYState��*/
                                            /* STDIN_FILENO ��ʾ��׼���� */
    //turn off canonical mode
    tTTYState.c_lflag &= ~ICANON;           /*�ر��ն˵Ĺ淶ģʽ��������ַ��������������ȡ����Ҫ�ȴ�Enter��������*/
                                            /*~ICANON ��һ����λȡ���Ĳ��������Ľ���� ICANON �Ĳ���*/

	//minimum of number input read.
    tTTYState.c_cc[VMIN] = 1;               /* ��һ������ʱ�����̷��� */

    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &tTTYState);   /*�����úõĲ�������*/

	return 0;
}

static int StdinDevExit(void)          /*��stdin�豸�ָ����淶ģʽ*/
{

    struct termios tTTYState;
 
    //get the terminal state
    tcgetattr(STDIN_FILENO, &tTTYState);
 
    //turn on canonical mode
    tTTYState.c_lflag |= ICANON;
	
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &tTTYState);	
	return 0;
}

static int StdinGetInputEvent(PT_InputEvent ptInputEvent)
{
	/* ��������ݾͶ�ȡ����������
	 * ���û������, ���̷���, ���ȴ�
	 */

	/* select, poll ���Բ��� UNIX�����߼���� */

 	char c;
	
		/* �������� */
	ptInputEvent->iType = INPUT_TYPE_STDIN;            /*˵������*/
	
	c = fgetc(stdin);                                  /* ������ֱ�������� */
	gettimeofday(&ptInputEvent->tTime, NULL);
	
	if (c == 'u')                                      
	{
		ptInputEvent->iVal = INPUT_VALUE_UP;
	}
	else if (c == 'n')
	{
		ptInputEvent->iVal = INPUT_VALUE_DOWN;
	}
	else if (c == 'q')
	{
		ptInputEvent->iVal = INPUT_VALUE_EXIT;
	}
	else
	{
		ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
	}		
	return 0;
 }
 
static T_InputOpr g_tStdinOpr = {                      
	.name          = "stdin",
	.DeviceInit    = StdinDevInit,
	.DeviceExit    = StdinDevExit,
	.GetInputEvent = StdinGetInputEvent,
};


int StdinInit(void)                                /*�����������������ע��stdin*/
{
	return RegisterInputOpr(&g_tStdinOpr);
}

