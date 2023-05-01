#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
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
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
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


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  uint64 va; // the starting virtual address of the first user page to check.
  int n;    // the number of pages to check
  uint64 result; // virtual address where we store the result
  if (argaddr(0, &va) < 0) 
    return -1;
  if(va >= MAXVA) // invalid virtual address
    return -1;
  if (argint(1, &n) < 0)
    return -1;
  if (n > 32) // max number of scanned pages
    return -1;
  if (argaddr(2, &result) < 0)
    return -1;
  
  pagetable_t pagetable = myproc()->pagetable;
  unsigned int abits = 0;
  // vmprint(pagetable);
  for(int i = 0; i < n; ++i) {
    pagetable_t tmp_pagetable = pagetable;
    for(int level = 2; level > 0; level--) {
      pte_t *pte = &tmp_pagetable[PX(level, va)];
      if(*pte & PTE_V) {
        tmp_pagetable = (pagetable_t)PTE2PA(*pte);
      } else {
        return -1; // invalid memory
      }
    }
    pte_t *pte = &tmp_pagetable[PX(0, va)];
    if ((*pte) & PTE_A) {
      abits |= (1<<i);
      (*pte) &= (~PTE_A);
    }
    va += PGSIZE;
  }
  if(copyout(pagetable, result, (char*)&abits, sizeof(abits)) < 0)
    return -1;
  // printf("[debug] pgaccess:\t%p\t%d\t%d\n", va, n, result);
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
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
