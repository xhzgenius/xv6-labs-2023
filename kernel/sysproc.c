#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

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


// #ifdef LAB_PGTBL
int
sys_pgaccess()
{
  // lab pgtbl: your code here.
  // Added by XHZ
  uint64 start_va;
  argaddr(0, &start_va);
  int page_num;
  argint(1, &page_num);
  uint64 dst_ptr;
  argaddr(2, &dst_ptr);
  uint64 buffer = 0; // Temporarily store here
  pagetable_t pagetable = myproc()->pagetable;

  for(int i = 0;i<page_num;i++)
  {
    pte_t *pte_ptr = walk(pagetable, start_va+i*PGSIZE, page_num);
    int is_accessed = (*pte_ptr & PTE_A);
    if(is_accessed)
    {
      buffer |= (1<<i);
      *pte_ptr ^= PTE_A;
    }
  }
  printf("Accessed pages buffer: %p\n", buffer);
  copyout(pagetable, dst_ptr, (char *)(&buffer), sizeof(buffer));

  return 0;
}
// #endif

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
