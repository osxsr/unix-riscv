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

//#define LAB_PGTBL
#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  int bitmap=0;
  int vaddr;
  int npage, uaddr;
  argint(0, &vaddr);
  argint(1, &npage);
  argint(2, &uaddr);
  if(npage > 32)
    panic("pgaccess : once only for 32 pages\n");


  pte_t *pte;
  pagetable_t pg = myproc()->pagetable;
  for(int i=0; i<npage; i++) {
    if((pte = walk(pg, vaddr+i*PGSIZE, 0)) == 0) {
      panic("pgaccess : page not found\n");
      return -1;
    }
    if(*pte & PTE_A) {
      bitmap |= 1<<i;
      *pte = *pte & (~PTE_A);
    }
  }
  //vmprint(myproc()->pagetable);
  //printf("\n\n\n\n");

  copyout(myproc()->pagetable, uaddr, (char *)&bitmap, 4);
  return 0;
}
#endif

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