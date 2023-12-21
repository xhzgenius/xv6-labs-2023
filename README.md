# Lab 7: locks

##### 个人提示：
永远不要在一个函数里调用多次mycpu()，由于进程会被调度，你可能会得到多个不同的结果。如果你用mycpu()的值来acquire或者release锁，那么这两个可能不是同一把锁，从而就panic了。

