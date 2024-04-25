#include <config.h>
#include <input_manager.h>
#include <stdlib.h>

#include <tslib.h>
#include <draw.h>

/* 参考tslib里的ts_print.c */

static struct tsdev *g_tTSDev;
static int giXres;
static int giYres;

/* 注意: 由于要用到LCD的分辨率, 此函数要在SelectAndInitDisplay之后调用 */
static int TouchScreenDevInit(void)
{
	char *pcTSName = NULL;

	if ((pcTSName = getenv("TSLIB_TSDEVICE")) != NULL ) 
	{
		g_tTSDev = ts_open(pcTSName, 0);       /* 以阻塞方式打开 */
	}
	else
	{
		g_tTSDev = ts_open("/dev/event0", 1); 
	}

	if (!g_tTSDev) {
		DBG_PRINTF(APP_ERR"ts_open error!\n");   /*APP_ERR为打印级别*/
		return -1;
	}

	if (ts_config(g_tTSDev)) {                   /*ts_config()函数接受一个指向tsdev结构体的指针(g_tTSDev)作为参数*/
		DBG_PRINTF("ts_config error!\n");        /*用于设置触摸屏的工作模式、分辨率、校准参数等*/
		return -1;
	}

	if (GetDispResolution(&giXres, &giYres))
	{
		return -1;
	}

	return 0;
}

static int TouchScreenDevExit(void)
{
	return 0;
}


static int isOutOf500ms(struct timeval *ptPreTime, struct timeval *ptNowTime)  /* 判断两次触摸事件是否间隔足够时间*/
{
	int iPreMs;
	int iNowMs;
	
	iPreMs = ptPreTime->tv_sec * 1000 + ptPreTime->tv_usec / 1000;
	iNowMs = ptNowTime->tv_sec * 1000 + ptNowTime->tv_usec / 1000;

	return (iNowMs > iPreMs + 500);
}

static int TouchScreenGetInputEvent(PT_InputEvent ptInputEvent)
{
	struct ts_sample tSamp;                               //struct ts_sample {
	struct ts_sample tSampPressed;                        //    struct timeval tv;    
	struct ts_sample tSampReleased;					      //    int x;  
	int iRet;                                             //    int y;   
	int bStart = 0;                                       //    int pressure;}  
	int iDelta;                                                          

	static struct timeval tPreTime;
	

	while (1)
	{
		iRet = ts_read(g_tTSDev, &tSamp, 1);                /* 如果无数据则休眠 */
		if (iRet == 1)                                      /*ts_read函数从g_tTSDev中读取数量为1的触摸事件数据*/
		{                                                   /*并将数据存储到tsamp数组中 函数返回值表示实际读取到的触摸事件数据的数量*/
			if ((tSamp.pressure > 0) && (bStart == 0))      /*如果读取成功，则返回的值等于1 如果读取失败，则返回的值小于1*/
			{
				/* 刚按下 */
				/* 记录刚开始压下的点 */
				tSampPressed = tSamp;
				bStart = 1;
			}
			
			if (tSamp.pressure <= 0)
			{
				/* 松开 */
				tSampReleased = tSamp;

				/* 处理数据 */
				if (!bStart)
				{
					return -1;
				}
				else
				{
					iDelta = tSampReleased.x - tSampPressed.x;
					ptInputEvent->tTime = tSampReleased.tv;
					ptInputEvent->iType = INPUT_TYPE_TOUCHSCREEN;
					
					if (iDelta > giXres/5)
					{
						/* 翻到上一页 */
						ptInputEvent->iVal = INPUT_VALUE_UP;
					}
					else if (iDelta < 0 - giXres/5)
					{
						/* 翻到下一页 */
						ptInputEvent->iVal = INPUT_VALUE_DOWN;
					}
					else
					{
						ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
					}
					return 0;
				}
			}
		}
		else
		{
			return -1;
		}
	}

	return 0;
}


static T_InputOpr g_tTouchScreenOpr = {
	.name          = "touchscreen",
	.DeviceInit    = TouchScreenDevInit,
	.DeviceExit    = TouchScreenDevExit,
	.GetInputEvent = TouchScreenGetInputEvent,
};

int TouchScreenInit(void)
{
	return RegisterInputOpr(&g_tTouchScreenOpr);
}

