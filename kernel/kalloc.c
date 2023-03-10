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
} kmem;

uint64 npage, used;
char *ref_array;


void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);

  /* build ref_array */
  uint64 array_pg_cnt = npage * sizeof(char) / PGSIZE + (npage * sizeof(char) % PGSIZE > 0);
  printf("pages:%d", npage);
  struct run *r, *rn;
  /* allocate for sequential array at one time */
  acquire(&kmem.lock);
  r = kmem.freelist;
  while(array_pg_cnt){
    if(r){
      rn = r->next;
      memset((void *)r, 0, PGSIZE);
      ref_array = (char *)r;
      r = rn;
    }
    else
      printf("ref array: page not enough\n");
    array_pg_cnt--;
  }
  kmem.freelist = r;
  release(&kmem.lock);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  npage = 0;
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    npage++;
    kfree(p);
  }
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
  if(ref_array && ref_array[((uint64)pa>>12) - ((uint64)end>>12)]>0)
  {
    //printf("refused pa=%p . but proceed : %d\n", (uint64)pa, ref_array[((uint64)pa>>12) - ((uint64)end>>12)]);
    return;
  }
  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    ref_array[((uint64)r>>12) - ((uint64)end>>12)] = 1;
  }
  return (void*)r;
}

int add_ref_count(uint64 pa) {
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    printf("add: addr not correct: %p\n", pa);
  else
    ref_array[(pa>>12) - ((uint64)end>>12)]++;
  return 0;
}

int sub_ref_count(uint64 pa) {
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    printf("sub: addr not correct: %p\n", pa);
  else{
    //printf("pa: %p   %d -> %d\n",pa, ref_array[(pa>>12) - ((uint64)end>>12)],ref_array[(pa>>12) - ((uint64)end>>12)]-1);//
    ref_array[(pa>>12) - ((uint64)end>>12)]--;
  }
  return 0;
}

int get_ref_count(uint64 pa) {
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    printf("get: addr not correct: %p\n", pa);
  else{
    return ref_array[(pa>>12) - ((uint64)end>>12)];
  }
  return -1;
}