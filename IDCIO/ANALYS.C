#define NAMEOFDATAMODULE "VARS"
#include <time.h>
#include "idcio.h"
#include "../meta.h"
  
main(argc, argv)
int argc;
char *argv[];
{
  char *meta, *headerPtr1, *dm, *headerPtr2;
  int i, extended, no;
  struct _metaEntry *metaEntry;

  if (argv[1][0] == '-' && argv[1][1] == 'e')
    extended = 1;
  else
    extended = 0;

  initidcio();

  meta = (char *) linkDataModule("METAVAR", &headerPtr1);
  if (!meta) {
    printf("cannot link to datamodule '%s'\n", "METAVAR");
    printf("check if process 'scan' is running\n");
    return 0;
  }
  dm = (char *) linkDataModule(NAMEOFDATAMODULE, &headerPtr2);
  if (!dm) {
    printf("cannot link to datamodule '%s'\n", NAMEOFDATAMODULE);
    printf("check if process 'scan' is running\n");
    return 0;
  }
  
  metaEntry = (struct _metaEntry *) meta;
  no = metaEntry->nameOffset;
  printf("No of entries %d\n", no);
  for (i = 1; i <= no; i++)
  {
    long iValue;
    double dValue;
    int typ;
    
    if (extended) {
      printf("----Entry %d ------------------\n", i);
      printf("Name offset : %d (%s)\n", metaEntry[i].nameOffset, 
                                           metaName(meta, i));
      printf("Size        : %d\n", metaEntry[i].size);
      typ = metaEntry[i].type & ~_REMOTE_MASK;
      if (typ == TYPE_INT)
        iValue = *((int *) metaValue(dm, meta, i));
      else
        dValue = *((double *) metaValue(dm, meta, i));

      if (typ == TYPE_INT)
        printf("Offset      : %d (%d)\n", metaEntry[i].offset, iValue);
      else if (typ == TYPE_FLOAT)
        printf("Offset      : %d (%g)\n", metaEntry[i].offset, dValue);
      printf("Lock offset : %d\n", metaEntry[i].lockOffset);
      printf("Type        : %d %s\n", typ, 
                      (metaEntry[i].type & _REMOTE_MASK) ? "(remote)" : "");
    }
    
  } 
  
  unlinkDataModule(headerPtr1);
  unlinkDataModule(headerPtr2);
}
