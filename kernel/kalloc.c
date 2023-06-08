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

struct kmem {
  struct spinlock lock;
  struct run *freelist;
  uint freenum;
};
struct kmem kmems[NCPU];

void
kinit()
{
  // char lockname[16];
  for(int i = 0; i < NCPU; ++i) {
    // snprintf(lockname, sizeof(lockname), "kmem_%d", i);
    // initlock(&kmems[i].lock, lockname);
    initlock(&kmems[i].lock, "kmem");
    kmems[i].freenum = 0;
  }
  freerange(end, (void*)PHYSTOP);
  printf("kinit succeed\n");
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    kfree(p);
  }
}

// Free the page of physical memory pointed at by v,
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

  push_off();
  int mycpu_id = cpuid();
  acquire(&kmems[mycpu_id].lock);
  r->next = kmems[mycpu_id].freelist;
  kmems[mycpu_id].freelist = r;
  kmems[mycpu_id].freenum++;
  release(&kmems[mycpu_id].lock);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int mycpu_id = cpuid();
  acquire(&kmems[mycpu_id].lock);
  r = kmems[mycpu_id].freelist;
  if(r) {
    kmems[mycpu_id].freelist = r->next;
    kmems[mycpu_id].freenum--;
  } else { // steal
    int max_freenum = 0, max_id = mycpu_id, i = 0;
    for(i = 0; i < NCPU; ++i) {
      if (i == mycpu_id) continue;
      acquire(&kmems[i].lock);
      if (kmems[i].freenum > max_freenum) {
        if (max_id != mycpu_id) {
          release(&kmems[max_id].lock);
        }
        max_id = i;
        max_freenum = kmems[i].freenum;
      } else {
        release(&kmems[i].lock);
      }
    }

    if (max_freenum == 0) {
      release(&kmems[mycpu_id].lock);
      pop_off();
      return (void*)0;
    }
    // printf("cpu %d steal %d from cpu %d, current is %d\n", mycpu_id, max_freenum, max_id, kmems[mycpu_id].freenum);

    struct run* new_tail = kmems[max_id].freelist;
    kmems[mycpu_id].freelist = new_tail;
    int new_count = 1;
    for(; new_count <= max_freenum/2; new_count++) {
      new_tail = new_tail->next;
    }
    kmems[mycpu_id].freenum = new_count;
    kmems[max_id].freenum -= new_count;
    kmems[max_id].freelist = new_tail->next;
    new_tail->next = 0;
    release(&kmems[max_id].lock);
    r = kmems[mycpu_id].freelist;
    kmems[mycpu_id].freelist = r->next;
    kmems[mycpu_id].freenum--;
    // printf("after cpu %d freenum %d; cpu %d, freenum %d\n", mycpu_id, kmems[mycpu_id].freenum, max_id, kmems[mycpu_id].freenum);
  }
  release(&kmems[mycpu_id].lock);
  pop_off();

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
