# 原生可移植构建验证记录

日期：2026-10-30。基于 master `68389f0` 的本轮工作区修改。

环境：WSL Ubuntu 22.04、Linux x86_64、GCC/G++ 11.4.0。

已完成：

- CMake 配置、官方 Autotools 全量构建与私有前缀安装。
- NXP 原生库、PN7160 固件库、nfcDemoApp 均编译成功。
- CTest 的 C 调用测试通过：默认值、结构体大小、空指针、配置缺失、设置配置、初始化冻结及非法地址。
- 实际安装的动态库导出 get_default_config、set_config、config_abi_version。
- ctypes 读取实际 NXP 动态库，验证自定义 /dev/i2c-7、IRQ=535、VEN=536。
- 改变 CMake 默认 IRQ=537 后重新构建，实际安装库返回 537，旧对象清理/重建生效。
- 非法地址、重复 GPIO、非 Linux 目标、缺少 GNU triplet 的交叉配置均被拒绝。
- 强制原子操作探测走回退分支，验证链接 libatomic 成功。
- git diff --check 及新增文件空白检查通过。

未完成：

- 本轮尚未使用 ARMHF/AArch64 交叉工具链编译此 master 工作区。
- 本轮未连接 PN7160，未执行 I2C/GPIO/射频/卡片操作。
- 现有 32_release、64_release 包未重建；它们不是本轮新 CMake 构建的 ARM 验证结果。
- libgpiod、Windows、RTOS、裸机平台后端尚未实现。

构建和测试日志位于被 Git 忽略的 build/ 目录。
该记录不构成所有 SoC、所有树莓派或所有 Linux 内核的支持声明。
