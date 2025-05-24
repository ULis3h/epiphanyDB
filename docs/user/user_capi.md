# C接口手册

### 创建配置 - enpiphany_config_create

只有当调用enpiphany_open_by_cfg接口时才需要创建配置.

```c
EPIPHANY_CONFIG* epiphany_config_create(const char* home, EPIPHANY_STORAGE_MODE mode, 
                              void* cache_base, size_t cache_size);
```
- home : 数据库主目录
- mode : 打开模式
- cache_base : 缓存基地址
- cache_size : 缓存大小

### 销毁配置 - enpiphany_config_destroy

```c
void epiphany_config_destroy(EPIPHANY_CONFIG* cfg);
```
- cfg : 配置指针

### 打开数据库 - enpiphany_open_by_cfg

```c
EPIPHANY_RET epiphany_open_by_cfg(EPIPHANY_CONFIG* config, EPIPHANY_CONNECTION** connection);
```
- config : 配置指针
- connection : 连接指针

### 关闭数据库 - enpiphany_close

```c
EPIPHANY_RET epiphany_close(EPIPHANY_CONNECTION* connection);
```
- connection : 连接指针 

