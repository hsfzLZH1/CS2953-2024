#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p0[2];//the pipe from parent to child
  int p1[2];//the pipe from child to parent
  char buf[4]="a";//buffer
  pipe(p0);
  pipe(p1);

  if(fork()==0)//child
  {
    close(p0[1]);
    close(p1[0]);//the child only use 2 fd
    read(p0[0],buf,1);
    printf("%d: received ping\n",getpid());
    write(p1[1],buf,1);
  }
  else //parent
  {
    close(p0[0]);
    close(p1[1]);//the parent only use 2 fd
    write(p0[1],buf,1);//send a byte to child
    wait((int*)0);
    read(p1[0],buf,1);//read the byte from the child
    printf("%d: received pong\n",getpid());
  }

  exit(0);
}
