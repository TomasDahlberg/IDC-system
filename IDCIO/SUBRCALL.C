#include <module.h>

typedef int (*PFI)();

static int fcnNotFound()
{
  printf("ERROR: call to none existing function !!\n");
  exit(0);
}

/* add new entries here */
PFI _initSubrFcn(moduleName, functionName)
char *moduleName;
char *functionName;
{
  mod_exec *modlink();
  mod_exec *execPtr;
  extern int errno;
  int (*lookup)();
  int (*fcnPtr)();
  
  if ((execPtr = modlink(moduleName, 0)) == (mod_exec *) -1) {
/*    printf("error code is %d\n", errno);    */
    printf("Cannot link to subroutine '%s'\n", moduleName);
/*    exit(errno);  */
    return fcnPtr = fcnNotFound;
  }
  lookup = (PFI) (((char *) execPtr) + execPtr->_mexec);

  if (!(fcnPtr = (PFI) lookup(functionName))) {
    fcnPtr = fcnNotFound;
    printf("Cannot find function '%s' in module '%s' !!\n", functionName, 
                                                      moduleName);
  }
  munlink(execPtr);
  return fcnPtr;
}

