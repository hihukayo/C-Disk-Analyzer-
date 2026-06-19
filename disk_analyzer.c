#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <io.h>
#include <windows.h>
#include <fcntl.h>
#include <shellapi.h>

/* 固定大小字段宽度（如 "1024.00 TB" 需要 10 字符） */
#define SIZE_WIDTH 10

/* ---------- 宽字符缓存链表 ---------- */
typedef struct size_entry {
    wchar_t *path;
    long long size;
    struct size_entry *next;
} size_entry;

static size_entry *size_cache_head = NULL;

static void add_to_cache(const wchar_t *path, long long size) {
    size_entry *e = malloc(sizeof(size_entry));
    if (!e) return;
    e->path = _wcsdup(path);
    e->size = size;
    e->next = size_cache_head;
    size_cache_head = e;
}

static long long get_cached_size(const wchar_t *path) {
    size_entry *e = size_cache_head;
    while (e) {
        if (wcscmp(e->path, path) == 0) return e->size;
        e = e->next;
    }
    return -1;
}

static void free_cache(void) {
    while (size_cache_head) {
        size_entry *tmp = size_cache_head;
        size_cache_head = size_cache_head->next;
        free(tmp->path);
        free(tmp);
    }
}

/* 格式化大小为可读字符串 */
void format_size(long long bytes, wchar_t *buf, size_t bufsize) {
    const wchar_t *units[] = {L"B", L"KB", L"MB", L"GB", L"TB"};
    int unit_idx = 0;
    double size = (double)bytes;
    while (size >= 1024.0 && unit_idx < 4) {
        size /= 1024.0;
        unit_idx++;
    }
    _snwprintf(buf, bufsize, L"%.2f %s", size, units[unit_idx]);
}

/* 第一遍：递归计算并缓存所有目录大小 */
long long cache_sizes(const wchar_t *path) {
    wchar_t search_path[MAX_PATH];
    _snwprintf(search_path, MAX_PATH, L"%s\\*", path);

    struct _wfinddata_t fd;
    intptr_t handle = _wfindfirst(search_path, &fd);
    if (handle == -1) {
        fwprintf(stderr, L"Cannot open directory: %s\n", path);
        return 0;
    }

    long long total = 0;
    wchar_t fullpath[MAX_PATH];
    do {
        if (wcscmp(fd.name, L".") == 0 || wcscmp(fd.name, L"..") == 0) continue;
        _snwprintf(fullpath, MAX_PATH, L"%s\\%s", path, fd.name);
        if (fd.attrib & _A_SUBDIR) {
            total += cache_sizes(fullpath);
        } else {
            struct _stat64 st;
            if (_wstat64(fullpath, &st) == 0)
                total += st.st_size;
        }
    } while (_wfindnext(handle, &fd) == 0);
    _findclose(handle);

    add_to_cache(path, total);
    return total;
}

/* 子项链表 */
typedef struct item {
    wchar_t name[_MAX_FNAME + 1];
    int is_dir;
    long long size;
    struct item *next;
} item;

static item* insert_sorted(item *head, item *new_it) {
    if (!head || new_it->size > head->size) {
        new_it->next = head;
        return new_it;
    }
    item *curr = head;
    while (curr->next && curr->next->size >= new_it->size)
        curr = curr->next;
    new_it->next = curr->next;
    curr->next = new_it;
    return head;
}

/* 输出一行：第一行 [大小] 柱子 百分比，第二行 树状线 + 文件名 */
void print_item(const wchar_t *name, int is_dir,
                long long size, long long parent_total, int bar_width,
                const wchar_t *tree_prefix, const wchar_t *connector) {
    wchar_t size_str[32];
    format_size(size, size_str, 32);

    double ratio = (parent_total > 0) ? (double)size / parent_total : 1.0;
    int bar_len = (int)(ratio * bar_width);
    if (bar_len < 0) bar_len = 0;
    if (bar_len > bar_width) bar_len = bar_width;

    // 第一行：树状前缀 + 大小 + 柱子 + 百分比
    wprintf(L"%s %*s  ", tree_prefix, SIZE_WIDTH, size_str);
    for (int i = 0; i < bar_len; i++) wprintf(L"█");
    for (int i = bar_len; i < bar_width; i++) wprintf(L" ");
    wprintf(L" %6.2f%%\n", ratio * 100.0);

    // 第二行：树状前缀 + 连接线 + 文件名
    wprintf(L"%s%s%s%s\n", tree_prefix, connector, name, is_dir ? L"/" : L"");
}

