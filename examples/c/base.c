#include <stdlib.h>

#include "epiphany.h"

/** 数据库主目录. */
const char* DB_HOME_DIR = "./";

/** 缓冲池大小. */
const size_t MEM_POOL_SIZE = 1024 * 1024;

int main(int argc, char* argv[]) {

    EPIPHANY_RET rc = EPIPHANY_OK;
    EPIPHANY_CONNECTION *conn;
    
    /** 申请一块内存给存储引擎使用. */
    void* mem_pool_base = malloc(MEM_POOL_SIZE);

    /** 创建配置结构 */
    EPIPHANY_CONFIG* config = epiphany_config_create(DB_HOME_DIR, MODE_DEFAULT, 
                                           mem_pool_base, MEM_POOL_SIZE);

    /** 创建存储引擎 */
    rc = epiphany_open_by_cfg(config, &conn); // enpiphany_open("./", "create", &conn);

    


    /** 销毁配置结构 */
    epiphany_config_destroy(config);

    return 0;
}
