# PN7160 NXP Linux 原生库派生仓库

基于 NXP 官方 `linux_libnfc-nci` 的 `NCI2.0_PN7160` 分支，保留官方提交历史。
适配基线为 `d5af4ef88244b9f962986b39eb4126a8c1c0aead`。

本仓库提供 PN7160 Linux 原生库、C ABI 配置接口和构建入口，供 C/C++ 应用及 Python SDK 调用。
TUI、Python 业务封装和发布包由独立 SDK 项目维护，不包含在本仓库中。

## 1. 与官方版本的差异

| 修改 | 作用 |
|---|---|
| 32/64 位兼容修复 | 合入官方 64 位兼容修改，并修正其在 32 位下的重复 `case` |
| 板级配置 C ABI v1 | 通过结构体设置 I²C 路径、地址、GPIO 和复位时序 |
| 编译期默认配置 | 通过 CMake 参数或编译宏定义默认值，运行时仍可覆盖 |
| 初始化与资源处理 | 配置校验、路径复制、配置冻结及部分文件描述符处理修复 |
| NCI 诊断 | 提供有限长度的传输记录接口 |
| 运行配置路径 | 默认跟随安装配置目录，支持初始化前显式覆盖 |
| CMake 构建入口 | 直接定义原生库、固件库和示例目标；探测目标 ABI 与 libatomic |
| 示例适配 | `nfcDemoApp` 显式读取并设置默认配置后初始化 |

此派生版本已包含兼容补丁，不要再次应用 `64bit_patch/ROOT_src.patch`。
`master` 是公共原生库基线；已有读卡专项改动保留在 `pn7160-sdk-reader` 分支，未自动合入公共基线。

## 2. 平台范围

- 当前后端依赖 Linux i2c-dev、sysfs GPIO 和 POSIX 线程/时间接口。
- CMake 使用目标编译器判断 32/64 位 ABI，不通过宿主机 `uname` 或树莓派型号推断目标架构。
- 不同 Linux SoC 需核对驱动、GPIO 编号、中断行为、权限和复位时序。
- sysfs GPIO 不可用的内核需要实现其他 GPIO 后端；当前未提供 libgpiod、Windows、RTOS 或裸机后端。
- ARMHF 不等于 ARMv6，工具链模板不代表所有树莓派型号均已验证。

原生 CMake/Ninja 已在 WSL Ubuntu 22.04 / x86_64 / GCC 11.4 完成全量构建、安装及配置目录覆盖验证。
ARM32/ARM64 新入口及目标硬件仍需分别验证；编译成功不能代替 PN7160 实机验证。

## 3. 本机编译

Debian、Ubuntu 或 Raspberry Pi OS 可安装以下依赖：

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config
```

在仓库根目录执行，构建本身不需要 sudo 或连接 PN7160：

```bash
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release
cmake --build build/native --parallel
cmake --install build/native
```

默认安装到 `build/native/install`，其中包含：

```text
include/       公共 C 头文件
lib/           NXP 原生库、固件库及 pkg-config 文件
sbin/          nfcDemoApp
etc/           NXP 配置文件模板
```

本机编译不需要指定 ARM32/ARM64，也不需要工具链文件：CMake 使用本机编译器识别目标 ABI。
可以通过 `-DCMAKE_INSTALL_PREFIX=/absolute/path` 修改安装前缀，
通过 `-DCMAKE_INSTALL_SYSCONFDIR=etc` 指定前缀内的配置目录，或者传入绝对配置目录。
必要时用 `-DCMAKE_INSTALL_LIBDIR=lib` 明确库目录。修改安装路径后需重新构建、安装。

CMake 直接编译源文件，不调用 bootstrap、configure、Makefile.am，也不需要 Autoconf、Automake 或 Libtool。
`cmake --build` 只编译，`cmake --install` 执行安装；默认私有前缀无需 sudo。
可使用 Ninja：`cmake -S . -B build/ninja -G Ninja`（需先安装 Ninja）。
更换目标架构或工具链时使用新的构建目录。源文件清单显式维护在 CMakeLists.txt 中。
配置阶段执行必要的编译、链接探测，不执行目标程序或访问控制器。
**原生 CMake 是本派生仓库唯一受支持的构建入口。** 旧 Autotools 入口不再维护、不再支持。
`Makefile.am` 等上游文件仅作历史保留，不再添加 SDK 编译或安装规则。
`bootstrap` 会提示使用 CMake 并退出；绕过它重新生成 configure 后，也会明确拒绝旧构建流程。
CMake 不读取或解析这些旧文件。

## 4. 编译期板级配置

```bash
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release \
  -DPN7160_DEFAULT_I2C_DEVICE=/dev/i2c-1 \
  -DPN7160_DEFAULT_I2C_ADDRESS=0x28 \
  -DPN7160_DEFAULT_IRQ_GPIO=535 \
  -DPN7160_DEFAULT_VEN_GPIO=536 \
  -DPN7160_DEFAULT_DWL_REQ_GPIO=-1 \
  -DPN7160_DEFAULT_RESET_DELAY_US=10000
