#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "kernel/sysinfo.h" // Added by XHZ, for struct sysinfo

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Below are added by XHZ. 
// The sys_xxx functions have no arguments, and takes user's arguments from argint(). 
uint64 sys_trace(void)
{
  argint(0, &myproc()->trace_mask); // Store the trace mask to the struct proc of current process. 
  return 0;
}

uint64 sys_sysinfo(void)
{
  uint64 dst_virtual;
  argaddr(0, &dst_virtual);
  struct sysinfo info;
  info.freemem = count_freemem();
  info.nproc = count_procs();
  // We need to copy data to a given virtual address, so copyout() is needed. 
  if(copyout(myproc()->pagetable, dst_virtual, (char *)&info, sizeof(struct sysinfo)) < 0)
    return -1;
  return 0;
}