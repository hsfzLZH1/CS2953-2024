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

// fix number of buckets
#define NBUK 13

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers with hash value i
  // through prev/next.
  struct buf head[NBUK];

  // a lock for each linked list
  struct spinlock lockll[NBUK];
} bcache;

// hash function
int
buf_hshval(int dev,int blknum)
{
  return (dev+blknum)%NBUK;
}

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock,"bcache");// init global lock

  for(int i=0;i<NBUK;i++)// for each bucket
  {
    initlock(&bcache.lockll[i], "bcache");// init lock for each linked list

    bcache.head[i].prev=bcache.head[i].next=&bcache.head[i];// init linked list
  }

  // add all buffer to linked list[0]
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head[0].next;
    b->prev = &bcache.head[0];
    initsleeplock(&b->lock, "buffer");
    bcache.head[0].next->prev = b;
    bcache.head[0].next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  int v=buf_hshval(dev,blockno);

  // Is the block already cached?
  acquire(&bcache.lockll[v]);
  for(b = bcache.head[v].next; b!=&bcache.head[v] ; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lockll[v]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  release(&bcache.lockll[v]);

  // Not cached.
  // find an cached block not in use and evict
  for(int j=0;j<NBUK;j++)
  {
    int i=(v+j)%NBUK;// find not-in-use block in linked list[i]

    acquire(&bcache.lockll[i]);
    for(b=bcache.head[i].next;b!=&bcache.head[i];b=b->next)
    {
      if(b->refcnt==0)// evict b
      {
        b->dev=dev;
        b->blockno=blockno;
        b->valid=0;
        b->refcnt=1;

        if(v!=i)// hash to different bucket
        {
          // remove b from linked list[i]
          b->prev->next=b->next;
          b->next->prev=b->prev;
          release(&bcache.lockll[i]);

          // add b to linked list[v]
          acquire(&bcache.lockll[v]);
          b->next=bcache.head[v].next;
          b->prev=&bcache.head[v];
          bcache.head[v].next->prev=b;
          bcache.head[v].next=b;
          release(&bcache.lockll[v]);
        }
        // hash to the same bucket, no need to modify linked list
        else release(&bcache.lockll[i]);

        acquiresleep(&b->lock);
        return b;
      }
    }
    release(&bcache.lockll[i]);
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

  int v=buf_hshval(b->dev,b->blockno);

  releasesleep(&b->lock);

  // no need to maintain LRU list
  acquire(&bcache.lockll[v]);
  b->refcnt--;
  release(&bcache.lockll[v]);
}

// Lab4: no global lock?
void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}
