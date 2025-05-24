#ifndef EPIPHANY_OS_CPU_H
#define EPIPHANY_OS_CPU_H

/* CPU related definitions and functions */

typedef unsigned short CPU_INT16U;
typedef unsigned int  CPU_INT32U;
typedef unsigned char  CPU_INT08U;

typedef CPU_INT32U CPU_ADDR;

/** Stack data type redefinition. */
typedef CPU_INT32U CPU_STK;
typedef CPU_ADDR CPUY_STK_SIZE;

typedef volatile CPU_INT32U CPU_REG32;

#endif /* EPIPHANY_OS_CPU_H */