# PN7160 Linux 可移植构建与 SDK 接口

本仓库保留 NXP `NCI2.0_PN7160` 历史，基线为
`d5af4ef88244b9f962986b39eb4126a8c1c0aead`。修改直接体现在派生仓库的源码提交中。
`master` 提供公共原生库；读卡专项功能仍在 `pn7160-sdk-reader` 分支，不自动混入公共基线。

## 支持边界

目标是让同一份 Linux 原生库源码可用于不同 CPU 架构和板级配置。
当前平台后端依赖 Linux i2c-dev、sysfs GPIO、POSIX 线程/时间接口，尚不是 Windows、RTOS 或裸机 SDK。
更换 Linux SoC 时仍需核对 GPIO、中断行为、权限、I2C 驱动及复位时序。
没有 sysfs GPIO 的内核需要新增 GPIO 后端（如 libgpiod），仅改变 GPIO 数字不能解决。
未完成的后端不会通过“自动识别树莓派型号”来假装适配。

| 层 | 当前实现 | 移植责任 |
|---|---|---|
| 公共配置接口 | C ABI v1，C/C++、Python ctypes 均可调用 | 保持 ABI 布局及所有权契约 |
| 协议栈 | NXP Linux NCI2.0 PN7160 | 保持官方历史、审阅并合入上游变更 |
| I2C | Linux i2c-dev | 新系统需实现等价传输接口 |
| GPIO/IRQ | Linux sysfs 全局 GPIO 编号 | 新内核/SoC 需验证或实现其他后端 |
| 线程与延时 | POSIX | 非 Linux 系统需完整移植，并非仅修改构建文件 |

## 本机编译

需要 C/C++ 工具链、CMake >=3.16、GNU Make、Autoconf、Automake、Libtool。
不需要连接 PN7160，不需要 GPIO 或 I2C 设备存在。

```bash
cmake -S . -B build/native \
  -DCMAKE_BUILD_TYPE=Release \
  -DPN7160_DEFAULT_I2C_DEVICE=/dev/i2c-1 \
  -DPN7160_DEFAULT_I2C_ADDRESS=0x28 \
  -DPN7160_DEFAULT_IRQ_GPIO=535 \
  -DPN7160_DEFAULT_VEN_GPIO=536 \
  -DPN7160_DEFAULT_DWL_REQ_GPIO=-1 \
  -DPN7160_DEFAULT_RESET_DELAY_US=10000
cmake --build build/native
ctest --test-dir build/native --output-on-failure
```

GPIO 数字仅为示例：它们是目标 Linux 的 sysfs 全局编号，不是物理针脚编号，不执行 gpio_base 换算。
DWL=-1 表示不使用此输出。地址是未移位的 7 位地址。默认 GPIO 为 23/24，不能保证匹配任意开发板。

CMake 根据目标编译器的指针宽度识别 32/64 位，检查 C/C++ ABI 和 64 位原子操作链接需求，
需要时添加 `-latomic`。不使用 `uname -m` 或板型识别推断 ABI，也不重复应用历史上的 64 位补丁。
当前 `config.cc` 已包含 32 位重复 case 修复。

CMake 以 Autotools 为官方协议栈构建后端，将源码复制到构建目录后 bootstrap/configure/make，
不会把生成文件写进 Git 源码目录。默认安装位置为 `build/native/install`，无需 sudo。
可用 `-DCMAKE_INSTALL_PREFIX=/absolute/path` 更改。`cmake --build` 包括安装到该前缀。
并行度通过 `-DPN7160_BUILD_JOBS=4` 设置。

运行时 NXP 配置文件搜索目录仍为 `/usr/local/etc/`；私有安装前缀不会自动改变该目录。
安装结果的 `etc/libnfc-*.conf` 需要按部署环境放入运行时配置目录。SDK 安装脚本另行处理此事。
这里的配置测试不访问硬件，不能代替射频/卡片实测。

## 交叉编译

示例工具链不下载安装交叉编译器，也不包含目标 sysroot：

```bash
cmake -S . -B build/aarch64 \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-aarch64.cmake \
  -DPN7160_DEFAULT_IRQ_GPIO=535 -DPN7160_DEFAULT_VEN_GPIO=536
cmake --build build/aarch64

cmake -S . -B build/armhf \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-armhf.cmake
cmake --build build/armhf
```

自定义工具链设置 `CMAKE_SYSTEM_NAME=Linux`、目标 C/C++ 编译器、必要的 `CMAKE_SYSROOT`，
以及 `PN7160_AUTOTOOLS_HOST` GNU triplet。可通过目标编译器标志指定最低 CPU/浮点 ABI。
ARMHF 工具链默认值不代表 ARMv6 或所有树莓派兼容。不同 ABI 使用不同构建目录。
更换编译器、sysroot 或 ABI 会拒绝复用旧目录；默认值或编译选项变化会清理旧对象后重建。
交叉编译得到的测试程序需在目标机运行；本机不可直接执行 ARM 程序。
工具链示例不是完成硬件验证的声明。CMake 配置阶段不运行目标程序。

## C ABI 与编译期默认值

```c
#include "pn7160_board_config.h"
#include "linux_nfc_api.h"

int main(void) {
    pn7160_config_t c;
    if (pn7160_get_default_config(&c, sizeof(c))) return 1;
    /* 可在此覆盖 c 的字段。 */
    if (pn7160_set_config(&c)) return 1;
    if (doInitialize()) return 1;
    /* 业务操作。 */
    return doDeinitialize();
}
```

`pn7160_get_default_config` 返回库编译时的默认值，不会启用硬件或改变活动配置。
仍须显式调用 setter。setter 复制路径、校验参数，初始化后禁止变更配置。
getter 输出中的路径归库所有，不能释放。结构体的 reserved 字段必须为零。
空指针、容量不足返回 INVALID 且不写输出。新 getter 不改变 ABI v1 布局。
Python 可继续传完整结构体，无需读取默认值；Python JSON 与编译默认值不自动合并。

CMake 从 `cmake/board_defaults.h.in` 生成配置头，并传给原生库。
直接使用 Autotools 时，也可通过 `CPPFLAGS=-DPN7160_DEFAULT_IRQ_GPIO=...` 等宏覆盖
`src/include/pn7160_board_defaults.h` 的默认值；字符串路径推荐使用生成头。
宏必须传给库的编译过程，仅给应用传宏不会改变已构建的 `.so`。
更新默认值必须重新编译，不能靠更改运行时环境变量生效。
示例 `nfcDemoApp` 已显式读取/设置默认值后再初始化。

## Git 审阅与验证

```bash
git diff --check
git diff --stat
git diff -- src/include src/pn7160_board_config.cpp demoapp/main.c Makefile.am
git diff -- CMakeLists.txt cmake tests PORTING.md README.md
git log --oneline d5af4ef..HEAD
```

源码和补丁进入 Git；构建目录、动态库、发布压缩包不进入源码提交。
官方源码已提交到派生仓库，因此不要再次应用 SDK 的官方 64 位补丁。
`ctest` 检查 C ABI、编译默认值、参数校验与生命周期，不访问 PN7160。
正式声明某个 SoC 支持前，需补充对应系统/内核/编译器及读卡、写卡、APDU、模拟等实测记录。
