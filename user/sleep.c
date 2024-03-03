#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc < 2){
    fprintf(2, "Error: pass an argument\n");
    exit(1);
  }

  int n=atoi(argv[1]);
  //pause for n clock ticks
  sleep(n);

  exit(0);
}
