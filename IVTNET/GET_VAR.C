extern char *meta;
extern int DEBUG;

#define TIME_SINCE_PREV_REQ 5   /* 1 */

int checkNextRemote()
{
  static short int currentRemote;
  static long int prevVarRequest;
  short int id;
  long t1;
  
  t1 = getRelTime();
/*  t1 = time(0);   */
  
  if (abs(t1 - prevVarRequest) < TIME_SINCE_PREV_REQ)
    return 0;

  if (id = nextRemote(&currentRemote)) {
    struct _remote { long timeStamp; } *remotePtr;
/*
if (DEBUG) printf("Node %d var '%s'\n", metaRemoteNode(meta, id), metaName(meta, id));
*/
    prevVarRequest = t1;
    return id;
  }
/*  prevVarRequest = t1;    */
  return 0;
}

nextRemote(currentRemote)
short int *currentRemote;
{
  int i, id;

#define MAX_COUNT 50
  i = MAX_COUNT;
  id = *currentRemote;
  if (id <= 0)
    id = 1;
/*
if (DEBUG) printf("Start testing with id %d\n", id);
*/
  while (i-- > 0) {
    if (metaName(meta, id) > 0) {
      if (metaRemote(meta, id)) {
        
/*   printf("Remote var '%s'\n", metaName(meta, id));  */
        *currentRemote = id + 1;
        return id;
      } else {
/*   printf("Local var '%s'\n", metaName(meta, id));   */
        id ++;
      }
    } else {
      id = 1;
      break;
    }
  }
  *currentRemote = id;
  return 0;
}

