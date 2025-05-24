#include "os_internal.h"

void OSTaskCreate(
    OS_TCB *p_tcb, /** 任务控制块指针 */
    OS_TASK_PTR p_task, /** 任务函数指针 */
    void *p_arg, /** 任务函数参数 */
    CPU_STK *p_stk, /** 栈指针 */
    CPU_STK_SIZE stk_size, /** 栈大小 */
    OS_ERR *p_err
    )
{
    CPU_STK *p_sp;

    p_sp = OSTaskStkInit(p_task, p_arg, p_stk, stk_size);

    p_tcb->StkPtr = p_sp;
    p_tcb->StkSize = stk_size;

    *p_err = OS_ERR_NONE;
}