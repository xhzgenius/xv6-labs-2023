# Lab8: File System

### Part 1: Large files

中规中矩，没什么特别的点，两次查表就行，如果没有就创建一个。

记得使用 `brelse(bp);` 释放已经用完的块。

### Part 2: Symbolic Links

要修改的杂七杂八的地方一览：

- `fcntl.h` 中加上 `#define O_NOFOLLOW 0x800`
- `stat.h` 中加上 `#define T_SYMLINK 4`
- `syscall.h` 中加上 `#define SYS_symlink 22`
- `syscall.c` 中加上 `extern uint64 sys_symlink(void);` 并且在那个数组里加上 `[SYS_symlink] sys_symlink,`
- `sysfile.c` 中加上 `sys_symlink` 的函数体；
- `usys.pl` 中加上 `entry("symlink");`
- `user.h` 中加上 `int symlink(const char*, const char*);` 的声明；
- `Makefile` 中加上 `$U/_symlinktest\` ，很坑，不加上不编译 `symlinktest` 测试程序，`make qemu` 还不报错。

应该就是这些，能通过编译就行。

接下来是实现过程：

首先实现 `sys_symlink` 的函数体；然后修改 `sys_open` 函数，使得其支持 `symlink `文件类型和 `O_NOFOLLOW` 的打开模式。

#### 避坑指南

`sys_open` 函数中，记得在读完symlink文件之后把它unlockput释放掉！！！要不然后续没法访问它了！！！

`create` 函数中，对于已存在的文件路径，貌似是直接overwrite掉这个inode。在判断条件中要加上文件类型为T_SYMLINK的情况，要不然并发情况下没法重复create，会失败。
