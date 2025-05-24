#ifndef EPIPHANY_OS_CFG_H
#define EPIPHANY_OS_CFG_H

/* OS Configuration Options */

/** Maximum priority level configuration */
#define OS_CFG_PRIO_MAX        64u     /* Maximum number of priorities */

/** Clock tick configuration */
#define OS_CFG_TICK_RATE_HZ    1000u   /* Tick rate in Hertz (ticks per second) */

/** Task-related configuration */
#define OS_CFG_TASK_STK_LIMIT  128u    /* Minimum allowable task stack size */
#define OS_CFG_TASK_STK_SIZE   512u    /* Default task stack size */

/** Debug configuration */
#define OS_CFG_DBG_EN          1u      /* Enable (1) or Disable (0) debug code/variables */

#endif /* EPIPHANY_OS_CFG_H */