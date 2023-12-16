// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmems[NCPU];

void
kinit()
{
  for(int id = 0;id<NCPU;id++)
  {
    char name_buf[16] = {0};
    snprintf(name_buf, sizeof(name_buf), "kmem_lock_%d", id);
    printf("kmem lock name: %s\n", name_buf);
    initlock(&kmems[id].lock, name_buf);
  }
  printf("kinit(): initlock() complete. \n");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
  printf("freerange() at CPU %d: Complete. \n", cpuid());
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  int cpu_id = cpuid(); // DO NOT call cpuid() several times: the results may be different!!! 
  acquire(&kmems[cpu_id].lock);
  r->next = kmems[cpu_id].freelist;
  kmems[cpu_id].freelist = r;
  release(&kmems[cpu_id].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  // push_off(); // Avoid the swith of CPU
  struct run *r;
  int cpu_id = cpuid(); // DO NOT call cpuid() several times: the results may be different!!! 

  acquire(&kmems[cpu_id].lock);
  r = kmems[cpu_id].freelist;
  if(r)
    kmems[cpu_id].freelist = r->next;
  release(&kmems[cpu_id].lock);
  for(int id = 0;!r && id<NCPU;id++)
  {
    if(id==cpu_id) continue;
    acquire(&kmems[id].lock);
    r = kmems[id].freelist;
    if(r)
      kmems[id].freelist = r->next;
    release(&kmems[id].lock);
  }

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  else
    printf("kalloc() at CPU %d: Cannot allocate memory. \n", cpu_id);
  
  // pop_off();
  return (void*)r;
}