cmake --build build/native --parallel
cmake --install build/native
```

GPIO 数字仅为示例，需按目标内核调整。它们是 **Linux sysfs 全局 GPIO 编号**，
不是物理引脚号，也不执行 Python SDK 的 GPIO base 自动换算。

| CMake 参数 / 编译宏 | 默认值 | 含义 |
|---|---|---|
| `PN7160_DEFAULT_I2C_DEVICE` | `/dev/i2c-1` | I²C 设备绝对路径 |
| `PN7160_DEFAULT_I2C_ADDRESS` | `0x28` | 未移位的 7 位地址 |
| `PN7160_DEFAULT_IRQ_GPIO` | `23` | IRQ 全局 GPIO 编号 |
| `PN7160_DEFAULT_VEN_GPIO` | `24` | VEN 全局 GPIO 编号 |
| `PN7160_DEFAULT_DWL_REQ_GPIO` | `-1` | DWL 输出编号，-1 表示不使用 |
| `PN7160_DEFAULT_RESET_DELAY_US` | `10000` | 复位等待时间，微秒 |

默认值由库编译时确定；修改后需要重新编译。参数范围和 GPIO 冲突会在编译阶段检查，
运行时 setter 也会检查配置。设备是否存在、是否有权限以及接线是否正确，需要运行时验证。

板级默认值统一通过上述 CMake 缓存参数设置，再由生成头传入原生库。
仅给应用定义宏不会改变已经编译的 `.so`。

## 5. 交叉编译

先准备对应 Linux 交叉编译器及必要的目标 sysroot。仓库中的工具链文件不安装这些依赖。

```bash
# ARM64 / AArch64
cmake -S . -B build/aarch64 \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-aarch64.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/aarch64 --parallel
cmake --install build/aarch64

# ARM32 / hard-float
cmake -S . -B build/armhf \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-armhf.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/armhf --parallel
cmake --install build/armhf
```

交叉编译同样可以传入上一节的板级参数。其他 Linux SoC 使用自己的工具链文件，设置：

- `CMAKE_SYSTEM_NAME=Linux`、目标 C/C++ 编译器和目标处理器。
- 必要时设置 `CMAKE_SYSROOT`、最低 CPU 和浮点 ABI 编译选项。

本机编译不需要以上交叉参数。交叉编译时，目标架构由调用者选择，例如在 x86 主机生成 ARM64 程序；
宿主机无法推断用户希望生成哪个目标，所以需要工具链文件或等价的编译器设置。
工具链决定目标架构，结构体及默认宏决定板级参数；这两部分独立配置。

## 6. C/C++ 调用

```c
#include "pn7160_board_config.h"
#include "linux_nfc_api.h"

int main(void) {
    pn7160_config_t config;
    if (pn7160_get_default_config(&config, sizeof(config)) != PN7160_CONFIG_OK)
        return 1;

    /* 按需覆盖 config 字段。 */
    if (pn7160_set_config(&config) != PN7160_CONFIG_OK)
        return 1;
    if (doInitialize() != 0)
        return 1;

    /* NFC 业务操作。 */
    return doDeinitialize();
}
```

将上述内容保存为 `example.c`，可以使用构建结果编译：

```bash
cc example.c -Ibuild/native/install/include \
  -Lbuild/native/install/lib -lnfc_nci_linux -o example
LD_LIBRARY_PATH="$PWD/build/native/install/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" ./example
```

`pn7160_get_default_config()` 只返回库的默认值，不启用硬件，也不改变活动配置。
调用者仍须在初始化前调用 `pn7160_set_config()`，或者自行构造完整的 ABI v1 结构体。
setter 复制设备路径；getter 返回的路径属于库，不能释放。
初始化期间禁止修改配置；此实现使用进程内全局配置，当前不是多控制器实例接口。

| 状态码 | 含义 |
|---|---|
| `0` | 成功 |
| `-1` | 配置无效，或 getter 输出指针/容量无效 |
| `-2` | 配置正在使用 |
| `-3` | ABI 版本或结构体大小不兼容 |
| `-4` | 内部初始化检查发现配置尚未设置 |

这些是板级配置接口状态码，不等同于所有 NXP API 的返回值。

## 7. Python 调用

Python 通过 `ctypes` 调用同一套 C ABI，不依赖 CMake 运行。CMake 用于编译库和设定默认值。
已有 Python SDK 可以继续传入完整结构体，其 JSON 配置不会自动与编译期默认值合并。

以下示例只读取并设置配置，不执行硬件初始化：

```python
import ctypes as C
from pathlib import Path

