#include <stdio.h>
#include <time.h>
#include "idcio.h"
#define NO_OF_ALARMS 1
#include "../alarm.h"

struct _pid
{
  double e_n_1;
  double uPrev;
};

double pid(regNo, actual, set_val, P, I, D)
int regNo;
double actual, set_val, P, I, D;
{
    double output, e_n, integral;
    static struct _pid Buff_context[4];
    struct _pid *context;
    
    context = &Buff_context[regNo & 0x03];
    e_n = set_val - actual;
    if (I < 1.0)
      integral = 0;
    else
      integral = (e_n) / I;

    output = context->uPrev +
                 P * (
                        (e_n - context->e_n_1) +
                        integral
                      );

    context->e_n_1 = e_n;
    if (output > 100.0)
      output = 100.0;
    if (output < 0.0)
      output = 0.0;
    context->uPrev = output;
    return output;
}

