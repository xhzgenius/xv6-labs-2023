// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#ifndef BCACHE_NUM
#define BCACHE_NUM 11 // Added by XHZ
#endif
struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head;
} bcaches[BCACHE_NUM]; // Hash table

void
binit(void)
{
  struct buf *b;

  for(int id = 0;id<BCACHE_NUM;id++)
  {
    char name_buf[16] = {0};
    snprintf(name_buf, sizeof(name_buf), "bcache_lock_%d", id);
    printf("kmem lock name: %s\n", name_buf);
    initlock(&bcaches[id].lock, name_buf);

    // Create linked list of buffers
    bcaches[id].head.prev = &bcaches[id].head;
    bcaches[id].head.next = &bcaches[id].head;
    for(b = bcaches[id].buf; b < bcaches[id].buf+NBUF; b++){
      b->next = bcaches[id].head.next;
      b->prev = &bcaches[id].head;
      initsleeplock(&b->lock, "buffer");
      bcaches[id].head.next->prev = b;
      bcaches[id].head.next = b;
    }
  }
  printf("binit(): Complete. \n");

}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int id = blockno%BCACHE_NUM;

  acquire(&bcaches[id].lock);

  // Is the block already cached?
  for(b = bcaches[id].head.next; b != &bcaches[id].head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcaches[id].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(b = bcaches[id].head.prev; b != &bcaches[id].head; b = b->prev){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcaches[id].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int id = b->blockno % BCACHE_NUM;
  acquire(&bcaches[id].lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcaches[id].head.next;
    b->prev = &bcaches[id].head;
    bcaches[id].head.next->prev = b;
    bcaches[id].head.next = b;
  }
  
  release(&bcaches[id].lock);
}

void
bpin(struct buf *b) {
  int id = b->blockno % BCACHE_NUM;
  acquire(&bcaches[id].lock);
  b->refcnt++;
  release(&bcaches[id].lock);
}

void
bunpin(struct buf *b) {
  int id = b->blockno % BCACHE_NUM;
  acquire(&bcaches[id].lock);
  b->refcnt--;
  release(&bcaches[id].lock);
}


