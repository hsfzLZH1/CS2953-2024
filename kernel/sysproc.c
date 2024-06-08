#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

#include "sysinfo.h" // Lab1 sysinfo

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
#ifdef LAB_TRAPS
  backtrace();
#endif
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
  uint64 base,mask;
  int len;
  argaddr(0,&base);
  argint(1,&len);
  argaddr(2,&mask);

  // scan at most 32*32 pages
  if(len>1024)return -1;

  uint32 buf[32];
  for(int i=0;i<32;i++)buf[i]=0;

  for(int i=0;i<len;i++)
  {
    int va=base+i*PGSIZE;
    uint32*pte=(uint32*)walk(myproc()->pagetable,va,0);
    buf[i>>5]|=((((*pte)&PTE_A)>>6)<<(i&31));// check PTE_A

    (*pte)=(*pte|PTE_A)^(PTE_A);// set PTE_A to 0
  }

  // the number of bytes is ceil(len/8)
  copyout(myproc()->pagetable,mask,(char*)buf,(len+7)/8);

  return len;
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

// remember the trace-mask in current process 
uint64
sys_trace(void)
{
  int mask;
  argint(0,&mask);
  myproc()->trace_mask=mask;
  return 0;
}

// collect freemem & nproc about the running system
uint64
sys_sysinfo(void)
{
  uint64 vaddr; // user pointer to struct sysinfo
  struct sysinfo info;

  info.freemem=count_free();
  info.nproc=count_proc();

  argaddr(0,&vaddr);
  if(copyout(myproc()->pagetable,vaddr,(char*)&info,sizeof(info))<0)
    return -1;
  return 0;
}

uint64
sys_sigalarm(void)
{
  int n;
  uint64 fnptr;

  argint(0,&n);
  argaddr(1,&fnptr);

  myproc()->alarm_period=n;
  myproc()->alarm_handler=fnptr;
  myproc()->ticks_passed=0;

  return 0;
}

uint64
sys_sigreturn(void)
{
  // copy alarm_frame back to trapframe
  memmove(myproc()->trapframe,myproc()->alarm_frame,sizeof(struct trapframe));

  myproc()->ticks_passed=0;
  myproc()->is_handling=0;
  return myproc()->trapframe->a0;
}

uint64
sys_mmap(void)
{
  size_t length;
  int prot,flags,fd;
  struct file*f;
  argaddr(1,&length);
  argint(2,&prot);
  argint(3,&flags);
  argint(4,&fd);
  f=myproc()->ofile[fd];

  // find an unused region in the process's address space to map the file
  // the initial address is page-aligned
  uint64 addr=PGROUNDUP(myproc()->sz);
  uint64 MAXSZ=255ull*512ull*512ull*PGSIZE;
  // ensure not overflow pagetable
  if(length>=MAXSZ||addr+length>=MAXSZ)return 0xffffffffffffffff;
  myproc()->sz=addr+length;

  // find an vma to record mmap
  int t=-1;
  for(int k=0;k<16;k++)
    if(myproc()->vmalist[k].length==0)
      {t=k;break;}
  if(t==-1)// vma allocation failed
    return 0xffffffffffffffff;

  myproc()->vmalist[t].addr=addr;
  myproc()->vmalist[t].length=length;
  myproc()->vmalist[t].prot=prot;
  myproc()->vmalist[t].flags=flags;
  // record mapped file
  myproc()->vmalist[t].of=f;
  filedup(f);
  return addr; 
}

uint64
sys_munmap(void)
{
  return -1;
}
