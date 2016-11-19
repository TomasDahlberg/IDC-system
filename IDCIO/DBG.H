struct _dbg {
  long currLine[3];
  long breakLine[3];
  char release[4];
};

#define DBG_AREA 0x003ffa0
/*
struct _dbg *dbgPtr = DBG_AREA;
*/
