#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  int i;

  if(argc < 2){
    printf(2, "usage: kill pid...\n");
    xv6_exit();
  }
  for(i=1; i<argc; i++)
    xv6_kill(atoi(argv[i]));
  xv6_exit();
}
