#include <30F4013.h>

#FUSES WDT                    //No Watch Dog Timer
#FUSES FRC_PLL16 //XT_PLL16                 //XT Crystal Oscillator mode with 16X PLL
#FUSES PR_PLL                   //Primary Oscillator with PLL
//#FUSES CKSFSM                   //Clock Switching is enabled, fail Safe clock monitor is enabled
#FUSES WPSB10                   //Watch Dog Timer PreScalar B 1:10
#FUSES WPSA8                  //Watch Dog Timer PreScalar A 1:8
#FUSES PUT64                    //No Power Up Timer
#FUSES NOBROWNOUT               //No brownout reset
//#FUSES BORV47                   //Brownout reset at 4.7V
#FUSES LPOL_HIGH                //Low-Side Transistors Polarity is Active-High (PWM 0,2,4 and 6)
   //PWM module low side output pins have active high output polar
#FUSES HPOL_HIGH                //High-Side Transistors Polarity is Active-High (PWM 1,3,5 and 7)
   //PWM module high side output pins have active high output polarity
#FUSES PWMPIN                 //PWM outputs drive active state upon Reset
#FUSES MCLR                     //Master Clear pin enabled
#FUSES NOPROTECT                //Code not protected from reading
#FUSES NOWRT                    //Program memory not write protected
#FUSES NODEBUG                  //No Debug mode for ICD
#FUSES NOCOE                    //Device will reset into operational mode
//#FUSES ICS0                     //ICD communication channel 0
//#FUSES RESERVED                 //Used to set the reserved FUSE bits

#use delay(clock=120000000)

