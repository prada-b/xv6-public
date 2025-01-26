#include "types.h"
#include "user.h"

void
print_line(char *line, int count, bool opt_count)
{
  // Print count
    if (opt_count) {
      printf(1, "      %d %", count);
      }
      // Print line
      printf(1, "%s\n", line);
}

void
uniq(int fd, bool opt_count, bool opt_repeated, bool opt_ignore_case)
{
  int count;
  bool reading;

  #define LINE_SIZE 512
  char buf1[LINE_SIZE] = "";
  char buf2[LINE_SIZE] = "";
  char *prev_line = buf1;
  char *line = buf2;
  
  count = 1; // INIT
  reading = readline(fd, prev_line, LINE_SIZE); // INIT
  while(reading) {
    // Next line
    reading = readline(fd, line, LINE_SIZE);
   
    // If the new line matches the prev line...
    if((opt_ignore_case && strcmp_casefold(prev_line, line)) || strcmp(prev_line, line)){
      // Increment count
      count++;
    }
    else{
      // Write it!
      if (!opt_repeated || count > 1){
        print_line(prev_line, count, opt_count);
      }
      // Reset count
      count = 1;
    }

    // Update prev line
    strswp(&prev_line, &line);
  }
}

int
main(int argc, char *argv[])
{
  int fd, i;
  bool opt_count = false;
  bool opt_repeated = false;
  bool opt_ignore_case = false;

  for(i=1; i<argc; i++){
    if(argv[i][0] == '-'){
      for (int j=1; argv[i][j] != '\0'; j++){
        switch(argv[i][j]){
        case 'c':
            opt_count = true;
        break;
        case 'd':
            opt_repeated = true;
        break;
        case 'i':
            opt_ignore_case = true;
        break;
        default:
            printf(1, "undefined argument %c\n", argv[i][1]);
            break;
        }
      }
    }
    else{
      break;
    }
  }

  if(i >= argc) { exit(); }

  for(; i < argc; i++){
    if((fd = open(argv[i], 0)) < 0){
      printf(1, "uniq: cannot open %s\n", argv[i]);
      exit();
    }
    uniq(fd, opt_count, opt_repeated, opt_ignore_case);
    close(fd);
  }
  exit();
}