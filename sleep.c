#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int ticks;

  if(argc < 2){
    printf(2, "Usage: sleep ticks\n");
    exit();
  }
  // Sleep for number of ticks
  ticks = atoi(argv[1]);
  sleep(ticks);
  exit();
}