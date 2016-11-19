/*
!  file main_screen.c generated from main.idc at Wed Jul 27 10:08:25 1994
!  by translator IDCC, version 1.71
*/
#define _idc_source
#define _idcc_version 1.71
#ifndef PC
@_sysattr: equ $8001
@_sysedit: equ   71
#endif
#include <time.h>
#include "idcio.h"
#include "main_symTable.h"
void welcome()
{
  display("ALFA-LAVAL Hus 43 Duc 40\n");
  display("%s IVT Control", xxxswdctime(0));
  dm->level = 2;
}
void pass()
{
  password();
}
void visute()
{
  display("Utetemperatur = %5.1f\017C", dm->Utetemp);
}
void TA1_agg()
{
  display("Ventilations aggregat TA1");
}
void TA1_temp()
{
  display("Temperaturer och Tryck");
}
void TA1_temp_1()
{
  display("Tilluftstemperatur GT31 = %5.1f\017C\n", dm->TA1_GT31);
}
void TA1_temp_1_1()
{
  dm->TA1_GT31_korr = enter("Korrektionsresistans GT31 = %5.1f ohm\n3 ohm ger ca 1\017C s\004nkning", dm->TA1_GT31_korr);
}
void TA1_temp_2()
{
  display("Avluftstemperatur GT32 = %5.1f\017C\n", dm->TA1_GT32);
}
void TA1_temp_2_1()
{
  dm->TA1_GT32_korr = enter("Korrektionsresistans GT32 = %5.1f ohm\n3 ohm ger ca 1\017C s\004nkning", dm->TA1_GT32_korr);
}
void TA1_temp_3()
{
  display("Fr\006nluftstemperatur GT33 = %5.1f\017C\n", dm->TA1_GT33);
}
void TA1_temp_3_1()
{
  dm->TA1_GT33_korr = enter("Korrektionsresistans GT33 = %5.1f ohm\n3 ohm ger ca 1\017C s\004nkning", dm->TA1_GT33_korr);
}
void TA1_temp_4()
{
  display("Tilloppstemperatur GT11 = %5.1f\017C\n", dm->TA1_GT11);
}
void TA1_temp_4_1()
{
  dm->TA1_GT11_korr = enter("Korrektionsresistans GT11 = %5.1f ohm\n3 ohm ger ca 1\017C s\004nkning", dm->TA1_GT11_korr);
}
void TA1_temp_5()
{
  display("Returtemperatur GT51 = %5.1f\017C\n", dm->TA1_GT51);
}
void TA1_temp_5_1()
{
  dm->TA1_GT51_korr = enter("Korrektionsresistans GT51 = %5.1f ohm\n3 ohm ger ca 1\017C s\004nkning", dm->TA1_GT51_korr);
}
void TA1_temp_6()
{
  display("Rumstemperatur GT41 = %5.1f\017C\n", dm->GT41);
}
void TA1_temp_6_1()
{
  dm->GT41_korr = enter("Korrektionsresistans GT41 = %5.1f ohm\n3 ohm ger ca 1\017C s\004nkning", dm->GT41_korr);
}
void TA1_temp_8()
{
  display("Utelufttemperatur TA1/2-GT34 = %5.1f\017C\n", dm->GT34);
}
void TA1_temp_8_1()
{
  dm->GT34_korr = enter("Korrektionsresistans GT34 = %5.1f ohm\n3 ohm ger ca 1\017C s\004nkning", dm->GT34_korr);
}
void TA1_temp_9()
{
  display("Difftryck \024ver TA1 = %5.0f Pa\n", dm->TA1_GP11);
}
void TA1_temp_9_1()
{
  dm->TA1_GP11_k = enter("Konstant k = %5.1f\nFunktion GP11 = Y=k*insignal+m ", dm->TA1_GP11_k);
}
void TA1_temp_9_2()
{
  dm->TA1_GP11_m = enter("Konstant m = %5.1f\nFunktion GP11 = Y=k*insignal+m ", dm->TA1_GP11_m);
}
void TA1_regl()
{
  display("Reglering temperatur");
}
void TA1_regl_1()
{
  if (dm->TA1_Sekvens >= 1)
    if (dm->TA1_Rumsreglering)
      display("DRIFTSTATUS:Fr\006nluftsreglering    \n");
    else 
      display("DRIFTSTATUS:Tilluftsreglering     \n");
  else 
    display("DRIFTSTATUS:Stillest\006ndsreglering \n");
  display("VVX = %3.0f%% SV21 = %3.0f%%", dm->TA1_VVX, dm->TA1_SV21);
}
void TA1_regl_2()
{
  if (dm->TA1_Rumsreglering)
    display("Ledv\004rde tilluft GT31 =%5.1f\017C\n\016rv\004rde = %5.1f\017C ", dm->TA1_Tilluftbor, dm->TA1_GT31);
  else 
    dm->TA1_Tilluftbor = enter("Tilluftsb\024rv\004rde GT31 =%5.1f\017C\n\016rv\004rde=%5.1f\017C ", dm->TA1_Tilluftbor, dm->TA1_GT31);
}
void TA1_regl_3()
{
  if (dm->TA1_Rumsreglering)
    dm->TA1_GT33bor = enter("Fr\006nluftsb\024rv\004rde GT33 =%5.1f\017C\n\016rv\004rde=%5.1f\017C Ledv.GT31=%5.1f\017C", dm->TA1_GT33bor, dm->TA1_GT33, dm->TA1_Tilluftbor);
  else 
    display("Fr\006nluftsreglering ej aktiv \004ndra p\006 DHC");
}
void TA1_regl_4()
{
  if (dm->TA1_Rumsreglering)
    dm->TA1_Maxtilluft = enter("Max tilluftstemperatur GT31 = %5.1f \017C\nTilluftstemperatur GT31 = %5.1f\017C", dm->TA1_Maxtilluft, dm->TA1_GT31);
  else 
    display("Fr\006nluftsreglering ej aktiv \004ndra p\006 DHC");
}
void TA1_regl_5()
{
  if (dm->TA1_Rumsreglering)
    dm->TA1_Mintilluft = enter("Min tilluftstemperatur GT31 = %5.1f \017C\nTilluftstemperatur GT31 = %5.1f\017C", dm->TA1_Mintilluft, dm->TA1_GT31);
  else 
    display("Fr\006nluftsreglering ej aktiv \004ndra p\006 DHC");
}
void TA1_regl_6()
{
  dm->TA1_Bor_StoppAgg = enter("B\024rv Stillest\006ndsregl GT51 = %5.1f\017C\nReturtemp.v\004rmebatteri GT51=%5.1f\017C", dm->TA1_Bor_StoppAgg, dm->TA1_GT51);
}
void TA1_regl_7()
{
  dm->TA1_ForFrysgrans = enter("B\024rv F\024rfrysskyddsregl GT51 = %5.1f\017C\nReturtemp.v\004rmebatteri GT51=%5.1f\017C", dm->TA1_ForFrysgrans, dm->TA1_GT51);
}
void TA1_regl_8()
{
  dm->TA1_Frysgrans = enter("Frysgr\004ns v\004rmebatteri GT51 = %5.1f \017C\nReturtemp.v\004rmebatteri GT51=%5.1f\017C", dm->TA1_Frysgrans, dm->TA1_GT51);
}
void TA1_regl_9()
{
  display("Reglerkonstanter\n                ---->");
}
void TA1_regl_9_1()
{
  display("Stillest\006ndsreglering GT51\n                ---->");
}
void TA1_regl_9_1_1()
{
  dm->TA1_P_Still = enter("P-konstant = %3.2f ", dm->TA1_P_Still);
}
void TA1_regl_9_1_2()
{
  dm->TA1_I_Still = enter("I-tid      = %5.1f sek", dm->TA1_I_Still);
}
void TA1_regl_9_2()
{
  display("Tilluftsregl. GT31 VVX i Kyl sekvens\n                ---->");
}
void TA1_regl_9_2_1()
{
  dm->TA1_P_Kyla = enter("P-konstant = %3.2f ", dm->TA1_P_Kyla);
}
void TA1_regl_9_2_2()
{
  dm->TA1_I_Kyla = enter("I-tid      = %5.1f sek", dm->TA1_I_Kyla);
}
void TA1_regl_9_3()
{
  display("Tilluftsreglering GT31 VVX sekvens\n                ---->");
}
void TA1_regl_9_3_1()
{
  dm->TA1_P_VVX = enter("P-konstant = %3.2f ", dm->TA1_P_VVX);
}
void TA1_regl_9_3_2()
{
  dm->TA1_I_VVX = enter("I-tid      = %5.1f sek", dm->TA1_I_VVX);
}
void TA1_regl_9_4()
{
  display("Tilluftsreglering GT31 V\004rme sekvens\n                ---->");
}
void TA1_regl_9_4_1()
{
  dm->TA1_P_Varme = enter("P-konstant = %3.2f ", dm->TA1_P_Varme);
}
void TA1_regl_9_4_2()
{
  dm->TA1_I_Varme = enter("I-tid      = %5.1f sek", dm->TA1_I_Varme);
}
void TA1_regl_9_5()
{
  display("Fr\006nluftsreglering GT33\n                ---->");
}
void TA1_regl_9_5_1()
{
  dm->TA1_P_Rum = enter("P-konstant = %3.2f ", dm->TA1_P_Rum);
}
void TA1_regl_9_5_2()
{
  dm->TA1_I_Rum = enter("I-tid      = %5.1f sek", dm->TA1_I_Rum);
}
void TA1_regl1()
{
  display("Reglering tryck");
}
void TA1_regl1_1()
{
  if (dm->TA1_Sekvens >= 1 && dm->TA1_start)
    display("DRIFTSTATUS:Reglerar               \n");
  else 
    display("DRIFTSTATUS:Stillast\006ende          \n");
  display("Frekvensomfromare = %3.0f%%", dm->TA1_Frekv);
}
void TA1_regl1_2()
{
  dm->TA1_GP11_bv = enter("Tryckb\024rv\004rde GP11 =%5.1f\017C\n\016rv\004rde=%5.1f\017C ", dm->TA1_GP11_bv, dm->TA1_GP11);
}
void TA1_regl1_3()
{
  display("Reglerkonstanter\n                ---->");
}
void TA1_regl1_3_1()
{
  dm->TA1_GP11_P = enter("P-konstant = %3.2f ", dm->TA1_GP11_P);
}
void TA1_regl1_3_2()
{
  dm->TA1_GP11_I = enter("I-tid      = %5.1f sek", dm->TA1_GP11_I);
}
void TA1_regl1_3_3()
{
  dm->TA1_GP11_D = enter("D-tid      = %5.1f sek", dm->TA1_GP11_D);
}
void TA1_styr()
{
  display("Styrning");
}
void TA1_styr_1()
{
  display("Tidkanal\nInst\004llning --->");
}
void TA1_styr_1_1()
{
  enter_calendar(&dm->TA1_tid);
}
void TA1_delay()
{
  display("Larmf\024rdr\024jningar och Larmgr\004nser");
}
void TA1_delay_1()
{
  dm->TA1_GT31_delay = enter("F\024rdr\024jn regleravvikelse GT31: %3.0d min\nInst\004llning Regleravvikelse --->", dm->TA1_GT31_delay);
}
void TA1_delay_1_1()
{
  dm->TA1_GT31avv = enter("Regleravvikelse GT31 : %5.1f\017C", dm->TA1_GT31avv);
}
void TA1_delay_2()
{
  dm->TA1_delay_drfel = enter("F\024rdr\024jn driftfel TA1-GP11: %3.0d min\nInst\004llning tryckgr\004nser --->", dm->TA1_delay_drfel);
}
void TA1_delay_2_1()
{
  dm->TA1_GP11_av = enter("Regleravvikelse GP11 : %5.1f\017C", dm->TA1_GP11_av);
}
void TA1_delay_2_2()
{
  dm->TA1_GP11_ll = enter("L\006gtryck GP11 : %5.1f\017C", dm->TA1_GP11_ll);
}
void TA1_delay_3()
{
  dm->TA1_GP21_delay = enter("F\024rdr\024jn Fliltervakt GP21: %3.0d min", dm->TA1_GP21_delay);
}
void TA1_delay_4()
{
  dm->TA1_GP31_delay = enter("F\024rdr\024jn Filtervakt GP31: %3.0d min", dm->TA1_GP31_delay);
}
void TA2_agg()
{
  display("Ventilations aggregat TA2");
}
void TA2_temp()
{
  display("Temperaturer och Tryck");
}
void TA2_temp_1()
{
  display("Tilluftstemperatur GT31 = %5.1f\017C\n", dm->TA2_GT31);
}
void TA2_temp_1_1()
{
  dm->TA2_GT31_korr = enter("Korrektionsresistans GT31 = %5.1f ohm\n3 ohm ger ca 1\017C s\004nkning", dm->TA2_GT31_korr);
}
void TA2_temp_2()
{
  display("Avluftstemperatur GT32 = %5.1f\017C\n", dm->TA2_GT32);
}
void TA2_temp_2_1()
{
  dm->TA2_GT32_korr = enter("Korrektionsresistans GT32 = %5.1f ohm\n3 ohm ger ca 1\017C s\004nkning", dm->TA2_GT32_korr);
}
void TA2_temp_3()
{
  display("Fr\006nluftstemperatur GT33 = %5.1f\017C\n", dm->TA2_GT33);
}
void TA2_temp_3_1()
{
  dm->TA2_GT33_korr = enter("Korrektionsresistans GT33 = %5.1f ohm\n3 ohm ger ca 1\017C s\004nkning", dm->TA2_GT33_korr);
}
void TA2_temp_4()
{
  display("Tilloppstemperatur GT11 = %5.1f\017C\n", dm->TA2_GT11);
}
void TA2_temp_4_1()
{
  dm->TA2_GT11_korr = enter("Korrektionsresistans GT11 = %5.1f ohm\n3 ohm ger ca 1\017C s\004nkning", dm->TA2_GT11_korr);
}
void TA2_temp_5()
{
  display("Returtemperatur GT51 = %5.1f\017C\n", dm->TA2_GT51);
}
void TA2_temp_5_1()
{
  dm->TA2_GT51_korr = enter("Korrektionsresistans GT51 = %5.1f ohm\n3 ohm ger ca 1\017C s\004nkning", dm->TA2_GT51_korr);
}
void TA2_temp_6()
{
  display("Rumstemperatur GT41 = %5.1f\017C\n", dm->GT41);
}
void TA2_temp_6_1()
{
  dm->GT41_korr = enter("Korrektionsresistans GT41 = %5.1f ohm\n3 ohm ger ca 1\017C s\004nkning", dm->GT41_korr);
}
void TA2_temp_8()
{
  display("Utelufttemperatur TA1/2-GT34 = %5.1f\017C\n", dm->GT34);
}
void TA2_temp_8_1()
{
  dm->GT34_korr = enter("Korrektionsresistans GT34 = %5.1f ohm\n3 ohm ger ca 1\017C s\004nkning", dm->GT34_korr);
}
void TA2_temp_9()
{
  display("Difftryck \024ver TA2 = %5.0f Pa\n", dm->TA2_GP11);
}
void TA2_temp_9_1()
{
  dm->TA2_GP11_k = enter("Konstant k = %5.1f\nFunktion GP11 = Y=k*insignal+m ", dm->TA2_GP11_k);
}
void TA2_temp_9_2()
{
  dm->TA2_GP11_m = enter("Konstant m = %5.1f\nFunktion GP11 = Y=k*insignal+m ", dm->TA2_GP11_m);
}
void TA2_regl()
{
  display("Reglering temperatur");
}
void TA2_regl_1()
{
  if (dm->TA2_Sekvens >= 1)
    if (dm->TA2_Rumsreglering)
      display("DRIFTSTATUS:Fr\006nluftsreglering   \n");
    else 
      display("DRIFTSTATUS:Tilluftsreglering    \n");
  else 
    display("DRIFTSTATUS:Stillest\006ndsreglering   \n");
  display("VVX = %3.0f%% SV21 = %3.0f%%", dm->TA2_VVX, dm->TA2_SV21);
}
void TA2_regl_2()
{
  if (dm->TA2_Rumsreglering)
    display("Ledv\004rde tilluft GT31 =%5.1f\017C\n\016rv\004rde = %5.1f\017C ", dm->TA2_Tilluftbor, dm->TA2_GT31);
  else 
    dm->TA2_Tilluftbor = enter("Tilluftsb\024rv\004rde GT31 =%5.1f\017C\n\016rv\004rde=%5.1f\017C ", dm->TA2_Tilluftbor, dm->TA2_GT31);
}
void TA2_regl_3()
{
  if (dm->TA2_Rumsreglering)
    dm->TA2_GT33bor = enter("Fr\006nluftsb\024rv\004rde GT33 =%5.1f\017C\n\016rv\004rde=%5.1f\017C Ledv.GT31=%5.1f\017C", dm->TA2_GT33bor, dm->TA2_GT33, dm->TA2_Tilluftbor);
  else 
    display("Fr\006nluftsreglering ej aktiv \004ndra p\006 DHC");
}
void TA2_regl_4()
{
  if (dm->TA2_Rumsreglering)
    dm->TA2_Maxtilluft = enter("Max tilluftstemperatur GT31 = %5.1f \017C\nTilluftstemperatur GT31 = %5.1f\017C", dm->TA2_Maxtilluft, dm->TA2_GT31);
  else 
    display("Fr\006nluftsreglering ej aktiv \004ndra p\006 DHC");
}
void TA2_regl_5()
{
  if (dm->TA2_Rumsreglering)
    dm->TA2_Mintilluft = enter("Min tilluftstemperatur GT31 = %5.1f \017C\nTilluftstemperatur GT31 = %5.1f\017C", dm->TA2_Mintilluft, dm->TA2_GT31);
  else 
    display("Fr\006nluftsreglering ej aktiv \004ndra p\006 DHC");
}
void TA2_regl_6()
{
  dm->TA2_Bor_StoppAgg = enter("B\024rv Stillest\006ndsregl GT51 = %5.1f\017C\nReturtemp.v\004rmebatteri GT51=%5.1f\017C", dm->TA2_Bor_StoppAgg, dm->TA2_GT51);
}
void TA2_regl_7()
{
  dm->TA2_ForFrysgrans = enter("B\024rv F\024rfrysskyddsregl GT51 = %5.1f\017C\nReturtemp.v\004rmebatteri GT51=%5.1f\017C", dm->TA2_ForFrysgrans, dm->TA2_GT51);
}
void TA2_regl_8()
{
  dm->TA2_Frysgrans = enter("Frysgr\004ns v\004rmebatteri GT51 = %5.1f \017C\nReturtemp.v\004rmebatteri GT51=%5.1f\017C", dm->TA2_Frysgrans, dm->TA2_GT51);
}
void TA2_regl_9()
{
  display("Reglerkonstanter\n                ---->");
}
void TA2_regl_9_1()
{
  display("Stillest\006ndsreglering GT51\n                ---->");
}
void TA2_regl_9_1_1()
{
  dm->TA2_P_Still = enter("P-konstant = %3.2f ", dm->TA2_P_Still);
}
void TA2_regl_9_1_2()
{
  dm->TA2_I_Still = enter("I-tid      = %5.1f sek", dm->TA2_I_Still);
}
void TA2_regl_9_2()
{
  display("Tilluftsregl. GT31 VVX i Kyl sekvens\n                ---->");
}
void TA2_regl_9_2_1()
{
  dm->TA2_P_Kyla = enter("P-konstant = %3.2f ", dm->TA2_P_Kyla);
}
void TA2_regl_9_2_2()
{
  dm->TA2_I_Kyla = enter("I-tid      = %5.1f sek", dm->TA2_I_Kyla);
}
void TA2_regl_9_3()
{
  display("Tilluftsreglering GT31 VVX sekvens\n                ---->");
}
void TA2_regl_9_3_1()
{
  dm->TA2_P_VVX = enter("P-konstant = %3.2f ", dm->TA2_P_VVX);
}
void TA2_regl_9_3_2()
{
  dm->TA2_I_VVX = enter("I-tid      = %5.1f sek", dm->TA2_I_VVX);
}
void TA2_regl_9_4()
{
  display("Tilluftsreglering GT31 V\004rme sekvens\n                ---->");
}
void TA2_regl_9_4_1()
{
  dm->TA2_P_Varme = enter("P-konstant = %3.2f ", dm->TA2_P_Varme);
}
void TA2_regl_9_4_2()
{
  dm->TA2_I_Varme = enter("I-tid      = %5.1f sek", dm->TA2_I_Varme);
}
void TA2_regl_9_5()
{
  display("Fr\006nluftsreglering GT33\n                ---->");
}
void TA2_regl_9_5_1()
{
  dm->TA2_P_Rum = enter("P-konstant = %3.2f ", dm->TA2_P_Rum);
}
void TA2_regl_9_5_2()
{
  dm->TA2_I_Rum = enter("I-tid      = %5.1f sek", dm->TA2_I_Rum);
}
void TA2_regl1()
{
  display("Reglering tryck");
}
void TA2_regl1_1()
{
  if (dm->TA2_Sekvens >= 1 && dm->TA2_start)
    display("DRIFTSTATUS:Reglerar                   \n");
  else 
    display("DRIFTSTATUS:Stillast\006ende              \n");
  display("Frekvensomfromare = %3.0f%%", dm->TA2_Frekv);
}
void TA2_regl1_2()
{
  dm->TA2_GP11_bv = enter("Tryckb\024rv\004rde GP11 =%5.1f\017C\n\016rv\004rde=%5.1f\017C ", dm->TA2_GP11_bv, dm->TA2_GP11);
}
void TA2_regl1_3()
{
  display("Reglerkonstanter\n                ---->");
}
void TA2_regl1_3_1()
{
  dm->TA2_GP11_P = enter("P-konstant = %3.2f ", dm->TA2_GP11_P);
}
void TA2_regl1_3_2()
{
  dm->TA2_GP11_I = enter("I-tid      = %5.1f sek", dm->TA2_GP11_I);
}
void TA2_regl1_3_3()
{
  dm->TA2_GP11_D = enter("D-tid      = %5.1f sek", dm->TA2_GP11_D);
}
void TA2_styr()
{
  display("Styrning");
}
void TA2_styr_1()
{
  display("Tidkanal\nInst\004llning --->");
}
void TA2_styr_1_1()
{
  enter_calendar(&dm->TA2_tid);
}
void TA2_delay()
{
  display("Larmf\024rdr\024jningar och Larmgr\004nser");
}
void TA2_delay_1()
{
  dm->TA2_GT31_delay = enter("F\024rdr\024jn regleravvikelse GT31: %3.0d min\nInst\004llning Regleravvikelse --->", dm->TA2_GT31_delay);
}
void TA2_delay_1_1()
{
  dm->TA2_GT31avv = enter("Regleravvikelse GT31 : %5.1f\017C", dm->TA2_GT31avv);
}
void TA2_delay_2()
{
  dm->TA2_delay_drfel = enter("F\024rdr\024jn driftfel TA2-GP11: %3.0d min\nInst\004llning tryckgr\004nser --->", dm->TA2_delay_drfel);
}
void TA2_delay_2_1()
{
  dm->TA2_GP11_av = enter("Regleravvikelse GP11 : %5.1f\017C", dm->TA2_GP11_av);
}
void TA2_delay_2_2()
{
  dm->TA2_GP11_ll = enter("L\006gtryck GP11 : %5.1f\017C", dm->TA2_GP11_ll);
}
void TA2_delay_3()
{
  dm->TA2_GP21_delay = enter("F\024rdr\024jn Fliltervakt GP21: %3.0d min", dm->TA2_GP21_delay);
}
void TA2_delay_4()
{
  dm->TA2_GP31_delay = enter("F\024rdr\024jn Filtervakt GP31: %3.0d min", dm->TA2_GP31_delay);
}
void Visdig()
{
  display("Visning av digitala signaler");
}
void Visdig_in_1()
{
  display("Kortplats 6 dig.in  987654321\n");
  display("                    %0.9a", dm->Kort_6);
}
void Visdig_in_2()
{
  display("Kortplats 7 dig.in  987654321\n");
  display("                    %0.9a", dm->Kort_7);
}
void datumtid()
{
  display("St\004ll Datum och Tid ");
}
void stalltid()
{
  set_time();
}
void Sc_h_1()
{
  display("Pilarna i h\024gerkanten visar \006t\nvilka h\006ll man kan g\006");
}
void Digitalapilar()
{
  display("Pil upp - ing\006ngen \004r aktiv\n");
  display("Pil ner - ing\006ngen \004r ej aktiv");
}
void P_Konst()
{
  display("Anger vilken f\024rst\004rkning i ggr\n");
  display("regulatorn anv\004nder ");
}
void I_Konst()
{
  display("Anger vilken integrationstid i sek.\n");
  display("regulatorn anv\004nder ");
}
void D_Konst()
{
  display("Anger vilken deriveringstid i sek.\n");
  display("regulatorn anv\004nder ");
}

