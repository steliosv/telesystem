
void i2c_begin2(int address,int read)
{
   i2c_start();
   i2c_write(address*2+read);     // Device address
}
void i2c_begin(int address)
{
   i2c_begin2(address,0);
}

void i2c_request(int address) //begin write transaction
{
   i2c_begin2(address,1);
}

void i2c_end()
{
   i2c_stop();
}

void gyro_On() //activation of the module
{
   i2c_begin(0x53);//WM+ starts out deactivated at address 0x53
   i2c_write(0xfe); //send 0x04 to address 0xFE to activate WM+
   i2c_write(0x04);
   i2c_end(); //WM+ jumps to address 0x52 and is now active
}

void gyro_Off() //deactivation of the module
{
   i2c_begin(0x52);
   i2c_write(0xf0);
   i2c_write(0x55);
   i2c_end();
}

//raw 6 bytes wmp packet
void gyro_GetRawData(unsigned int8* dst)
{
   int i;
   
   //request measurement start from the module
   i2c_begin(0x52);
   i2c_write(0x00);
   i2c_end(); 
   
   //read the values
   i2c_request(0x52);
   for (i=0;i<6;i++)
   {
      dst[i]=i2c_read(i!=5);
   }
   i2c_end();
}

unsigned int16 gyro_DecodeValue(unsigned int16 a,unsigned int16 b, int c)
{
   unsigned int16 rv;
   rv=(a)+((b>>2)<<8);
   if (c==0)
      rv/=80;
   else
      rv/=16;
   return rv;
}

//16 bit signed -uncalibrated- values
void gyro_GetData(unsigned int16& yaw, unsigned int16& roll, unsigned int16& pitch)
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
   unsigned int16 yv,pv,rv;

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
   unsigned int16 yv,pv,rv;
   gyro_GetData(yv,pv,rv);
   
   //for info on what each byte represents
   yaw=yv-yaw0; 
   pitch=pv-pitch0;
   roll=rv-roll0;
   
}