/* 第二遍：递归输出子项，控制空行 */
void print_children(const wchar_t *parent_path, long long parent_total, int bar_width,
                    const wchar_t *tree_prefix) {
    wchar_t search_path[MAX_PATH];
    _snwprintf(search_path, MAX_PATH, L"%s\\*", parent_path);

    struct _wfinddata_t fd;
    intptr_t handle = _wfindfirst(search_path, &fd);
    if (handle == -1) return;

    item *items = NULL;
    wchar_t fullpath[MAX_PATH];
    do {
        if (wcscmp(fd.name, L".") == 0 || wcscmp(fd.name, L"..") == 0) continue;
        _snwprintf(fullpath, MAX_PATH, L"%s\\%s", parent_path, fd.name);

        long long size = 0;
        int is_dir = 0;
        if (fd.attrib & _A_SUBDIR) {
            size = get_cached_size(fullpath);
            if (size < 0) continue;
            is_dir = 1;
        } else {
            struct _stat64 st;
            if (_wstat64(fullpath, &st) == 0)
                size = st.st_size;
            else continue;
        }

        item *it = malloc(sizeof(item));
        wcsncpy(it->name, fd.name, _MAX_FNAME);
        it->name[_MAX_FNAME] = L'\0';
        it->is_dir = is_dir;
        it->size = size;
        it->next = NULL;
        items = insert_sorted(items, it);
    } while (_wfindnext(handle, &fd) == 0);
    _findclose(handle);

    item *curr = items;
    while (curr) {
        const wchar_t *connector = (curr->next != NULL) ? L"├── " : L"└── ";
        print_item(curr->name, curr->is_dir,
                   curr->size, parent_total, bar_width,
                   tree_prefix, connector);

        if (curr->is_dir) {
            const wchar_t *suffix = (curr->next != NULL) ? L"│   " : L"    ";
            wchar_t new_prefix[MAX_PATH];
            _snwprintf(new_prefix, MAX_PATH, L"%s%s", tree_prefix, suffix);

            _snwprintf(fullpath, MAX_PATH, L"%s\\%s", parent_path, curr->name);
            print_children(fullpath, curr->size, bar_width, new_prefix);
        }

        // 有内容的目录后空一行分隔（带上树状前缀保持线条连续）
        if (curr->is_dir && curr->size > 0 && curr->next != NULL)
            wprintf(L"%s\n", tree_prefix);

        item *tmp = curr;
        curr = curr->next;
        free(tmp);
    }
}

int main(void) {
    // 设置 stdout 为 Unicode 输出模式
    _setmode(_fileno(stdout), _O_U16TEXT);

    // 获取 Unicode 命令行参数
    int argc;
    wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    const wchar_t *start_path = L".";
    if (argc > 1) start_path = argv[1];

    wchar_t real_path[MAX_PATH];
    if (_wfullpath(real_path, start_path, MAX_PATH) == NULL) {
        fwprintf(stderr, L"Failed to resolve path.\n");
        LocalFree(argv);
        return EXIT_FAILURE;
    }

    // 自动计算柱子宽度（终端列数 - 左边固定部分 - 安全余量）
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns = 80;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        columns = csbi.dwSize.X;
    int bar_width = columns - 30;  // 30 为 [大小] + 百分比 + 缩进留空
    if (bar_width < 10) bar_width = 10;
    if (bar_width > 60) bar_width = 60;

    // 打印头部
    wprintf(L"\nDisk Usage Analyzer\n");
    wprintf(L"Root: %s\n", real_path);
    wprintf(L"-------------------------------------------------------------------------\n");

    // 计算总大小（首次遍历）
    long long total = cache_sizes(real_path);

    // 直接输出子项（不输出根目录行）
    print_children(real_path, total, bar_width, L"│   ");

    // 底部信息
    wprintf(L"-------------------------------------------------------------------------\n");
    wchar_t total_str[32];
    format_size(total, total_str, 32);
    wprintf(L"Total: %s\n\n", total_str);

    free_cache();
    LocalFree(argv);
    return 0;
}