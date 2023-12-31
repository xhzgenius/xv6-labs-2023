# Lab 9: mmap

本次lab代码思路来源于https://zhuanlan.zhihu.com/p/599200259，在此表示感谢。

### 避坑点

记得在 `trap.c` 中添加如下头文件：

```c
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
```

不然会提示不允许使用不完整的类型。
