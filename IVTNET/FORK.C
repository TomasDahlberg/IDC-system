extern int os9fork();

char *argblk[] = {
    "shell",
    0
};

extern char **environ;

#define SHELL_TERMINATED 0x0101

main(argc, argv)
int argc;
char *argv[];
{
  int shellid, pid;

  pid = atoi(argv[1]);
  
/*
  int paramSize;
  char param[20];

  if ((screenContext->spawnedScreenPid = 
                    os9fork("screen", param, paramSize, 0, 0, 0, 0)) == -1) {
*/ 

  shellid = os9exec(os9fork, argblk[0], argblk, environ, 0, 0, 3);
  wait(0);
  kill(pid, SHELL_TERMINATED);
}
