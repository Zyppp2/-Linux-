
#include <input_manager.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>


static int StdinDevInit(void)         /*stdin设备初始化   */
{
    struct termios tTTYState;               /*termios是描述终端设备属性的结构体*/
 
    //get the terminal state
    tcgetattr(STDIN_FILENO, &tTTYState);    /*tcgetattr()函数用于获取终端设备的属性，并将其保存到tTTYState中*/
                                            /* STDIN_FILENO 表示标准输入 */
    //turn off canonical mode
    tTTYState.c_lflag &= ~ICANON;           /*关闭终端的规范模式，输入的字符会立即被程序读取不需要等待Enter键的输入*/
                                            /*~ICANON 是一个按位取反的操作，它的结果是 ICANON 的补码*/

	//minimum of number input read.
    tTTYState.c_cc[VMIN] = 1;               /* 有一个数据时就立刻返回 */

    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &tTTYState);   /*将设置好的参数传回*/

	return 0;
}

static int StdinDevExit(void)          /*将stdin设备恢复到规范模式*/
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
	/* 如果有数据就读取、处理、返回
	 * 如果没有数据, 立刻返回, 不等待
	 */

	/* select, poll 可以参数 UNIX环境高级编程 */

 	char c;
	
		/* 处理数据 */
	ptInputEvent->iType = INPUT_TYPE_STDIN;            /*说明类型*/
	
	c = fgetc(stdin);                                  /* 会休眠直到有输入 */
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


int StdinInit(void)                                /*在输入管理器链表中注册stdin*/
{
	return RegisterInputOpr(&g_tStdinOpr);
}

