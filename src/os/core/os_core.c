#include "os_internal.h"

void 
OS_RdyListInit(void)
{
    OS_PRIO i;
    OS_RDY_LIST *p_rdy_list;

    for (int i = 0u; i < OS_CFG_PRIO_MAX; i++)
    {
        p_rdy_list = &OSRdyList[i];
        p_rdy_list->HeadPtr = (OS_TCB *)0;
        p_rdy_list->TailPtr = (OS_TCB *)0;
    }
}

void 
OSInit(OS_ERR *p_err)
{
    /**
     * 系统用一个全局变量OSRunning来指示系统的运行状态,刚开始初始化系统时,默认为停止状态,即
     * OS_STATE_OS_STOPPED.
     */
    OSRunning = OS_STATE_OS_STOPPED;

    /**
     * 全局变量OSTCBCurPtr是系统用于指向当前运行的任务的TCB指针,在任务切换的时候用到.
     */
    OSTCBCurPtr = (OS_TCB*)0;

    /**
     * 全局变量OSTCBHighRdyPtr用于指向就绪任务中优先级最高的任务的TCB,在任务切换的时候用到.
     */
    OSTCBHighRdyPtr = (OS_TCB*)0;

    /**
     * 用于初始化全局变量OSRdyList[],初始化就绪列表.
     */
    OS_RdyListInit();

    *p_err = OS_ERR_NONE;
}

void 
OSStart(OS_ERR *p_err)
{
    /** 系统第一次启动. */
    if ( OSRunning == OS_STATE_OS_STOPPED )
    {
        OSTCBHighRdyPtr = OSRdyList[0].HeadPtr;

        /** 启动任务切换, 不会返回. */
        OSStartHighRdy();

        *p_err = OS_ERR_FATAL_RETURN;
    }
    else {
        *p_err = OS_STATE_OS_RUNNING;
    }
}