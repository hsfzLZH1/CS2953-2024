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
  // one spinlock and runlist for each CPU
  struct spinlock lock[NCPU];
  struct run *freelist[NCPU];
} kmem;

void
kinit()
{
  // init lock for each CPU
  for(int i=0;i<NCPU;i++)
    initlock(&(kmem.lock[i]), "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
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

  push_off();// intr off
  int c=cpuid();// get current core number
  pop_off();// intr on

  acquire(&kmem.lock[c]);
  r->next = kmem.freelist[c];
  kmem.freelist[c] = r;
  release(&kmem.lock[c]);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();// intr off
  int c=cpuid();// get current core number
  pop_off();// intr on

  acquire(&kmem.lock[c]);
  r = kmem.freelist[c];
  if(r)
    kmem.freelist[c] = r->next;
  release(&kmem.lock[c]);
  // try to steal if r==0 here(no free page for this CPU)
  // find another CPU with nonempty freelist, and steal
  if(!r)
  {
    for(int i=0;i<NCPU;i++)
      if(c!=i)
      {
        acquire(&kmem.lock[i]);
        if(kmem.freelist[i])
        {
          // remove from freelist of CPU[i]
          r=kmem.freelist[i];
          kmem.freelist[i]=r->next;
        }
        release(&kmem.lock[i]);

        if(r)break;// success stealing
      }
  }

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk

  return (void*)r;
}

// Collect amount of free memory.
// The length of run is the free page number.
// UPD in Lab4: freelist for every CPU
int
count_free(void)
{
  int cnt=0;
  struct run*r;

  for(int i=0;i<NCPU;i++)
  {
    for(r=kmem.freelist[i];r;r=r->next)
      cnt++;
  }
  return cnt*PGSIZE;
}
