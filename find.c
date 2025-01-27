#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

typedef enum { BOTH, FILE, DIR } opt_type_t;
typedef enum { ALL, EQUAL, LESS, GREATER } opt_inum_t;

#define PATH_SIZE 512
char curr_path[PATH_SIZE];
char curr_name[DIRSIZ];

void
find_hlpr(char *eop, char *name,
          opt_type_t opt_type, opt_inum_t opt_inum, int inum, bool opt_printi)
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
    if (strcmp(curr_name, name) == 0 &&
        (opt_type == BOTH || opt_type == FILE) &&
        (opt_inum == ALL || (opt_inum == EQUAL && st.ino == inum) ||
                            (opt_inum == LESS && st.ino < inum)   ||
                            (opt_inum == GREATER && st.ino > inum)))
    {
      if (opt_printi) { printf(1, "%d ", st.ino); }

      printf(1, "%s\n", curr_path);
    }
    break;

  case T_DIR:
    // Mark as directory
    *eop++ = '/';
    *eop = '\0';

    // Print?
    if (strcmp(curr_name, name) == 0 &&
        (opt_type == BOTH || opt_type == DIR) &&
        (opt_inum == ALL || (opt_inum == EQUAL && st.ino == inum) ||
                            (opt_inum == LESS && st.ino < inum)   ||
                            (opt_inum == GREATER && st.ino > inum)))
    {
      if (opt_printi) { printf(1, "%d ", st.ino); }

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
      find_hlpr(eop + strlen(de.name), name, opt_type, opt_inum, inum, opt_printi);
    }
    
    close(fd);
    break;
  }

  return;
}

void
find(char *path, char *name,
     opt_type_t opt_type, opt_inum_t opt_inum, int inum, bool opt_printi)
{
  char *eop = curr_path;
  
  // Copy path to buffer
  if(strlen(path) + 1 > sizeof(curr_path)){
    printf(1, "find: path too long\n");
  }
  strcpy(curr_path, path);
  eop += strlen(path);

  // Call helper
  find_hlpr(eop, name, opt_type, opt_inum, inum, opt_printi);
}

int
main(int argc, char *argv[])
{
  int i;
  char *path;
  char *name = "";
  char *p;
  opt_type_t opt_type = BOTH;
  opt_inum_t opt_inum = ALL;
  bool opt_printi = false;
  int inum = 0;

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
    else if (strcmp(argv[i], "-type") == 0)
    {
      if (++i < argc)
      {
        if      (strcmp(argv[i], "f") == 0) { opt_type = FILE; }
        else if (strcmp(argv[i], "d") == 0) { opt_type = DIR;  }
      }
    }
    else if (strcmp(argv[i], "-inum") == 0)
    {
      if (++i < argc)
      {
        p = argv[i]; // For skipping sign char
        if      (argv[i][0] == '-') { opt_inum = LESS;    p++; }
        else if (argv[i][0] == '+') { opt_inum = GREATER; p++; }
        else                        { opt_inum = EQUAL;        }
        
        inum = atoi(p);
      }
      else
      {
        printf(2, "-inum requires an argument\n");
        exit();
      }
    }
    else if (strcmp(argv[i], "-printi") == 0)
    {
      opt_printi = true;
    }
  }

  find(path, name, opt_type, opt_inum, inum, opt_printi);

  exit();
}
