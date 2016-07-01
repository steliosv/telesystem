
typedef struct
{
   int8 data[fifo_size];
   int8 rp,wp;
} fifo;

void fifo_write(fifo& f, int8 data)
{
   disable_interrupts(INTR_GLOBAL);
   f.data[f.wp]=data;
   f.wp++;
   f.wp%=fifo_size;
   enable_interrupts(INTR_GLOBAL);
}

int fifo_data(fifo& f)
{
   if (f.rp>(f.wp))
      return fifo_size+f.wp-f.rp;
   else
      return f.wp-f.rp;
}
int fifo_empty(fifo& f)
{
   return fifo_data(f)==0;
}
int fifo_read(fifo& f)
{
   int8 rv;
   disable_interrupts(INTR_GLOBAL);
   if (fifo_empty(f))
      return 0;
   rv=f.data[f.rp];
   f.rp++;
   f.rp%=fifo_size;
   enable_interrupts(INTR_GLOBAL);
   return rv;
}

