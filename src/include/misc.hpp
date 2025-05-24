#ifndef _MISC_H_
#define _MISC_H_

#ifdef __cplusplus
    extern "C"
    {
#endif 

/**
 * @brief 数据块大小， 默认为4K，块的文件偏移量都是4k的整数倍. 
 * 
 */
#ifndef EPIPHANY_BLOCK_SIZE
#define EPIPHANY_BLOCK_SIZE 4096
#endif //! EPIPHANY_BLOCK_SIZE

/**
 * @brief 数据块转换成字节. 
 * 
 */
#ifndef EPIPHANY_BLOCK_TO_BYTES
#define EPIPHANY_BLOCK_TO_BYTES(block) ((block)*EPIPHANY_BLOCK_SIZE)
#endif //! EPIPHANY_BLOCK_TO_BYTES

/**
 * @brief 字节转换成数据块. 
 * 
 */
#ifndef EPIPHANY_BYTES_TO_BLOCK
#define EPIPHANY_BYTES_TO_BLOCK(bytes) ((bytes)/EPIPHANY_BLOCK_SIZE)
#endif //! EPIPHANY_BYTES_TO_BLOCK





#ifdef __cplusplus
    }
#endif 

#endif //! _MISC_H_