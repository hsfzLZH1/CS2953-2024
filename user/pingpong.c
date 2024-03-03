#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p[2];//the pipe
  char buf[4];//buffer
  pipe(p);

  if(fork()==0)//child
  {
    read(p[0],buf,1);
    printf("%d: received ping\n",getpid());
    write(p[1],buf,1);
  }
  else //parent
  {
    write(p[1],buf,1);//send a byte to child
    wait((int*)0);
    read(p[0],buf,1);//read the byte from the child
    printf("%d: received pong\n",getpid());
  }

  exit(0);
}
