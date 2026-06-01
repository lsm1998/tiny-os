# tiny-os

tiny-os 是一个基于 C 的操作系统内核，旨在提供一个简单、轻量级的操作系统环境，适用于嵌入式系统和教育用途。提供了基本的内核功能，如任务调度、内存管理和设备驱动程序。

## 环境搭建 (MacOS ARM64)

- 安装 qemu
```bash
brew install qemu
```

- 安装交叉编译工具链
```bash
brew install x86_64-elf-gcc
brew install x86_64-elf-gdb
```

## 环境搭建 （Linux amd64）

以 Ubuntu / Debian 为例：

- 安装基础编译工具（gcc、make 等）
```bash
sudo apt update
sudo apt install -y build-essential cmake
```

- 安装 32 位编译支持（项目使用 `-m32` 编译 32 位内核）
```bash
sudo apt install -y gcc-multilib
```

- 安装 QEMU 模拟器（运行内核）
```bash
sudo apt install -y qemu-system-x86
```

- 安装 GDB 调试器
```bash
sudo apt install -y gdb
```

> **说明**：Linux 下 CMakeLists.txt 中 `TOOL_PREFIX` 保持为空，直接使用原生 GCC（`gcc`、`ld`、`objcopy` 等）编译。如需使用 VS Code 调试，请将 `.vscode/launch.json` 中的 `miDebuggerPath` 由 `x86_64-elf-gdb` 改为 `gdb`。

### 编译与运行

```bash
# 编译内核
make build

# 使用 QEMU 启动调试（GDB stub 监听 127.0.0.1:1234）
bash script/qemu-debug-linux.sh
```

> `img-write-linux.sh` 中使用了 `sudo mount` 挂载磁盘镜像，首次运行可能需要输入密码。


