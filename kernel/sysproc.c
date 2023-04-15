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


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64  pageaddr;           // virtual address of the first page to check       
  uint64  usrbuf;             // userspace buffer to store the bitmask
  int     len;                // number of pages to check incl. the first one
  unsigned int tempbuf;       // temp buffer to construct the bitmask
  pte_t   *pte;               // page table entry corresponding to a va

  argaddr(0, &pageaddr);
  argint(1, &len);
  argaddr(2, &usrbuf);

  // Find out which pages have been accessed and construct the bitmask
  tempbuf = 0;
  for (int i = 0; i < len; i++)   // Iterate through len # of pages
  {
    // Find the pte that corresponds to the i'th page's virtual address
    pte = walk(myproc()->pagetable, pageaddr + (PGSIZE * i), 0);

    // If the pte has the access bit set, set the i'th bit in the bitmask
    // and then clear the access bit
    if (*pte & PTE_A)
    {
      tempbuf = (1 << i) | tempbuf;
      *pte = *pte & ~PTE_A;
    }
  }
  
  copyout(myproc()->pagetable, usrbuf, (char *)&tempbuf, sizeof(tempbuf));
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