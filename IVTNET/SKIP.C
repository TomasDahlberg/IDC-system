#ifdef INDEXVAR
/*
!   move index to 'indexVarBuf'
!   Before:   name         =   'xyz[va1]'
!
!   After:    name         = 'xyz'
!             indexVarBuf  = 'va1'
*/
void skipIndex(name, indexVarBuf)
char *name, *indexVarBuf;
{
  int i, size;

  size = strlen(name);
  *indexVarBuf = '\0';
  for (i = 0; i < size; i++)
    if (name[i] == '[')
      break;
  if (i < size)
  {
    name[i] = 0;
    for (i++; (i < size) && (name[i] != ']'); i++)
      *indexVarBuf++ = name[i];
  }
}

/*
! input:  indexVarBuf = '17'
! returns 17
!
*/
int getIndex(dm, meta, indexVarBuf, idxId)
char *dm, *meta, *indexVarBuf;
int *idxId;
{
  int id;
  
  if (idxId)
    *idxId = 0;
  if (isdigit(indexVarBuf[0]))
    return atoi(indexVarBuf);
  if ((id = metaId(meta, indexVarBuf)) < 0)      /* name not found */
    return 0;
  if (idxId)
    *idxId = id;
  if (metaType(meta, id) == TYPE_INT)
    return *((int *) metaValue(dm, meta, id));
  else if (metaType(meta, id) == TYPE_FLOAT)
    return (int) (*((double *) metaValue(dm, meta, id)));
  else
    return 0;  
}
#endif

