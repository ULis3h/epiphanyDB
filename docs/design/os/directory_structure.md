# OS Directory Structure Design

```
src/os/
├── core/                      # OS核心功能实现
│   ├── os_task.c             # 任务管理
│   ├── os_var.c              # 全局变量
│   ├── os_time.c             # 时间管理
│   └── os_sched.c            # 调度器
│
├── include/                   # OS头文件
│   ├── os_cfg.h              # OS配置
│   ├── os_internal.h         # OS内部定义
│   └── cpu.h                 # CPU相关定义
│
├── portable/                  # 可移植层
│   ├── arch/                 # 架构相关代码
│   │   ├── arm/             # ARM架构
│   │   │   ├── armv7m/     # ARMv7-M架构(Cortex-M3/M4/M7)
│   │   │   │   ├── port.c
│   │   │   │   └── portmacro.h
│   │   │   ├── armv6m/     # ARMv6-M架构(Cortex-M0/M0+)
│   │   │   └── armv8m/     # ARMv8-M架构(Cortex-M23/M33)
│   │   └── risc-v/         # 预留其他架构支持
│   │
│   └── compiler/            # 编译器相关代码
│       ├── gcc/            # GCC编译器
│       ├── iar/            # IAR编译器
│       └── keil/           # KEIL编译器
│
└── bsp/                     # 板级支持包
    ├── stm32/              # STM32系列
    │   ├── stm32f1xx/
    │   ├── stm32f4xx/
    │   └── stm32h7xx/
    └── other_vendors/      # 其他厂商的芯片
```

## 目录说明

1. **core/** - OS核心功能
   - 包含与硬件无关的OS核心功能实现
   - 任务管理、调度、同步等基础功能

2. **include/** - 头文件
   - OS对外接口定义
   - 配置选项
   - 内部数据结构定义

3. **portable/** - 可移植层
   - **arch/** - 架构相关代码
     - 支持不同ARM架构系列
     - 上下文切换等架构相关实现
   - **compiler/** - 编译器相关代码
     - 编译器特定的宏和函数

4. **bsp/** - 板级支持包
   - 特定芯片系列的驱动和配置
   - 中断向量表等芯片相关实现

## 移植说明

要支持新的ARM芯片，需要：

1. 在 `portable/arch/arm/` 下添加对应架构支持
2. 在 `bsp/` 下添加相应芯片系列支持
3. 在 `compiler/` 下确保支持目标编译器

## 关键文件职责

- `port.c`: 架构相关的实现，如上下文切换
- `portmacro.h`: 架构相关的宏定义和数据类型
- `os_cpu_c.c`: CPU相关的C语言实现
- `os_cpu_a.asm`: CPU相关的汇编实现
