#include "C:\Users\stelios\Desktop\Ground_station\main.h"
#include <stdio.h>
#include <stdlib.h>

#use rs232(UART2,baud=115200,parity=N,bits=8,errors,stream=xbee)
#use rs232(rcv=pin_f2,xmit=pin_f3,baud=115200,parity=N,bits=8,errors,stream=PC)


//functions for the communication from the pc to the xbee
void pctoxbee()
{
   if (kbhit(PC))
   {
      char v;
      v=getc(PC);
      putc(v,xbee);
      //putc(v,PC);
   }
}
//functions for the communication from the xbee to pc
void xbeetopc()
{
   if (kbhit(xbee))
      putc(getc(xbee),PC);
}

void main()
{
   setup_spi(SPI_SS_DISABLED);
   //setup_wdt(WDT_ON);
   //setup_timer1(TMR_DISABLED|TMR_DIV_BY_1);
   fprintf(PC,"Ground_station v2.6 \r\n");

   for (;;)
   {
      pctoxbee();
      xbeetopc();
   }
}
