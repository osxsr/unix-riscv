#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"
#include "kernel/fs.h"

const char *
fmtname(const char *path)
{
  const char *p;
  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  return p + 1;
}

void find_u(const char *path, const char *name)
{
  struct dirent de;
  struct stat st;
  int fd;
  char buf[128] = "", *p;

  if ((fd = open(path, 0)) < 0)
  {
    fprintf(2, "find: cannot open %s\n", path);
    exit(1);
  }

  if (fstat(fd, &st) < 0)
  {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    exit(1);
  }

  if(st.type == T_FILE)
    close(fd);

  //fprintf(2, "%s-fd:%d\n", path, fd);
  if (strcmp(name, fmtname(path)) == 0)
    fprintf(1, "%s\n", path);

  switch (st.type)
  {
  case T_DEVICE:
  case T_FILE:
    //fprintf(2, "file:%s\n", path);
    break;

  case T_DIR:
    //fprintf(2, "dir:%s\n", path);
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
      fprintf(2, "find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
      if (de.inum == 0 || !strcmp(de.name, ".") || !strcmp(de.name, ".."))
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      find_u(buf, name);
    }
    break;
  }
}

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    fprintf(2, "find usage: find path name (find . b)\n");
    exit(1);
  }
  find_u(argv[1], argv[2]);

  exit(0);
}