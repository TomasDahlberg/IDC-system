int minicallSendNumber = 901952;
int minicallLegitimering = 0;
int minicallSendPassword[8] = { ' ', ' ', 'B', 'V', 'G', 'D', 'U', 'C' };
/*
int minicallSendPassword[8] = { 32, 32, 66, 86, 71, 68, 85, 67 };
*/

float minicallPris = 0;

int globalAlarmMask_A = 0;
int globalAlarmMask_B = 0;
int globalAlarmMask_C = 0;
int globalAlarmMask_D = 0;

int telephonePC[8] = { 482086, 0, 0, 0, 0, 0, 0, 0 };
int telephonePC_area[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int telephonePC_ctrl[8] = { 8, 0, 0, 0, 0, 0, 0, 0 };

int alarm_1 = 0, alarm_2 = 0;
int display(), password();
int level, level5 = 1791;

int modemTimeout = 10;

alarm 1 A when (alarm_1) do 
{
  display("Driftfel B112-LB65-TF1 : KV.Oxhuvudet");
}

alarm 2 C when (alarm_2) do 
{
  display("Larmtext 2");
}

calendar calle;

at calle do {
  globalAlarmMask_A |= 1;
} else {
  globalAlarmMask_A &= ~1;
}

screen m
{
  password();
}
