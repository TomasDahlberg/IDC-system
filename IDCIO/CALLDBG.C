/*
struct _dbg {
  long currLine[3];
  long breakLine[3];
  char release[4];
} *dbgPtr;
*/
#include "dbg.h"

struct _dbg *dbgPtr = DBG_AREA;

callDbg(line, prcs)
int line, prcs;
{
  prcs--;
  dbgPtr->currLine[prcs] = line;
  if ((dbgPtr->breakLine[prcs] == -1) || (dbgPtr->breakLine[prcs] == line)) {
    dbgPtr->release[prcs] = 0;
    while (!dbgPtr->release[prcs])
      ;
  }
  return 0;
}
