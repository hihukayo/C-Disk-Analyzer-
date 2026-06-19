# Disk Usage Analyzer / 磁盘空间分析器

A lightweight Windows command-line tool that recursively scans a directory and displays its space usage in a clean, tree-like visual format — sorted by size with bar charts and percentage breakdowns.

轻量的 Windows 命令行工具，递归扫描目录并以树状可视化形式展示磁盘空间占用——按大小排序，附带柱状图和百分比。

## Preview / 效果预览

```
Disk Usage Analyzer
Root: C:\Users\Example
-------------------------------------------------------------------------
[  2.34 GB] ██████████████████████████████████████████████████████████ 100.00%
[  1.20 GB] ████████████████████████████████████                     51.28%
  Documents/
[   512 MB] ████████████████████                                      21.88%
  Videos/
[   256 MB] ██████████                                                10.94%
  Downloads/
...
-------------------------------------------------------------------------
Total: 2.34 GB
```

## Features / 功能特点

- **Recursive scanning** — traverse all subdirectories automatically
- **Size-sorted output** — larger items displayed first
- **Visual bar chart** — relative size visualization with Unicode block characters (█)
- **Human-readable sizes** — B, KB, MB, GB, TB auto-scaled
- **Nested tree view** — hierarchical display with indentation
- **Terminal-aware layout** — bar width auto-adjusts to console width
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
