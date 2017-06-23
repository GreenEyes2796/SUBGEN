/* TITLE BLOCK

   product  : standard product library file

   filename : telem.c
   purpose  : telemetry functions
   hardware : PIC18F8722

   CHANGE RECORD
   
   version  : 001 - new   
*/

char sms_buffer[SMS_BUFFER_LEN];
int16 data_pos;

char pcent[2] ={'%','\0'};

int8  nv_phone_len;
int8  nv_phone_num[PHONE_MAX];

int8 get_number(char *s, int8 max)
{
   int1 end;
   char c;
   int8 len;
   
   end = FALSE;
   --max;
   len=0;
   
   do {
     if (kbhit(COM_A)) { 
        c=fgetc(COM_A); 
        if ((c>='/')&&(c<=':')) { 
           fputc(c, COM_A);
           if(len<max) s[len++]=c;
        } else end=TRUE;
        if (c==13) end=TRUE; 
        //exit if string is complete 
      } 
   } while (!end); 
   
   return(len);
}

void get_phone_number()
{
   int8 addr, n;
   
   addr = ADDR_PHONE_LEN;
   
   for(n=0; n<PHONE_MAX; n++) nv_phone_num[n] = 0x00; //blank it
   
   fputc('[', COM_A);
   
   nv_phone_len = get_number(nv_phone_num, PHONE_MAX);
   
   write8(addr, nv_phone_len);
   
   addr = ADDR_PHONE_NUM;
   for (n=0; n<nv_phone_len; ++n){
      write8((addr+n), nv_phone_num[n]);
   }

   ++n;
   nv_phone_num[n] = '\0';

   fprintf(COM_A, "]\r\n");
}

void read_phone_number()
{
   int8 addr, n;
   
   for(n=0; n<PHONE_MAX; n++) nv_phone_num[n] = 0x00; //blank it
   
   if (nv_phone_len > 0){
      addr = ADDR_PHONE_NUM;
      
      for (n=0; n<nv_phone_len; ++n){
         nv_phone_num[n] = read8(addr+n);
      }
      ++n;
      nv_phone_num[n] = '\0';
   }
   else {
      nv_phone_num[0] = '0';
      nv_phone_num[1] = '\0';
   }
}

int8 get_cmgs()
{
   int8 c, i;
   char modem_reply[40];
   
   c=1;
   for(i=0; i<40; i++) modem_reply[i] = 0x00; //blank it
   
   while (c != 0) {    
      c = timed_getc_B();
      if (isamong (c, MODEM_RESP_LIST)) break;
   }
   
   if (c != 0) {
      get_string(modem_reply, 40);
      fprintf(COM_A, "%s\r\n", modem_reply);
   }
   else fprintf(COM_A, "@T/O\r\n");
   
   return(c); 
}

void clear_sms_buffer()
{
   int8 i;
   for(i=0; i<SMS_BUFFER_LEN; i++) sms_buffer[i] = 0x00; //blank it
}

int8 parse_data_buffer()
{
   int8 c, n;
   c=0;
  
   for(n=0; n<20; n++) sub_string[n] = 0x00; //blank it 
   
   for(n=0; n<20; n++) {
        c = data_buffer[data_pos]; 
        ++data_pos;
        if (c == '\0') return(1); // found end
        if (c == ',')  return(0); // found seperator 
        sub_string[n] = c;
   }
   
   return(2); // failed = bad string
}

void assemble_sms()
{
   int8 f, l;
   
   // Keeper fields ... 2,3,7,9
   
   for (f=1; f<12; ++f){
      parse_data_buffer();
      if (f == 2 || f == 3 || f == 7 || f == 9){
         l = strlen(sms_buffer);
         if (l < 139) {
            strcat(sms_buffer, sub_string);
            strcat(sms_buffer, comma);
         } else if (l < 158) strcat(sms_buffer, pcent);
      }
   }
}

void push_sms_buffer()
{
   char c;
   int8 l;
   
   c = '?';
   l = 0;
   
   while (c != '\0'){
      c = sms_buffer[l];
      if (c != '\0') {
         fputc(c, COM_B);
         delay_ms(1);
         ++l;
      }
   }
}

void transmit_sms()
{
   int c, n;
   
   c = 1;
   
   for (n=0; n<5; ++n){
      fputc('\r');
      delay_ms(500);
      fprintf(COM_B, "AT+CMGS=\"%s\"\r", nv_phone_num);
      delay_ms(200);
      fprintf(COM_B, "@,");
      push_sms_buffer();
      c = get_cmgs();
      if (c != 0) return;
      delay_ms(5000);
   }
}

void send_sms(int16 data_lines)
{
   int8 l, n;
   
   fprintf(COM_A, "@SMS\r\n");
   
   data_pos = 0;
   
   strcat(sms_buffer, comma);          // adds comma after header
   
   for (n=0; n<data_lines; ++n){
      assemble_sms();
   }
   
   l = strlen(sms_buffer);
   sms_buffer[l] = '!';                // ! = end of data
   ++l;
   sms_buffer[l] = 26;                 // 26 = CTRL-Z
   ++l;
   sms_buffer[l] = '\0';               // NULL ... end of string
   
   transmit_sms();
}

void send_sms_test()
{
   char end[2] = {26, '\0'};
   
   fprintf(COM_A, "@SMS\r\n");
   
   clear_sms_buffer();
   
   strcpy(sms_buffer, "12/31/09 23:50:00,9999,888,2,4,2,!");
   strcat(sms_buffer, end);
   
   transmit_sms();
}