#ifndef PC
typedef int (*PFI)();
#else
typedef void (*PFI)();
#endif

struct {
  short left, right, up, down, help;
  PFI fcn;
  char *name;
} screens[] = {
    /*left	right	up	down	help	fcn	name*/
    { 0,	0,	0,	0,	-1,	D_Konst,	"D_Konst"},	/*1*/
    { 0,	0,	0,	0,	-1,	Digitalapilar,	"Digitalapilar"},	/*2*/
    { 0,	0,	0,	0,	-1,	I_Konst,	"I_Konst"},	/*3*/
    { 0,	0,	0,	0,	-1,	P_Konst,	"P_Konst"},	/*4*/
    { 0,	0,	0,	0,	-1,	Sc_h_1,	"Sc_h_1"},	/*5*/
    { 0,	50,	136,	68,	5,	TA1_agg,	"TA1_agg"},	/*6*/
    { 0,	8,	47,	0,	5,	TA1_delay,	"TA1_delay"},	/*7*/
    { 7,	9,	0,	10,	5,	TA1_delay_1,	"TA1_delay_1"},	/*8*/
    { 8,	0,	0,	0,	5,	TA1_delay_1_1,	"TA1_delay_1_1"},	/*9*/
    { 0,	11,	8,	13,	5,	TA1_delay_2,	"TA1_delay_2"},	/*10*/
    { 10,	0,	0,	12,	5,	TA1_delay_2_1,	"TA1_delay_2_1"},	/*11*/
    { 0,	0,	11,	0,	5,	TA1_delay_2_2,	"TA1_delay_2_2"},	/*12*/
    { 0,	0,	10,	14,	5,	TA1_delay_3,	"TA1_delay_3"},	/*13*/
    { 0,	0,	13,	0,	5,	TA1_delay_4,	"TA1_delay_4"},	/*14*/
    { 0,	23,	50,	16,	5,	TA1_regl,	"TA1_regl"},	/*15*/
    { 0,	17,	15,	47,	5,	TA1_regl1,	"TA1_regl1"},	/*16*/
    { 16,	0,	0,	18,	5,	TA1_regl1_1,	"TA1_regl1_1"},	/*17*/
    { 0,	0,	17,	19,	5,	TA1_regl1_2,	"TA1_regl1_2"},	/*18*/
    { 0,	20,	18,	0,	5,	TA1_regl1_3,	"TA1_regl1_3"},	/*19*/
    { 19,	0,	0,	21,	4,	TA1_regl1_3_1,	"TA1_regl1_3_1"},	/*20*/
    { 0,	0,	20,	22,	3,	TA1_regl1_3_2,	"TA1_regl1_3_2"},	/*21*/
    { 0,	0,	21,	0,	1,	TA1_regl1_3_3,	"TA1_regl1_3_3"},	/*22*/
    { 15,	0,	0,	24,	5,	TA1_regl_1,	"TA1_regl_1"},	/*23*/
    { 0,	0,	23,	25,	5,	TA1_regl_2,	"TA1_regl_2"},	/*24*/
    { 0,	0,	24,	26,	5,	TA1_regl_3,	"TA1_regl_3"},	/*25*/
    { 0,	0,	25,	27,	5,	TA1_regl_4,	"TA1_regl_4"},	/*26*/
    { 0,	0,	26,	28,	5,	TA1_regl_5,	"TA1_regl_5"},	/*27*/
    { 0,	0,	27,	29,	5,	TA1_regl_6,	"TA1_regl_6"},	/*28*/
    { 0,	0,	28,	30,	5,	TA1_regl_7,	"TA1_regl_7"},	/*29*/
    { 0,	0,	29,	31,	5,	TA1_regl_8,	"TA1_regl_8"},	/*30*/
    { 0,	32,	30,	0,	5,	TA1_regl_9,	"TA1_regl_9"},	/*31*/
    { 31,	33,	0,	35,	5,	TA1_regl_9_1,	"TA1_regl_9_1"},	/*32*/
    { 32,	0,	0,	34,	4,	TA1_regl_9_1_1,	"TA1_regl_9_1_1"},	/*33*/
    { 0,	0,	33,	0,	3,	TA1_regl_9_1_2,	"TA1_regl_9_1_2"},	/*34*/
    { 0,	36,	32,	38,	5,	TA1_regl_9_2,	"TA1_regl_9_2"},	/*35*/
    { 35,	0,	0,	37,	4,	TA1_regl_9_2_1,	"TA1_regl_9_2_1"},	/*36*/
    { 0,	0,	36,	0,	3,	TA1_regl_9_2_2,	"TA1_regl_9_2_2"},	/*37*/
    { 0,	39,	35,	41,	5,	TA1_regl_9_3,	"TA1_regl_9_3"},	/*38*/
    { 38,	0,	0,	40,	4,	TA1_regl_9_3_1,	"TA1_regl_9_3_1"},	/*39*/
    { 0,	0,	39,	0,	3,	TA1_regl_9_3_2,	"TA1_regl_9_3_2"},	/*40*/
    { 0,	42,	38,	44,	5,	TA1_regl_9_4,	"TA1_regl_9_4"},	/*41*/
    { 41,	0,	0,	43,	4,	TA1_regl_9_4_1,	"TA1_regl_9_4_1"},	/*42*/
    { 0,	0,	42,	0,	3,	TA1_regl_9_4_2,	"TA1_regl_9_4_2"},	/*43*/
    { 0,	45,	41,	0,	5,	TA1_regl_9_5,	"TA1_regl_9_5"},	/*44*/
    { 44,	0,	0,	46,	4,	TA1_regl_9_5_1,	"TA1_regl_9_5_1"},	/*45*/
    { 0,	0,	45,	0,	3,	TA1_regl_9_5_2,	"TA1_regl_9_5_2"},	/*46*/
    { 0,	48,	16,	7,	5,	TA1_styr,	"TA1_styr"},	/*47*/
    { 47,	49,	0,	0,	5,	TA1_styr_1,	"TA1_styr_1"},	/*48*/
    { 48,	0,	0,	0,	5,	TA1_styr_1_1,	"TA1_styr_1_1"},	/*49*/
    { 6,	51,	0,	15,	5,	TA1_temp,	"TA1_temp"},	/*50*/
    { 50,	52,	0,	53,	5,	TA1_temp_1,	"TA1_temp_1"},	/*51*/
    { 51,	0,	0,	0,	5,	TA1_temp_1_1,	"TA1_temp_1_1"},	/*52*/
    { 0,	54,	51,	55,	5,	TA1_temp_2,	"TA1_temp_2"},	/*53*/
    { 53,	0,	0,	0,	5,	TA1_temp_2_1,	"TA1_temp_2_1"},	/*54*/
    { 0,	56,	53,	57,	5,	TA1_temp_3,	"TA1_temp_3"},	/*55*/
    { 55,	0,	0,	0,	5,	TA1_temp_3_1,	"TA1_temp_3_1"},	/*56*/
    { 0,	58,	55,	59,	5,	TA1_temp_4,	"TA1_temp_4"},	/*57*/
    { 57,	0,	0,	0,	5,	TA1_temp_4_1,	"TA1_temp_4_1"},	/*58*/
    { 0,	60,	57,	61,	5,	TA1_temp_5,	"TA1_temp_5"},	/*59*/
    { 59,	0,	0,	0,	5,	TA1_temp_5_1,	"TA1_temp_5_1"},	/*60*/
    { 0,	62,	59,	63,	5,	TA1_temp_6,	"TA1_temp_6"},	/*61*/
    { 61,	0,	0,	0,	5,	TA1_temp_6_1,	"TA1_temp_6_1"},	/*62*/
    { 0,	64,	61,	65,	5,	TA1_temp_8,	"TA1_temp_8"},	/*63*/
    { 63,	0,	0,	0,	5,	TA1_temp_8_1,	"TA1_temp_8_1"},	/*64*/
    { 0,	66,	63,	0,	5,	TA1_temp_9,	"TA1_temp_9"},	/*65*/
    { 65,	0,	0,	67,	5,	TA1_temp_9_1,	"TA1_temp_9_1"},	/*66*/
    { 0,	0,	66,	0,	5,	TA1_temp_9_2,	"TA1_temp_9_2"},	/*67*/
    { 0,	112,	6,	130,	5,	TA2_agg,	"TA2_agg"},	/*68*/
    { 0,	70,	109,	0,	5,	TA2_delay,	"TA2_delay"},	/*69*/
    { 69,	71,	0,	72,	5,	TA2_delay_1,	"TA2_delay_1"},	/*70*/
    { 70,	0,	0,	0,	5,	TA2_delay_1_1,	"TA2_delay_1_1"},	/*71*/
    { 0,	73,	70,	75,	5,	TA2_delay_2,	"TA2_delay_2"},	/*72*/
    { 72,	0,	0,	74,	5,	TA2_delay_2_1,	"TA2_delay_2_1"},	/*73*/
    { 0,	0,	73,	0,	5,	TA2_delay_2_2,	"TA2_delay_2_2"},	/*74*/
    { 0,	0,	72,	76,	5,	TA2_delay_3,	"TA2_delay_3"},	/*75*/
    { 0,	0,	75,	0,	5,	TA2_delay_4,	"TA2_delay_4"},	/*76*/
    { 0,	85,	112,	78,	5,	TA2_regl,	"TA2_regl"},	/*77*/
    { 0,	79,	77,	109,	5,	TA2_regl1,	"TA2_regl1"},	/*78*/
    { 78,	0,	0,	80,	5,	TA2_regl1_1,	"TA2_regl1_1"},	/*79*/
    { 0,	0,	79,	81,	5,	TA2_regl1_2,	"TA2_regl1_2"},	/*80*/
    { 0,	82,	80,	0,	5,	TA2_regl1_3,	"TA2_regl1_3"},	/*81*/
    { 81,	0,	0,	83,	4,	TA2_regl1_3_1,	"TA2_regl1_3_1"},	/*82*/
    { 0,	0,	82,	84,	3,	TA2_regl1_3_2,	"TA2_regl1_3_2"},	/*83*/
    { 0,	0,	83,	0,	1,	TA2_regl1_3_3,	"TA2_regl1_3_3"},	/*84*/
    { 77,	0,	0,	86,	5,	TA2_regl_1,	"TA2_regl_1"},	/*85*/
    { 0,	0,	85,	87,	5,	TA2_regl_2,	"TA2_regl_2"},	/*86*/
    { 0,	0,	86,	88,	5,	TA2_regl_3,	"TA2_regl_3"},	/*87*/
    { 0,	0,	87,	89,	5,	TA2_regl_4,	"TA2_regl_4"},	/*88*/
    { 0,	0,	88,	90,	5,	TA2_regl_5,	"TA2_regl_5"},	/*89*/
    { 0,	0,	89,	91,	5,	TA2_regl_6,	"TA2_regl_6"},	/*90*/
    { 0,	0,	90,	92,	5,	TA2_regl_7,	"TA2_regl_7"},	/*91*/
    { 0,	0,	91,	93,	5,	TA2_regl_8,	"TA2_regl_8"},	/*92*/
    { 0,	94,	92,	0,	5,	TA2_regl_9,	"TA2_regl_9"},	/*93*/
    { 93,	95,	0,	97,	5,	TA2_regl_9_1,	"TA2_regl_9_1"},	/*94*/
    { 94,	0,	0,	96,	4,	TA2_regl_9_1_1,	"TA2_regl_9_1_1"},	/*95*/
    { 0,	0,	95,	0,	3,	TA2_regl_9_1_2,	"TA2_regl_9_1_2"},	/*96*/
    { 0,	98,	94,	100,	5,	TA2_regl_9_2,	"TA2_regl_9_2"},	/*97*/
    { 97,	0,	0,	99,	4,	TA2_regl_9_2_1,	"TA2_regl_9_2_1"},	/*98*/
    { 0,	0,	98,	0,	3,	TA2_regl_9_2_2,	"TA2_regl_9_2_2"},	/*99*/
    { 0,	101,	97,	103,	5,	TA2_regl_9_3,	"TA2_regl_9_3"},	/*100*/
    { 100,	0,	0,	102,	4,	TA2_regl_9_3_1,	"TA2_regl_9_3_1"},	/*101*/
    { 0,	0,	101,	0,	3,	TA2_regl_9_3_2,	"TA2_regl_9_3_2"},	/*102*/
    { 0,	104,	100,	106,	5,	TA2_regl_9_4,	"TA2_regl_9_4"},	/*103*/
    { 103,	0,	0,	105,	4,	TA2_regl_9_4_1,	"TA2_regl_9_4_1"},	/*104*/
    { 0,	0,	104,	0,	3,	TA2_regl_9_4_2,	"TA2_regl_9_4_2"},	/*105*/
    { 0,	107,	103,	0,	5,	TA2_regl_9_5,	"TA2_regl_9_5"},	/*106*/
    { 106,	0,	0,	108,	4,	TA2_regl_9_5_1,	"TA2_regl_9_5_1"},	/*107*/
    { 0,	0,	107,	0,	3,	TA2_regl_9_5_2,	"TA2_regl_9_5_2"},	/*108*/
    { 0,	110,	78,	69,	5,	TA2_styr,	"TA2_styr"},	/*109*/
    { 109,	111,	0,	0,	5,	TA2_styr_1,	"TA2_styr_1"},	/*110*/
    { 110,	0,	0,	0,	5,	TA2_styr_1_1,	"TA2_styr_1_1"},	/*111*/
    { 68,	113,	0,	77,	5,	TA2_temp,	"TA2_temp"},	/*112*/
    { 112,	114,	0,	115,	5,	TA2_temp_1,	"TA2_temp_1"},	/*113*/
    { 113,	0,	0,	0,	5,	TA2_temp_1_1,	"TA2_temp_1_1"},	/*114*/
    { 0,	116,	113,	117,	5,	TA2_temp_2,	"TA2_temp_2"},	/*115*/
    { 115,	0,	0,	0,	5,	TA2_temp_2_1,	"TA2_temp_2_1"},	/*116*/
    { 0,	118,	115,	119,	5,	TA2_temp_3,	"TA2_temp_3"},	/*117*/
    { 117,	0,	0,	0,	5,	TA2_temp_3_1,	"TA2_temp_3_1"},	/*118*/
    { 0,	120,	117,	121,	5,	TA2_temp_4,	"TA2_temp_4"},	/*119*/
    { 119,	0,	0,	0,	5,	TA2_temp_4_1,	"TA2_temp_4_1"},	/*120*/
    { 0,	122,	119,	123,	5,	TA2_temp_5,	"TA2_temp_5"},	/*121*/
    { 121,	0,	0,	0,	5,	TA2_temp_5_1,	"TA2_temp_5_1"},	/*122*/
    { 0,	124,	121,	125,	5,	TA2_temp_6,	"TA2_temp_6"},	/*123*/
    { 123,	0,	0,	0,	5,	TA2_temp_6_1,	"TA2_temp_6_1"},	/*124*/
    { 0,	126,	123,	127,	5,	TA2_temp_8,	"TA2_temp_8"},	/*125*/
    { 125,	0,	0,	0,	5,	TA2_temp_8_1,	"TA2_temp_8_1"},	/*126*/
    { 0,	128,	125,	0,	5,	TA2_temp_9,	"TA2_temp_9"},	/*127*/
    { 127,	0,	0,	129,	5,	TA2_temp_9_1,	"TA2_temp_9_1"},	/*128*/
    { 0,	0,	128,	0,	5,	TA2_temp_9_2,	"TA2_temp_9_2"},	/*129*/
    { 0,	131,	68,	133,	5,	Visdig,	"Visdig"},	/*130*/
    { 130,	0,	0,	132,	2,	Visdig_in_1,	"Visdig_in_1"},	/*131*/
    { 0,	0,	131,	0,	2,	Visdig_in_2,	"Visdig_in_2"},	/*132*/
    { 0,	135,	130,	0,	5,	datumtid,	"datumtid"},	/*133*/
    { 0,	0,	137,	136,	5,	pass,	"pass"},	/*134*/
    { 133,	0,	0,	0,	0,	stalltid,	"stalltid"},	/*135*/
    { 0,	0,	134,	6,	5,	visute,	"visute"},	/*136*/
    { 0,	0,	0,	134,	5,	welcome,	"welcome"},	/*137*/
    { 0, 0, 0, 0, 0, 0}
};
int _ROOT_SCREEN_POINTER = 137;
