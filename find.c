#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

#define PATH_SIZE 512
char curr_path[PATH_SIZE];
char curr_name[DIRSIZ];

void
find_hlpr(char *eop, char *name)
{
  int fd;
  struct dirent de;
  struct stat st;
    
  // Stat path
  if(stat(curr_path, &st) < 0){
    printf(2, "find: cannot stat %s\n", curr_path);
    return;
  }

  switch(st.type){
  case T_FILE:
    // Print?
    if (strcmp(curr_name, name) == 0)
    {
      printf(1, "%s\n", curr_path);
    }
    break;

  case T_DIR:
    // Mark as directory
    *eop++ = '/';
    *eop = '\0';

    // Print?
    if (strcmp(curr_name, name) == 0)
    {
      printf(1, "%s\n", curr_path);
    }
      
    // Open directory
    if((fd = open(curr_path, 0)) < 0){
      printf(2, "find: cannot open %s\n", curr_path);
      return;
    }

    // Read all directory entries
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0 ||
         strcmp(de.name, ".") == 0 ||
         strcmp(de.name, "..") == 0)
      {
        continue;
      }
        
      // Place name at end of current path
      if(eop - curr_path + strlen(de.name) + 1 > PATH_SIZE){
        printf(1, "find: path too long\n");
        close(fd);
        return;
      }
      strcpy(eop, de.name);

      // Update curr_name
      strcpy(curr_name, de.name);
      
      // Recurse
      find_hlpr(eop + strlen(de.name), name);
    }
    
    close(fd);
    break;
  }

  return;
}

void
find(char *path, char *name)
{
  char *eop = curr_path;
  
  // Copy path to buffer
  if(strlen(path) + 1 > sizeof(curr_path)){
    printf(1, "find: path too long\n");
  }
  strcpy(curr_path, path);
  eop += strlen(path);

  // Call helper
  find_hlpr(eop, name);
}

int
main(int argc, char *argv[])
{
  int i;
  char* path;
  char* name = "";

  if(argc < 3){
    printf(2, "usage: find <directory> -name <name>\n");
    exit();
  }

  if (argv[1][0] != '-') { path = argv[1]; }
  else                   { path = ".";     }
  
  for(i=1; i<argc; i++){
    if (strcmp(argv[i], "-name") == 0){
      if (++i < argc)
      {
        name = argv[i];
      }
      else
      {
        printf(2, "-name requires an argument\n");
        exit();
      }
    }
  }

  find(path, name);

  exit();
}
