#include "C:\Users\stelios\Desktop\UAV\main.h"
//#include <ctype.h>
//#include <float.h>
//#include <math.h>
//#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#use rs232(ERRORS,UART1A,baud=9600,parity=N,bits=8,stream=gps)
#use rs232(ERRORS,rcv=PIN_F4,xmit=PIN_F5,baud=38400,parity=N,bits=8,stream=xbee)
#use i2c(Master,Fast,Force_SW,sda=PIN_F2,scl=PIN_F3)

//starting bytes for the incoming and outgoing bytes
#define press_start 0x01;
#define accel_start 0x02;
#define gyro_start  0x03;
#define gps_start   0x04;
#define volt_start  0x05;
#define op_start    0x80;

//gps configuration messages
unsigned int8 posrate[10]=  {0xa0,0xa1,0x00,0x03,0x0e,0x01,0x00,0x0F,0x0D,0x0A}; //1Hz update rate
unsigned int8 powermode[10]={0xA0,0xA1,0x00,0x03,0x0C,0x00,0x00,0x0C,0x0D,0x0A}; //normal power mode
unsigned int8 msgtype[10]=  {0xA0,0xA1,0x00,0x03,0x09,0x01,0x00,0x09,0x0D,0x0A}; //receive NMEA messages
unsigned int8 nmeamsg[10]=  {0xA0,0xA1,0x00,0x03,0x09,0x01,0x00,0x09,0x0D,0x0A}; //NMEA configuration
unsigned int8 ackmsg[9]=    {0xA0,0xA1,0x00,0x02,0x83,0x02,0x81,0x0D,0x0A}; //ACK Msg to a request message
unsigned int8 nackmsg[9]=   {0xA0,0xA1,0x00,0x02,0x84,0x01,0x82,0x0D,0x0A}; //NACK Msg to a request message

int16 voltage_pkt[4]; //4 bytes
int16 press_pkt[4];  //4 bytes
int16 accel_pkt[5];  //5 bytes
int16 gyro_pkt[5];   //5 bytes
int16 gps_pkt[13];    //13 bytes
int16 op_pkt[10];     //10 bytes
int16 xaccel,yaccel,zaccel;
unsigned int16 voltage;
int16 press;
unsigned int16 yaw0, pitch0, roll0; //calibration zeroes
signed int32 yaw,pitch,roll; //actual values


void analfunc()      //read values from analog channels and send them into packets to the xbee
{
   int i,j,k;  
   output_low(PIN_B3); //conf bits for gselect
   output_high(PIN_B4);//
   output_high(PIN_B5);
   set_adc_channel(0);
   delay_us(10);
   xaccel = read_adc();
   
   set_adc_channel(1);
   delay_us(10);
   yaccel = read_adc();
   
   set_adc_channel(2);
   delay_us(10);
   zaccel = read_adc();
   
   set_adc_channel(6);
   delay_us(10);
   voltage = read_adc();
   
   set_adc_channel(8);
   delay_us(10);
   press = read_adc();     
}
void xput(int8 w)
{
   while(1==input(PIN_B11))
      ;
    putc(w,xbee);
}
void xput16(int16 w)
{
   xput(w);
   xput(w>>8);
}
void xput32(int32 w)
{
   xput16(w);
   xput16(w>>16);
}
void i2c_begin2(int address,int read)
{
   i2c_start();
   i2c_write(address*2+read);     // Device address
}
void i2c_begin(int address)
{
   i2c_begin2(address,0);
}

void i2c_request(int address)
{
   i2c_begin2(address,1);
}

void i2c_end()
{
   i2c_stop();
}

void gyro_On()
{
   i2c_begin(0x53);
   i2c_write(0xfe);
   i2c_write(0x04);
   
   i2c_end();
}

void gyro_Off()
{
   i2c_begin(0x52);
   i2c_write(0xf0);
   i2c_write(0x55);

   i2c_end();
}

// diavazi 6 bytes sinexomena (diladi sti seira)
void gyro_GetRawData(unsigned int8* dst)
{
   int i;
   
   i2c_begin(0x52);
   i2c_write(0x00); 
   i2c_end();
   
   i2c_request(0x52);
   for (i=0;i<6;i++)
   {
      dst[i]=i2c_read(i!=5);
   }
   i2c_end();
}

unsigned long gyro_DecodeValue(unsigned int16 a,unsigned int16 b, int c)
{
   unsigned long rv;
   rv=(a)+((b>>2)<<8);
   if (c==0)
      rv/=80;
   else
      rv/=16;
   return rv;
}

void gyro_GetData(unsigned long& yaw, unsigned long& roll, unsigned long& pitch)
{
   unsigned int8 data[6];
   gyro_GetRawData(data);
   yaw=gyro_DecodeValue(data[0],data[3],data[3]&2);
   pitch=gyro_DecodeValue(data[1],data[4],data[4]&2);
   roll=gyro_DecodeValue(data[2],data[5],data[3]&1);
}

void gyro_Calibrate()
{
   int i;
   unsigned int32 yc,pc,rc;
   unsigned long yv,pv,rv;

   for (i=0;i<32;i++)
   {
      gyro_GetData(yv,pv,rv);
      yc+=yv;pc+=pv;rc+=rv;
   }
   
   yaw0=yc/32;
   pitch0=pc/32;
   roll0=rc/32;
}

void gyro_ReadData()
{
   unsigned long yv,pv,rv;
   gyro_GetData(yv,pv,rv);
   
   
   yaw=yv-yaw0; 
   pitch=pv-pitch0;
   roll=rv-roll0;
   
}