class Config(C.Structure):
    _fields_ = [
        ("abi_version", C.c_uint32), ("struct_size", C.c_uint32),
        ("i2c_device", C.c_char_p), ("i2c_address", C.c_uint32),
        ("irq_gpio", C.c_int32), ("ven_gpio", C.c_int32),
        ("dwl_req_gpio", C.c_int32), ("reset_delay_us", C.c_uint32),
        ("reserved", C.c_uint32 * 4),
    ]

lib = C.CDLL(str(Path("build/native/install/lib/libnfc_nci_linux.so").resolve()))
lib.pn7160_config_abi_version.argtypes = []
lib.pn7160_config_abi_version.restype = C.c_uint32
if lib.pn7160_config_abi_version() != 1:
    raise RuntimeError("Unsupported board ABI")
lib.pn7160_get_default_config.argtypes = [C.POINTER(Config), C.c_uint32]
lib.pn7160_get_default_config.restype = C.c_int
lib.pn7160_set_config.argtypes = [C.POINTER(Config)]
lib.pn7160_set_config.restype = C.c_int

config = Config()
status = lib.pn7160_get_default_config(C.byref(config), C.sizeof(config))
if status != 0:
    raise RuntimeError(f"Get defaults failed: {status}")
# 在这里按目标板需要修改 config；保持结构体自然对齐，不设置 _pack_。
status = lib.pn7160_set_config(C.byref(config))
if status != 0:
    raise RuntimeError(f"Set config failed: {status}")
print(config.i2c_device.decode(), config.i2c_address, config.irq_gpio)
```

后续可绑定并调用 NXP 初始化及业务 API；完整的 Python 业务封装由独立 SDK 项目提供。

## 8. 运行配置与示例

配置搜索目录由构建配置决定，默认是 `<安装前缀>/etc`。
例如默认构建安装到 `build/native/install`，对应配置目录就是其下的 `etc`。
不存在固定的开发机路径或 `/usr/local/etc/` 常量；配置目录统一由 CMake 安装目录设置控制。
上游的 `/etc/` 回退逻辑仅在未显式指定运行目录时保留。

需要移动整个安装目录时，在**任何 NXP 配置读取和初始化之前**调用：

```c
if (pn7160_set_config_directory("/absolute/deployed/package/etc") != PN7160_CONFIG_OK)
    return 1;
```

Python 同样可以绑定 `pn7160_set_config_directory`，参数类型为 `c_char_p`，返回值为 `c_int`。
目录由调用者根据实际部署位置计算，不需要修改源码或重编译。
接口复制路径并补齐末尾斜杠；相对路径被拒绝。
由于 NXP 会缓存配置，首次读取后修改目录返回 BUSY，反初始化不会自动清除该缓存。
显式指定目录时不回退系统配置，以避免加载其他应用的参数。

使用本仓库结构体配置的 `/dev/i2c-*`、sysfs GPIO 后端时，
`libnfc-nxp.conf` 中需要设置 `NXP_TRANSPORT=0x02`（ALT_I2C）。
仓库保留的官方模板默认为 `0x00`（nxpnfc 内核驱动），结构体不会自动切换此选项。

默认私有安装时，配置文件已经由 `cmake --install` 放入相应目录，无需复制到系统目录。
使用 ALT_I2C 时，编辑安装后的配置文件即可：

```bash
sed -i 's/^NXP_TRANSPORT=.*/NXP_TRANSPORT=0x02/' build/native/install/etc/libnfc-nxp.conf
```

上述示例使用默认安装目录；自定义前缀或配置目录时替换为实际路径。
重新安装可能更新配置模板，部署程序应管理自己的配置文件。

其他 NXP 配置项（时钟、电源等）需按实际硬件核对。确保 I²C 及 GPIO 驱动可用、接线正确且调用进程有权限。
不要同时运行多个程序访问同一个 PN7160。

```bash
# 查看官方示例帮助，不初始化控制器
build/native/install/sbin/nfcDemoApp --help

# 接好硬件、完成配置后轮询；sudo 用于需要特权访问的开发环境
sudo env LD_LIBRARY_PATH="$PWD/build/native/install/lib" \
  "$PWD/build/native/install/sbin/nfcDemoApp" poll
```

`nfcDemoApp` 使用库的编译默认值。需要运行时配置的自有应用可使用 C ABI setter。

## 9. 上游与许可证

- [NXP 官方源码](https://github.com/NXPNFCLinux/linux_libnfc-nci)
- [NXP PN7160 Linux 应用说明 AN13287](https://www.nxp.com/doc/AN13287)
- 官方基线发布系列：NCI2.0-R1.1。

遵循仓库 `LICENSE.txt` 及各源码文件中的许可证声明。
跟进上游时保留官方历史并审阅适配差异，不重复应用已经合入的补丁。
