// *** Timed getc() for each port *** //

int8 timed_getc_A()
{
   int8 c = 0;
   int32 timeout_A = 0;

   while (++timeout_A < GETC_TIMEOUT)
   {
      if(kbhit(COM_A))
      {
         c = fgetc(COM_A);
         if (com_echo == TRUE)
         {
            fputc(c, COM_A);
         }
         break;
      }

      delay_us(10);
   }

   return(c);
}

int8 TestForEsc ()
{
   int8 EscChar = 88;
   int32 timeout_A = 0;
   //EscChar=timed_getc_A();
   // fputc('*',COM_A);
   while(++timeout_A < 50000)
   { //fputc('!',COM_A);
      delay_us(1);

      if(kbhit())
      {//fprintf(COM_A, "kbhit=true\r\n");
         EscChar = fgetc(COM_A);
         // fprintf(COM_A, "EscChar is:%u\r\n",EscChar);
         break;
      }
   }

   return(EscChar);
}

int8 timed_getc_B(int8 timeout_sec)
{
   int8 c = 0;
   int32 timeout_B, timeout_limit;
   
   timeout_B=0;
   timeout_limit = (timeout_sec * 100000);

   while (++timeout_B < timeout_limit)
   {
      if (kbhit(COM_B))
      {
          c = fgetc(COM_B);
          break;
      }

      delay_us(10);
   }

   return (c);
}


// *** Serial "pipes" to each COM port *** //

void multidrop_on()
{
   output_high(MD2_SHDN);     // MAX3323 ON
   output_low(MD2_REN);       // Rx high impedance (listen)
   output_high(MD2_TXEN);     // Tx enable 
   delay_ms(10);
}

void multidrop_off()
{
   /*
   TXSTAx: TRANSMIT STATUS AND CONTROL REGISTER
   bit 1 = TRMT: Transmit Shift Register Status bit
   1 = TSRx empty
   0 = TSRx full
   TXSTA2 addr = 0xF21
   */
   int16 c=0;
   while(TRMT2 == 0){
      ++c;
   }
   output_high(MD2_SHDN);     // MAX3323 ON
   output_low(MD2_REN);       // Rx high impedance (listen)   
   output_low(MD2_TXEN);      // Tx disable
}

void multidrop_shdn()
{
   output_low(MD2_SHDN);      // MAX3323 OFF
   output_low(MD2_REN);       // Rx high impedance (listen)   
   output_low(MD2_TXEN);      // Tx disable
}


void open_pipe()
{
   int8 charFromA = 0;
   int8 charFromB = 0;

   char tempBuffer[MAX_CMD_SIZE];
   int index = 0;
   int charCount = 0;

   fprintf(COM_A, "@[O]\r\n");

   while (charFromB != EOT)
   {
       if (kbhit(COM_A))
       {          // if data is received
          charFromA = fgetc(COM_A);                // read the received data
          if (com_echo == TRUE)
          {
             fputc(charFromA, COM_A);
          }

          if (charCount < MAX_CMD_SIZE)
          {
             tempBuffer[charCount++] = charFromA;
             if (CARRIAGE_RET == charFromA)
             {
                multidrop_on();
                for (index = 0 ; index < charCount; index++)
                {
                   fputc(tempBuffer[index], COM_B);
                }
                charCount = 0;
                multidrop_off();
             }
          }
          else
          {
             fprintf(COM_A, "@CMD too long\r\n");
             charCount = 0;
          }

       }                                 // send data via USART

       if (kbhit(COM_B))
       {          // if data is received
          charFromB = fgetc(COM_B);                // read the received data
          fputc(charFromB, COM_A);                  // send data via USART
       }
   }

   multidrop_shdn();

   fprintf(COM_A, "@[C]\r\n");
}

void get_string(char* s, unsigned int8 max) {
   unsigned int8 len;
   char c;

   --max;
   len=0;
   do {
     c=getc();
     if(c==8) {  // Backspace
        if(len>0) {
          len--;
          putc(c);
          putc(' ');
          putc(c);
        }
     } else if ((c>=' ')&&(c<='~'))
       if(len<=max) {
         s[len++]=c;
         putc(c);
       }
   } while(c != CARRIAGE_RET);
   s[len]=0;
}

float get_float() 
{
  char s[20];
  float f;

  get_string(s, 20);
  f = atof(s);
  return(f);
}








