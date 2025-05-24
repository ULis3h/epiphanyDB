#ifndef EPIPHANY_OS_INTERNAL_H
#define EPIPHANY_OS_INTERNAL_H

#include "os_cfg.h"


/* OS Internal definitions and structures */

/** Task Control Block redefinition. */
typedef os_tcb OS_TCB;

/** Task Control Block data type declaration */
struct os_tcb {
    CPU_STK *StkPtr; ///< Stack pointer.
    CPUY_STK_SIZE StkSize;
};

/**
 * @brief Task function pointer redefinition
 * @param p_arg Task function parameter
 */
typedef void (*OS_TASK_PTR)(void *p_arg);

typedef enum os_error {
    OS_ERR_NONE = 0u,
    OS_ERR_A = 10000u,
    OS_ERR_ACCEPT_ISR = OS_ERR_A + 1,
    OS_ERR_B = 11000u,
    OS_ERR_C = 12000u,
    OS_ERR_CREATE_ISR = OS_ERR_C + 1,
    OS_ERR_X = 33000u,
    OS_ERR_Y = 34000u,
    OS_ERR_YIELD_ISR = OS_ERR_Y + 1,
    OS_ERR_Z = 35000u
} OS_ERR;

typedef struct os_rdy_list OS_RDY_LIST;

struct os_rdy_list {
    OS_TCB *HeadPtr; /** Head pointer, pointing to the task control block */
    OS_TCB *TailPtr;
};

/** To implement variable definition in header files */
#ifdef OS_GLOBALS
#define OS_EXT
#else
#define OS_EXT extern
#endif

OS_EXT OS_TCB *OSTCBCurPtr;
OS_EXT OS_TCB *OSTCBHighRdyPtr;

/** Task ready queue */
OS_EXT OS_RDY_LIST OSRdyList[OS_CFG_PRIO_MAX];

OS_EXT OS_STATE OSRunning;

#define OS_STATE_OS_STOPPED (OS_STATE)(0u)
#define OS_STATE_OS_RUNNING (OS_STATE)(0u)

#endif /* EPIPHANY_OS_INTERNAL_H */