/*
signed int32 a1,a2,a3;
void clock_isr() 
{
   signed long y,r,p;
   gyro_GetData(y,r,p);
   y-=yaw0;
   r-=roll0;
   p-=pitch0;
   
   if (y>2 || y<-2)
      a1 += y;
   if (r>2 || r<-2)
      a2 += r;
   if (p>2 || p<-2)
      a3 += p;
}
*/

//0 to 2047
//3.750 + 3.750/2 
//5.794 ~ 7330
void set_pwm(int32 a,int32 b, int32 c, int32 d)
{
   //motor1 3960 - 8250 -- 4.290 2,0947265625 17/8 4352
   //motor2 3960 - 7400 -- 3.440 1,6796875 10/6 3413
   //motor3 3970 - 7690 -- 3.720 1,81640625 11/6 3754
   //motor4 3960 - 7400 -- 3.440 1,6796875 10/6 3413
   a=a<0?0:a>2047?2047:a;
   b=b<0?0:b>2047?2047:b;
   c=c<0?0:c>2047?2047:c;
   d=d<0?0:d>2047?2047:d;
   
   set_pwm_duty(1,3900+a*17/8);
   set_pwm_duty(2,3900+b*10/6);
   set_pwm_duty(3,3900+c*11/6);
   set_pwm_duty(4,3900+d*10/6);
}

void set_pwm_sync()
{
   set_pwm_duty(1,3750);
   set_pwm_duty(2,3750);
   set_pwm_duty(3,3750);
   set_pwm_duty(4,3750);
}

void sendframe()
{
   xput16(xaccel);
   xput16(yaccel);
   xput16(zaccel);
   
   xput16(yaw);
   xput16(pitch);
   xput16(roll);
   
   xput16(press);
   xput16(voltage);
}

void getframe()
{
   int a,b,c,d;
   a=getc(xbee)*8;
   b=getc(xbee)*8;
   c=getc(xbee)*8;
   d=getc(xbee)*8;
   
   set_pwm(a,b,c,d);
}

void main()
{
   int a,b,c,d,i;
   a=b=c=d=0;

   setup_timer3(TMR_INTERNAL | TMR_DIV_BY_8, 62500);  //120 mhz time base 120/4/8 -> 3.75MHZ 3.75M/62500=60hz

   setup_compare(1, COMPARE_PWM | COMPARE_TIMER3 );
   setup_compare(2, COMPARE_PWM | COMPARE_TIMER3 );
   setup_compare(3, COMPARE_PWM | COMPARE_TIMER3 );
   setup_compare(4, COMPARE_PWM | COMPARE_TIMER3 );
   
   setup_adc(ADC_CLOCK_INTERNAL);                //
   setup_adc_ports(sAN0|sAN1|sAN2|sAN6|sAN8,VSS_VDD); // AN0,AN1,AN2 and AN8 have been set to analog the other pins are digital, and 0<ref<=5V

  
   /*
      
   */
   //printf(xput,"UAVtel&control v2.6 \r\n");
   //printf(xput,"Syncing motors ...\r\n");
   set_pwm_sync();
   for ( i =0;i<40;i++)
   {
      restart_wdt();
      delay_ms(10);
   }
   gyro_On();
   //printf(xput,"Ready..\r\n");
  
   restart_wdt();
   setup_wdt(WDT_ON);
   
   if (1==1)
   {
      for(;;)
      {
         restart_wdt();
         analfunc();
         gyro_GetData(yaw,roll,pitch);
         sendframe();
         getframe();
      }
   }
   else
   {
      setup_wdt(WDT_OFF);
      restart_wdt();
      printf(xput,"DEBUG MENU \r\n");
      for(;;)
      {
         unsigned long y,r,p;
         restart_wdt();
         analfunc();
         gyro_GetData(y,r,p);
         printf(xput,"telemetry data is x: %d y %d z: %d pre: %d volt: %.2f \r\n",xaccel,yaccel,zaccel,press,voltage/65536.0f*5*3);
         printf(xput,"gyro data is x: %d y %d z: %d \r\n",y,r,p);
         
         if (kbhit(xbee))
         {
            char cmd;            
            cmd=getc(xbee);
            switch(cmd)
            {
               case 'W':
               case 'A':
               case 'S':
               case 'D':
               case 'w':
               case 'a':
               case 's':
               case 'd':
               case ' ':
               case 'F':
               case 'f':
               switch(cmd)
               {
                  case 'W':
                     a+=64;
                     break;
                  case 'w':
                     a-=64;
                     break;
                  
                  case 'S':
                     c+=64;
                     break;
                  case 's':
                     c-=64;
                     break;
                     
                  case 'A':
                     d+=64;
                     break;
                  case 'a':
                     d-=64;
                     break;
                     
                  case 'D':
                     b+=64;
                     break;
                  case 'd':
                     b-=64;
                     break;
                     
                  case ' ':
                     a=b=c=d=0;
                     break;
                     
                  case 'F':
                     a+=64;
                     b=c=d=a;
                     break;
                     
                  case 'f':
                     a-=64;
                     b=c=d=a;
                     break;
               }
               
               printf(xput,"\rnew period is %d %d %d %d   ",a,b,c,d);
               break;
            }
         }
   
         a=a<0?0:a>2047?2047:a;
         b=b<0?0:b>2047?2047:b;
         c=c<0?0:c>2047?2047:c;
         d=d<0?0:d>2047?2047:d;
         
         set_pwm(a,b,c,d);
      }
   }
}
