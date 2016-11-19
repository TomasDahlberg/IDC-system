/*
!	calendar text format <-> calendar struct <-> visionCom calendar format
!
!	functions for parsing calendars in text format, converting
!	to composed string (visioncom format), converting back
!	to calendar struct and writing calendar struct to file in readable
!	format.
!
void parseCalendar(char *line, CALENDAR *cal, int entry)
	parses one line into the calendar struct

void cvtCalendar2Str(CALENDAR *cal, char *line)
	converts the calendar struct to string ready for visioncom

void cvtStr2Calendar(CALENDAR *cal, char *line)
	converts answer from visioncom into the calendar struct

void printCalendar(FILE *fp, char *name, CALENDAR *cal)
	takes the calendar structs and writes its contents to file in a 
	readable format (text)
*/

#include <stdio.h>
#include <stdlib.h>


#define NO_OF_CAL_ENTRIES 10

typedef struct {
  unsigned short day[NO_OF_CAL_ENTRIES];
  unsigned short stopday[NO_OF_CAL_ENTRIES];
  unsigned char color[NO_OF_CAL_ENTRIES];
  unsigned short start[NO_OF_CAL_ENTRIES];
  unsigned short stop[NO_OF_CAL_ENTRIES];
} CALENDAR;

/* CALENDAR cal; */

/* 
setvar call:

    CALENDAR cal;

    for (i = 0; i < 10; i++) {
      fgets(line, 80, fpLog);
      parseCalendar(line, &cal, i);
    }
    cvtCalendar2Str(&cal, line);

    form request (<node> setcal <name> "<line>")

getvar call:
  
  get - calendar name
  form request (<node> getcal <name>)
  get answer to line

  cvtStr2Calendar(&cal, line);
  printCalendar(fp, &cal);


file format:

int x = 3;
calendar xyz = {
	"M,T,W,..... ",
	"M,T,W,..... "
};
*/

void parseCalendar(char *line, CALENDAR *cal, int entry);
void cvtCalendar2Str(CALENDAR *cal, char *line);
void cvtStr2Calendar(CALENDAR *cal, char *line);
void printCalendar(FILE *fp, char *name, CALENDAR *cal);


void parseCalendar(char *line, CALENDAR *cal, int entry)
{
  int w, weekDay = 0, color, stopDay = 0, startTime, stopTime;
  static char day[] = { 'M', 'T', 'W', 'T', 'F', 'S', 'S' };

  for (w = 0; w < 7; w++)
    if (line[w*3] == day[w])
	weekDay |= (1 << w);
  if (weekDay == 0) {
    weekDay = atoi(&line[0]);
    stopDay = atoi(&line[7]);
  } else
    weekDay |= 2048;

  if (line[23] == 'B') color = 1;
  else if (line[23] == 'O') color = 2;
  else if (line[23] == 'R') color = 4;
  else color = 0;

  if (line[32] == 'H') {
    startTime = 0;     
    stopTime = 2400;
  } else {
    double atof(), d;
    d = 100.0*atof(&line[32]);
    startTime = d;
    d = 100.0*atof(&line[40]);
    stopTime = d;
  }
  cal->day[entry] = weekDay;
  cal->stopday[entry] = stopDay;
  cal->color[entry] = color;
  cal->start[entry] = startTime;
  cal->stop[entry] = stopTime;
}

void cvtCalendar2Str(CALENDAR *cal, char *line)
{
    int i;

    *line++ = '\"';
    for (i = 0; i < NO_OF_CAL_ENTRIES; i++, line += 17)
    {
        sprintf(line, "%04d%04d%01d%04d%04d", (int) cal->day[i],
            (int) cal->stopday[i],
	    (int) cal->color[i], (int) cal->start[i], (int) cal->stop[i]);
    }
    *line++ = '\"';
    *line = 0;
}

static int readNo(char **bufPtr, int noOfBytes)
{
  int result = 0;

  while (noOfBytes--)
  {
    result = result * 10 + *(*bufPtr)++ - '0';
  }
  return result;
}

void cvtStr2Calendar(CALENDAR *cal, char *line)
{
  int i;

  if (*line == '"')
      line ++;
  for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
  {
      cal->day[i] = readNo(&line, 4);
      cal->stopday[i] = readNo(&line, 4);
      cal->color[i] = readNo(&line, 1);
      cal->start[i] = readNo(&line, 4);
      cal->stop[i] = readNo(&line, 4);
  }
}

void printDayMask(FILE *fp, int day)
{
  static char cday1[] = { 'M', 'T', 'W', 'T', 'F', 'S', 'S' };
  static char cday2[] = { 'o', 'u', 'e', 'h', 'r', 'a', 'u' };
  int w, mask = 1;

  for (w = 0; w < 7; w++, mask <<= 1)
    fprintf(fp, "%c%c,", (day & mask) ? cday1[w] : ' ',
			(day & mask) ? cday2[w] : ' ');
}

void printCalendar(FILE *fp, char *name, CALENDAR *cal)
{
    int i;
    fprintf(fp, "calendar %s = {\n", name);
    for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
    {
      if (cal->day[i] == 0)
      {
	fprintf(fp, "                     !        !               !\n");
	continue;
      }
      if (cal->day[i] & 2048)
      {
	printDayMask(fp, cal->day[i] - 2048);
	fprintf(fp, " ! ");
      }
      else
	fprintf(fp, "%04d - %04d          ! ", cal->day[i], cal->stopday[i]);
	fprintf(fp, "%s ! ", (cal->color[i] & 1) ? "Black " :
				  ((cal->color[i] & 2) ? "Orange" :
				  ((cal->color[i] & 4) ? "Red   " : "      ")));
      if (cal->start[i] == 0 && cal->stop[i] == 2400)
	fprintf(fp, "Hela dygnet   !\n");
      else
	fprintf(fp, "%02d.%02d - %02d.%02d !\n",
	  cal->start[i] / 100,
	  cal->start[i] % 100,
	  cal->stop[i] / 100,
	  cal->stop[i] % 100);
    }
    fprintf(fp, "}\n");
}
