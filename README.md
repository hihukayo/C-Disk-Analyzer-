# Disk Usage Analyzer / 磁盘空间分析器

A lightweight Windows command-line tool that recursively scans a directory and displays its space usage in a clean tree view — sorted by size with percentage breakdowns.

轻量的 Windows 命令行工具，递归扫描目录并以树状可视化形式展示磁盘空间占用——按大小排序，附带百分比。

## Preview / 效果预览

```
Disk Usage Analyzer
Root: C:\Users\Example
-------------------------------------------------------------------------
│   ├── Videos/      1.20 GB      51.28%
│   │   ├── movie.mkv    800.00 MB      34.19%
│   │   └── clip.mp4     400.00 MB      17.09%
│   
│   ├── Documents/    512.00 MB      21.88%
│   │   └── report.pdf     512.00 MB      21.88%
│   
│   └── Downloads/    256.00 MB      10.94%
│       └── setup.exe      256.00 MB      10.94%
-------------------------------------------------------------------------
Total: 2.34 GB
```

## Features / 功能特点

- **Recursive scanning** — traverse all subdirectories automatically
- **Size-sorted output** — larger items displayed first
- **Tree structure** — hierarchical view with continuous `│` / `├──` / `└──` connectors
- **Human-readable sizes** — B, KB, MB, GB, TB auto-scaled
- **Relative percentages** — each item's share of the total scanned size
- **Cached results** — directory sizes computed once and reused

## Usage / 使用方法

```cmd
disk-analyzer.exe [directory]
```

- If no directory is specified, scans the current working directory.
- 不指定目录则扫描当前工作目录。

### Examples / 示例

```cmd
disk-analyzer.exe
disk-analyzer.exe C:\Users
disk-analyzer.exe "C:\Program Files"
```

## Build / 编译

Compile with any C89/C99 compatible compiler on Windows:

```cmd
cl disk_analyzer.c
```

Or with MinGW:

```cmd
gcc disk_analyzer.c -o disk_analyzer.exe
```

## Requirements / 系统要求

- **OS:** Windows (uses Win32 API for Unicode console output)
- **Compiler:** MSVC or MinGW
- No external dependencies / 无外部依赖

## License / 许可

MIT
