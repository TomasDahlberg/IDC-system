/*
!   pseudo code for smartCache
*/


if (msg == var request )
{
  if (lookupCacheTable(node, varname)) {
    update delta-time
    if (timestampForValue is good) {
      make packet out of entry;
      reply this packet
    }
  } else {
    if (any-entry > 5 min)
      discard entry
    if (not any free entry)
      discard weakest entry (lowest update frequency)
    if (any entry not initialized or empty)
      insert var entry
  }
}


if (msg == var reply)
{
  if (lookupCacheTable(sourceNode, varname)) {
    insertValueAndTimeStamp();
  }
}
