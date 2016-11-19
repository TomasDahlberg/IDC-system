/* findmeta.c  1991-08-03 TD,  version 1.1 */
/*
 * This file contains proprietary information of IVT Electronic AB.
 * Copying or reproduction without prior written approval is prohibited.
 *
 * This file is furnished under a license agreement or nondisclosure
 * agreement. The software may be used or copied only in accordance 
 * with the terms of the agreement.
 *
 * In no event will IVT Electronic AB, be liable for any lost revenue or
 * profits or other special, indirect and consequential damages, even if
 * IVT has been advised of the possibility of such damages.
 *
 * IVT Electronic AB
 * Box 996
 * 191 29 Sollentuna
 * Sweden
 */

/*
! findmeta.c
! Copyright (C) 1991, IVT Electronic AB.
*/


#include <time.h>
#include "idcio.h"

char *metaValue(dm, meta, id)
char *dm;
char *meta;
int id;
{
  struct _metaEntry *metaEntry;
  metaEntry = (struct _metaEntry *) meta;
  if (id < 1 || id > metaEntry[0].nameOffset)
    return 0;
  return ((char *) dm + metaEntry[id].offset);
}

char *metaLock(dm, meta, id)
char *dm;
char *meta;
int id;
{
  struct _metaEntry *metaEntry;
  metaEntry = (struct _metaEntry *) meta;
  if (id < 1 || id > metaEntry[0].nameOffset)
    return 0;
  return ((char *) dm + metaEntry[id].lockOffset);
}

char *metaName(meta, id)
char *meta;
int id;
{
  struct _metaEntry *metaEntry;
  metaEntry = (struct _metaEntry *) meta;
  if (id < 1 || id > metaEntry[0].nameOffset)
    return 0;
  return ((char *) meta + metaEntry[id].nameOffset);
}

int metaId(meta, name)
char *meta;
char *name;
{
  int id, a, b, match;

  struct _metaEntry *metaEntry;
  metaEntry = (struct _metaEntry *) meta;
  a = 1;
  b = metaEntry[0].nameOffset;
  while (1) {
    id = (a + b) / 2;
    if ((match = strcmp(((char *) meta) + metaEntry[id].nameOffset, name)) < 0){
      if (a == b)
        break;
      if (a == id) {
        a = b;
        continue;               /* break */
      }
      a = id;                       /* go forward */
    } else if (match > 0) {
      if (a == b)
        break;
      if (b == id) {
        b = a;
        continue;     /* break */
      }
      b = id;                       /* back */
    } else
      break;                        /* ok, found ! */
  }
  return (match == 0) ? id : -1;
}

int metaRemoteId(meta, name, node)
char *meta;
char *name;
int node;
{
  int id, b;
  char *p;
  struct _metaEntry *metaEntry;
  metaEntry = (struct _metaEntry *) meta;

  if ((id = metaId(meta, name)) > 0) {
    if ((metaEntry[id].type & _REMOTE_MASK) &&
        (((metaEntry[id].type >> 8) & 0xFF) == node))
      return id;
  }
/* try for alias name */
  b = metaEntry[0].nameOffset;
  for (id = 1; id <= b; id++) {
    if ((metaEntry[id].type & _ALIAS_MASK) && 
        (metaEntry[id].type & _REMOTE_MASK) &&
        (((metaEntry[id].type >> 8) & 0xFF) == node)) {
      p = ((char *) meta) + metaEntry[id].nameOffset;
      if (strcmp(p + strlen(p) + 1, name) == 0)
        return id;
    }
  }
  return -1;
}

int metaRemoteNode(meta, id)
char *meta;
int id;
{
  struct _metaEntry *metaEntry;
  metaEntry = (struct _metaEntry *) meta;
  return ((metaEntry[id].type >> 8) & 0xFF);
}

int metaRemote(meta, id)
char *meta;
int id;
{
  struct _metaEntry *metaEntry;
  metaEntry = (struct _metaEntry *) meta;
  return metaEntry[id].type & _REMOTE_MASK;
}

int metaAlias(meta, id)
char *meta;
int id;
{
  struct _metaEntry *metaEntry;
  char *p;
  metaEntry = (struct _metaEntry *) meta;
  if (id < 1 || id > metaEntry[0].nameOffset)
    return 0;
  if (metaEntry[id].type & _ALIAS_MASK) {
    p = ((char *) meta + metaEntry[id].nameOffset);
    return p + strlen(p) + 1;
  } else
    return 0;
}

int metaRemoteData(dm, meta, id)
char *dm;
char *meta;
int id;
{
  struct _metaEntry *metaEntry;
  char *p;
  metaEntry = (struct _metaEntry *) meta;
  if (id < 1 || id > metaEntry[0].nameOffset)
    return 0;
  p = ((char *) dm + metaEntry[id].offset);
  p = p - sizeof(struct _remoteS);
  return p;
}


int metaType(meta, id)
char *meta;
int id;
{
  struct _metaEntry *metaEntry;
  metaEntry = (struct _metaEntry *) meta;
  return metaEntry[id].type & (~_REMOTE_MASK) & (~_ALIAS_MASK);
}

int metaSize(meta, id)
char *meta;
int id;
{
  struct _metaEntry *metaEntry;
  metaEntry = (struct _metaEntry *) meta;
  return metaEntry[id].size;
}

