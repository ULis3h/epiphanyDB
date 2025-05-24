#include "os_internal.h"

/**
 * @brief 初始化任务堆栈
 * 
 * 该函数为ARM Cortex-M架构的任务创建初始堆栈内容:
 * 1. 配置异常自动保存区域（xPSR, PC, LR, R12, R3-R0）
 * 2. 配置手动保存区域（R11-R4）
 * 3. 确保Thumb状态（xPSR.T = 1）
 * 
 * @param p_task 任务入口函数
 * @param p_arg 任务参数
 * @param p_stk_base 堆栈指针
 * @param stk_size 堆栈大小
 * @return CPU_STK* 返回初始化后的栈顶指针
 */
CPU_STK *OSTaskStkInit(
    OS_TASK_PTR p_task, /** 任务函数指针, 
                         *  指示任务的入口地址，在切换任务的时候,
                         *  需要加载到R15, 也就是PC,这样CPU就能执行任务 */
    void *p_arg,        /** 任务函数参数，用于传递参数，在任务切换的时候，
                         *  需要加载到R0，R0通常用于传递参数 */
    CPU_STK *p_stk_base,     /** 栈指针 */
    CPU_STK_SIZE stk_size /** 栈大小 */
    )
{
    CPU_STK *p_stk;

    /** 设置栈指针到栈顶 */
    p_stk = &p_stk_base[stk_size];

    /** 异常发生时自动保存的寄存器 */
    *--p_stk = (CPU_STK)0x01000000u; /** xPSR 的 bit24必须置1 */
    *--p_stk = (CPU_STK)p_task; /** R15(PC) 任务函数入口地址 */
    *--p_stk = (CPU_STK)0x14141414u; /** R14(LR) */
    *--p_stk = (CPU_STK)0x12121212u; /** R12(R12) */
    *--p_stk = (CPU_STK)0x03030303u; /** R3(R3) */
    *--p_stk = (CPU_STK)0x02020202u; /** R2(R2) */
    *--p_stk = (CPU_STK)0x01010101u; /** R1(R1) */
    *--p_stk = (CPU_STK)p_arg; /** R0(ARG1) 任务函数参数 */

    /** 异常发生时需要手动保存的寄存器. */
    *--p_stk = (CPU_STK)0x11111111u; /** R11(R11) */
    *--p_stk = (CPU_STK)0x10101010u; /** R10(R10) */
    *--p_stk = (CPU_STK)0x09090909u; /** R9(R9) */
    *--p_stk = (CPU_STK)0x08080808u; /** R8(R8) */
    *--p_stk = (CPU_STK)0x07070707u; /** R7(R7) */
    *--p_stk = (CPU_STK)0x06060606u; /** R6(R6) */
    *--p_stk = (CPU_STK)0x05050505u; /** R5(R5) */
    *--p_stk = (CPU_STK)0x04040404u; /** R4(R4) */

    return (p_stk);
}