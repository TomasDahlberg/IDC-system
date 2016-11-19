#include <stdio.h>
#include <time.h>
#include "idcio.h"

static double _localPid();

double pid(regNo, actual, set_val, P, I, D, min, max, uPrev)
int regNo;
double actual, set_val, P, I, D, min, max, uPrev;
{
  static struct _pid context[16];     /* 52 * 16 = 832 bytes */
  return _localPid(&context[regNo & 0x0f], actual, set_val, P, I, D, 
                  min, max, uPrev);
}

#define scale(in, a, b, c, d) ((in/(b-a)) * (d - c) + c)

static double _localPid(context, actual, set_val, P, I, D, min, max, uPrev)
struct _pid *context;
double actual, set_val, P, I, D, min, max, uPrev;
{
    double output, e_n, integral, T, up;
    long dms;
    int first;
    static double pSetVal, pP, pI, pD;
    
    first = (context->previous.date == 0);
    dms = deltatime(&context->previous);
    
    if (first) {
      return(0);
    }
    if (dms == 0)
      return uPrev;

    T = dms / 1000.0;
    e_n = set_val - actual;
    if (I < 1.0)
      integral = 0;
    else
      integral = (T * e_n) / I;

    up = scale(uPrev, min, max, 0.0, 100.0);


#define scale(in, a, b, c, d) (((in-a)/(b-a)) * (d - c) + c)

    output = up +
                 P * (
                        (e_n - context->e_n_1) +
                        integral +
                        D * (e_n - 2 * context->e_n_1 + context->e_n_2) / T
                      );

    context->e_n_2 = context->e_n_1;
    context->e_n_1 = e_n;
    if (output > 100.0)
      output = 100.0;
    if (output < 0.0)
      output = 0.0; 
    context->oldout = output;
    return(scale(output, 0.0, 100.0, min, max));
}
