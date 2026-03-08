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