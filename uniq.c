#include "types.h"
#include "user.h"

#define LINE_SIZE 512

char buf1[LINE_SIZE] = "";
char buf2[LINE_SIZE] = "";
char *prev_line = buf1;
char *line = buf2;

void
uniq(int fd)
{
  int n;

  while((n = readline(fd, line, LINE_SIZE)) > 0) {
    // If the new line does not match the prev line...
    if(strcmp(prev_line, line)){
      // ...write it!
      if (write(1, line, n) != n) {
        printf(1, "uniq: write error\n");
        exit();
      }
    }
    // Update prev line
    strswp(&prev_line, &line);
  }
  if(n < 0){
    printf(1, "uniq: read error\n");
    exit();
  }
}

int
main(int argc, char *argv[])
{
  int fd, i;

  if(argc <= 1) { exit(); }

  for(i = 1; i < argc; i++){
    if((fd = open(argv[i], 0)) < 0){
      printf(1, "uniq: cannot open %s\n", argv[i]);
      exit();
    }
    uniq(fd);
    close(fd);
  }
  exit();
}