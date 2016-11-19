/*
    bit stream starts with bit 0 in first byte and 
    terminates with bit 7 last byte.
    
    any values are inserted with msb first !!! 

*/
bitInsert(bF, pos, size, value)
unsigned char *bF;
long pos, size, value;
{
  long result = 0, byte, bitValue, i;

/* start from back ! */
  pos += (size - 1);
  byte = pos / 8;
  bitValue = 1 << (7 - (pos % 8));
  for (i = 0; i < size; i++) {
    if (value & 1)
      bF[byte] |= bitValue; 
    else
      bF[byte] &= (~bitValue);
    value >>= 1;                  /* skip lsb */
    bitValue <<= 1;
    if (bitValue > 128) {
      bitValue = 1;
      byte --;
    }
  }
}

bitExtract(bF, pos, size)
unsigned char *bF;
int pos, size;
{
  long result = 0, byte, bitValue, i;
  
  byte = pos / 8;
  bitValue = 1 << (7 - (pos % 8));
  for (i = 0; i < size; i++) {
    result <<= 1;
    if (bF[byte] & bitValue)
      result |= 1;
    bitValue >>= 1;
    if (bitValue < 1) {
      bitValue = 128;
      byte ++;
    }
  }
  return result;
}

#define NO_OF_CAL_ENTRIES 10
struct _calendar
{
  unsigned short day[NO_OF_CAL_ENTRIES];
  unsigned short stopday[NO_OF_CAL_ENTRIES];
  unsigned char color[NO_OF_CAL_ENTRIES];
  unsigned short start[NO_OF_CAL_ENTRIES];
  unsigned short stop[NO_OF_CAL_ENTRIES];
};

int unpackIdxCalendar(bitPack)
unsigned char *bitPack;
{
  return bitExtract(bitPack, 0, 9);
}

int unpackCalendar(bitPack, calendar)
unsigned char *bitPack;
struct _calendar *calendar;
{
  int size, packetPos, i, value, flag;

  if (!calendar)
    return 0;
  size = bitExtract(bitPack, 9, 4);
  packetPos = 13;
  for (i = 0; i < 10; i++) {
    calendar->day[i] = 0;
    calendar->stopday[i] = 0;
    calendar->color[i] = 0;
    calendar->start[i] = 0;
    calendar->stop[i] = 0;
  }
    
  for (i = 0; i < size; i++, packetPos += 43) {
    flag = bitExtract(bitPack, packetPos, 1);
    value = bitExtract(bitPack, packetPos + 1, 9);
    if (flag)
      calendar->day[i] = value | 2048;
    else
      calendar->day[i] = (value >> 5) * 100 + (value & 31);
    value = bitExtract(bitPack, packetPos + 10, 9);
    calendar->stopday[i] = (value >> 5) * 100 + (value & 31);
    calendar->color[i]   = bitExtract(bitPack, packetPos + 19, 2);
    if (calendar->color[i] == 3)
      calendar->color[i] = 4;
    calendar->start[i]   = bitExtract(bitPack, packetPos + 21, 5) * 100 +
              bitExtract(bitPack, packetPos + 26, 6);
    calendar->stop[i] = bitExtract(bitPack, packetPos + 32, 5) * 100 + 
              bitExtract(bitPack, packetPos + 37, 6);
  }
  return 0;
}

int packCalendar(bitPack, idx, calendar) /* returns size of packet in bytes */
unsigned char *bitPack;
int idx;
struct _calendar *calendar;
{
  int flag, startDay, stopDay, color, startHour, startMin, stopHour, stopMin;
  int size, i, start, stop, packetPos, startValue;
  
  if (!calendar)
    return 0;
  bitInsert(bitPack, 0, 9, idx);
  packetPos = 13;
  for (i = 0; i < 10 /* 9 */; i++, packetPos += 43) {
    if (!(startDay = calendar->day[i]))
      break;
    if (flag = ((startDay & 2048) ? 1 : 0))
      startValue = startDay & 2047;
    else
      startValue = ((startDay / 100) << 5) | (startDay % 100);
    stopDay = calendar->stopday[i];
    stopDay = ((stopDay / 100) << 5) | (stopDay % 100);
    color = calendar->color[i];
    start = calendar->start[i];
    stop = calendar->stop[i];
    startHour = start / 100;
    startMin = start % 100;
    stopHour = stop / 100;
    stopMin = stop % 100;
    bitInsert(bitPack, packetPos, 1, flag);
    bitInsert(bitPack, packetPos + 1, 9, startValue);
    bitInsert(bitPack, packetPos + 10, 9, stopDay);
    bitInsert(bitPack, packetPos + 19, 2, (color == 4) ? 3 : color);
    bitInsert(bitPack, packetPos + 21, 5, startHour);
    bitInsert(bitPack, packetPos + 26, 6, startMin);
    bitInsert(bitPack, packetPos + 32, 5, stopHour);
    bitInsert(bitPack, packetPos + 37, 6, stopMin);
  }
  bitInsert(bitPack, 9, 4, i);
  return (packetPos + 7) / 8;
}

#ifdef DEBUG
main()
{
  unsigned char bitPack[10];
  int i, idx, packetPos, flag, size;
  int startDay, startValue;
    
  idx = 12;
  flag = 1;
  size = 1;

  startDay = 2500;
  
  if (flag = ((startDay & 2048) ? 1 : 0))
    startValue = startDay & 2047;

  printf("Flag = %d, startDay = %d, startValue = %d\n", flag, startDay, startValue);

  bitInsert(bitPack, 0, 9, idx);
  packetPos = 13;
  bitInsert(bitPack, packetPos, 1, flag);
  bitInsert(bitPack, 9, 4, size);
  for (i = 0; i < 5; i++)
    printf("%02x, ", bitPack[i]);
  printf("\n"); 

  printf("extracs says %d\n", bitExtract(bitPack, 9, 4));
}
#endif
