#include <time.h>
#include "idcio.h"
#include "meta.h"

main(argc, argv)
int argc;
char *argv[];
{
  char *meta, *headerPtr1, *dm, *headerPtr2;
  int i, extended, no;
  struct _metaEntry *metaEntry;
  int id;


  meta = (char *) linkDataModule("METAVAR", &headerPtr1);
  if (!meta) {
    printf("cannot link to datamodule 'METAVAR'\n");
    printf("check if process 'scan' is running\n");
    return 0;
  }
  dm = (char *) linkDataModule("VARS", &headerPtr2);
  if (!dm) {
    printf("cannot link to datamodule 'VARS'\n");
    printf("check if process 'scan' is running\n");
    return 0;
  }


/*
  char *name;
  
  if ((id = metaId(meta, name)) < 0) {
    printf("cannot find variable '%s'\n", name);
  }
*/

  id = 17;
  
  if (metaType(meta, id) == TYPE_INT)
  {
    int value;
    
    value = *((int *) metaValue(dm, meta, id));
    
  } else if (metaType(meta, id) == TYPE_FLOAT)
  {
    double value;
    value = *((double *) metaValue(dm, meta, id));
  }

  unlinkDataModule(headerPtr1);
  unlinkDataModule(headerPtr2);
}
