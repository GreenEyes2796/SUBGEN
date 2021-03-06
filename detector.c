
void clear_slave_reply()
{
   memset (&slave_reply[0], 0x00, SLAVE_REPLY_CHARS); //blank it
}

void clear_flag_str()
{
   memset (&flag_str[0], 0x00, 20); //blank it
}

int8 parse_string()
{
   int8 c, n;
   c=0;

   memset (&sub_string[0], 0x00, 20); //blank it

   for(n=0; n<20; n++) {
        c = data_buffer[string_pos]; 
        ++string_pos;
        if (c == '\0') return(1); // found end
        if (c == ',')  return(0); // found seperator 
        sub_string[n] = c;
   }
   
   return(2); // failed = bad string
}
 
void load_parsed_data(int16 result)
{
   switch (chem){
      case "NO3" : NO3_array[read_i] = result;
         break;
      case "NO2" : NO2_array[read_i] = result;
         break;   
      case "PO4" : PO4_array[read_i] = result;
         break;
      case "NH4" : NH4_array[read_i] = result;
         break;
      case "SiO" : SiO_array[read_i] = result;
         break;
   }
}

int16 parse_data()
{
   int16 result = 0;
   int16 temp   = 0;
   int8 n, parsed;
   
   string_pos = 0;
   
   for (n=0; n<11; ++n){
      parsed = parse_string();
      if (parsed == 2) return(0);
      if (n==8) result = atol(sub_string);
      if (n==10)  temp = atol(sub_string);
   }
   load_parsed_data(result);
   return (temp);
}

// *** get data functions *** //

void clear_data_buffer()
{
   memset (&data_buffer[0], 0x00, BUFFER_LEN);  //blank it
}

int16 decode_flag()
{

   int16 wave_l;
   char temp_str[7];

   strcopy(chem,"XXX");

   clear_flag_str();

   sprintf(temp_str,"%Lu",macro_flag);   

   if (macro_flag < 100) {
      temp_str[3] = '\0';
      temp_str[2] = temp_str[1];
      temp_str[1] = temp_str[0];
      temp_str[0] = '0';
   }

   switch (temp_str[0]){
      case '0' : strcopy(chem,"NO3");
                 wave_l=543;
         break;
      case '1' : strcopy(chem,"PO4");
                 wave_l=880;      
         break;
      case '2' : strcopy(chem,"NH4");
                 wave_l=660;      
         break;
      case '3' : strcopy(chem,"SiO");
                 wave_l=810;      
         break;
      case '4' : strcopy(chem,"Ure");
                 wave_l=525;      
         break;
      case '5' : strcopy(chem,"NO2");
                 wave_l=543;      
         break;
      case '6' : strcopy(chem,"Fe_");
                 wave_l=543;      
         break;
      case '7' : strcopy(chem,"Cl_");
                 wave_l=560;      
         break;
      default  : strcopy(chem,"XXX");
                 wave_l=999;      
         break;
   }
   switch (temp_str[1]){
      case '1' : strcopy(anal,"Smp");
         break;
      case '2' : strcopy(anal,"Std");
         break;
      case '3' : strcopy(anal,"Cmb");
         break;
      case '4' : strcopy(anal,"Tst");
         break;
      case '5' : strcopy(anal,"Rbl");
         break;
      case '6' : strcopy(anal,"Cal");
         break;
      case '7' : strcopy(anal,"Fls");
         break;
      case '8' : strcopy(anal,"Utl");
         break;    
      default  : strcopy(anal,"XXX");
         break;         
   }
   switch (temp_str[2]){
      case '1' : strcopy(read_t,"I0s");
                 read_i = 0;
         break;
      case '2' : strcopy(read_t,"I1s");
                 read_i = 1;      
         break;
      case '3' : strcopy(read_t,"I0t");
                 read_i = 2;      
         break;
      case '4' : strcopy(read_t,"I1t");
                 read_i = 3;      
         break;
      case '5' : strcopy(read_t,"I0r");
                 read_i = 4;      
         break;
      case '6' : strcopy(read_t,"I1r");
                 read_i = 5;      
         break;
      case '7' : strcopy(read_t,"I0u");
                 read_i = 6;      
         break;
      case '8' : strcopy(read_t,"I1u");
                 read_i = 7;      
         break;
      case '9' : strcopy(read_t,"Chk");
                 read_i = 8;      
         break;       
      default  : strcopy(read_t,"XX");
                 read_i = 0;      
         break;         
   }
   
   strcat(flag_str, chem);
   strcat(flag_str, (char *)",");
   strcat(flag_str, anal);
   strcat(flag_str, (char *)",");
   strcat(flag_str, read_t);  
   
   return wave_l;
}

void det_cmd()
{
   if (cmd=='K') detector_ch = arg;
   if (cmd=='L') detector_li = arg;
  // if (cmd=='l') detector_li = arg;  // VK - not sure what to set detector_li to here

   // sends a detector command (e.g. set LED)
   if (nv_report_mode == 4) fprintf(COM_A, "@DET[/%u][%c]ARG[%Lu]\r\n", detector, cmd, arg);
   
   multidrop_on();
      fprintf(COM_B, "/%u%c%Lu\r\n",detector, cmd, arg);
   multidrop_off();
}

rcvd_data_type_t get_slave_data()
{
   int8 c, index = 0;
   int1 startCharacterRcvd = FALSE;
   rcvd_data_type_t rcvDataType = NO_DATA;
   int32 tenMicroSecDelayCounter = 0;

   memset (&slave_reply[0], 0x00, SLAVE_REPLY_CHARS); //blank it

   det_cmd();

   // restarts WD and disables timer interrupts
   // ... 4 seconds to complete code before restart or WD will bark
   // ... stops led blinking (caused by timer interrupts)
   suspend_heartbeat();

   // while record==FALSE or TRUE or buffer NOT full
   // END occurs when a CR is received
   // OUT occurs for a time-out
   while (rcvDataType != DATA_COMPLETE && rcvDataType != RCV_TIMEOUT && index < SLAVE_REPLY_CHARS)
   {
      // is data received?
      if (kbhit(COM_B))
      {
         c = fgetc(COM_B);          // Yes, read it

         if(startCharacterRcvd)
         {  // Store data only if the "start sentinel" has been received
            slave_reply[index++] = c;
            if (CARRIAGE_RET == c)
            {
               rcvDataType = DATA_COMPLETE;
            }
         }
         else
         {
            if (AT_SIGN == c)
            {
               startCharacterRcvd = TRUE;
            }
         }
      }

      delay_us(10);
      if (tenMicroSecDelayCounter++ > 500000)
      {
         rcvDataType = RCV_TIMEOUT;  // usec = tenMicroSecDelayCounter*delay_us(10) = 5,000,000 = 3 sec
      }
   }

   if (SLAVE_REPLY_CHARS <= index )
   {
      if ( (RCV_TIMEOUT != rcvDataType) && (DATA_COMPLETE != rcvDataType) )
      {
         rcvDataType = SOME_DATA;
      }
   }

   restart_heartbeat();
  
   // FALSE 0 = start condition
   // TRUE  1 = '@' received
   // END   2 = CR received
   // OUT   3 = time-out
   return(rcvDataType);
}

void display_rec(int8 store, int8 display)
{
   if (nv_report_mode == 0 && display == TRUE) fprintf(COM_A, "%s", data_buffer);
   else if (nv_report_mode >= 4) fprintf(COM_A, "%s", data_buffer);

   if (store==TRUE && sd_status==0) {
      
      buffer_select = 0;
     
      heartbeat(FALSE); // restarts WD & stops timer = 4 seconds to complete
         append_data(file_ptr_raw_all);
      heartbeat(TRUE);
      
      heartbeat(FALSE); // restarts WD & stops timer = 4 seconds to complete
         append_data(file_ptr_raw_new);
      heartbeat(TRUE);
   }
}

int16 det_read(int8 store, int8 display)
{
   // get detector reading and write to a file on the SD card
   // the string to be stored is compiled into "data_buffer"
   // first the data_buffer is cleared
   // then time_stamp_str, flags, results & end-of-line are concatenated
   // append_data writes "data_buffer" to the file "all" and "new" files
   
   #define SHT 4
   #define LNG 5

   char endofline[3] = {'\r' , '\n' , '\0'};
   char err_msg[ERR_MSG_LENGTH];
//!   char timeout[4] = {'T','-','O','\0'};
//!   char at_rx[4] = {'@','R','x','\0'};
//!   char nil_rx[4] = {'N','I','L','\0'};

   rcvd_data_type_t rcvdData = NO_DATA;
   int8  i, rec_len,flag_mod;

   clear_slave_reply();
   clear_data_buffer();
   memset (&err_msg[0], 0x00, ERR_MSG_LENGTH);  //blank it
   memset (&event_str[0], 0x00, BUFFER_LEN);    //blank it

   time_stamp();
   strcpy(data_buffer, time_stmp_str);
   strcat(data_buffer, (char *)",");

   flag_mod=fmod(macro_flag,10);
  // fprintf(COM_A, "Flag Mod=%u\r\n",flag_mod); // just a print for testing

   if (flag_mod==1 || flag_mod==3)
   {
      clear_proc_time_stmp_str();
      strcopy(proc_time_stmp_str,time_stmp_str);
   }

   decode_flag();   
   strncat(data_buffer, flag_str, (BUFFER_LEN - strlen(data_buffer) - 1));
   strcat(data_buffer, (char *)",");
   
   cmd = 'S';
   arg = 1;

   for(i = 0; i < 6; i++)
   {
      rcvdData = get_slave_data();
      if (DATA_COMPLETE == rcvdData)
      {
         break;
      }
   }

   strncat(data_buffer, slave_reply, (BUFFER_LEN - strlen(data_buffer) - 1));

   // if record either NIL or @ received, test the length
   rec_len = strlen(slave_reply);
   if (rec_len > 0 && rec_len < 12)
   {
      rcvdData = SHORT_DATA;
   }
   else
   {
      if (rec_len > 30)
      {
         rcvdData = LONG_DATA;
      }
   }

   switch(rcvdData)
   {
      case DATA_COMPLETE:
      {
         // do nothing
         break;
      }
      case RCV_TIMEOUT:
      {
         sprintf(err_msg, " time-out");
         break;
      }
      case SHORT_DATA:
      {
         sprintf(err_msg, " data too short");
         break;
      }
      case LONG_DATA:
      {
         sprintf(err_msg, " data too long");
         break;
      }
   }

   strncat(data_buffer, err_msg, (BUFFER_LEN - strlen(data_buffer) - 1));
   strncat(data_buffer, endofline, (BUFFER_LEN - strlen(data_buffer) - 1));

   if(DATA_COMPLETE != rcvdData)
   {    //housekeeping
      time_stamp();
      sprintf(event_str, ",detector[%s],len[%u]", err_msg, rec_len);
      strncat(event_str, data_buffer, (BUFFER_LEN - strlen(event_str) - 1));
      record_event();
   }

   display_rec(store, display);

   return (rcvdData);
}

void get_det_read(int8 store, int8 display)
{
   int8 index;

   for(index = 0; index < 3; ++index)
   {
      if (DATA_COMPLETE == det_read(store, display))
      {
         break;
      }
      else
      {
         fprintf(COM_A, "@RTY,%u\r\n", (index+1));
         output_bit(VDET,OFF);
         delay_ms(1000);
         output_bit(VDET,ON);
         delay_ms(1000);
         if (LightTargetFlag==0)
         {
          cmd='K';
          arg=detector_ch;
          det_cmd();
          delay_ms(1000);
          cmd='L';
          arg=detector_li;
          det_cmd();
         } 
         else
         {
          cmd='l';
          arg=1;
          det_cmd();
         } 
      }
   }

   parse_data();
   data_available = TRUE;
   macro_flag = 900;

   return;
}

int16 parse_temp()
{
   int16 temp = 0;

   temp = parse_data();
   return (temp);
}

void det_temp()
{
   int8 store,display;
   int16 n,t;
   int16 current_temp = 0;
   int16 target_temp;
   
   switch (detector)
   {
      case 1 : target_temp = nv_d1_temp;
         break;
      case 2 : target_temp = nv_d2_temp;
         break;
      default : target_temp = 20000;
         break;
   }

   t = 60;         // 5 mins = 300 sec >> delay = 300 / 5 = 60
   n = 0;
   store=FALSE;
   display=FALSE;

   time_stamp();
   sprintf(event_str, ",start heating,target[%Lu],actual[%Lu],readings[%Lu]\r\n", target_temp,current_temp,n);
   record_event();

   for (n=1; n<t; ++n)
   {
      det_read(store, display);           // detector read ~ 2.5 sec
      current_temp=parse_temp();
      if (current_temp >= target_temp || current_temp == 0) break;  // 0=fail (no detector)
      else delay_ms(2500);                               // completes the 5.0 loop
   }
   
   sprintf(event_str, ",end heating,target[%Lu],actual[%Lu],readings[%Lu]\r\n", 
                           target_temp,current_temp,n);
   record_event();   
}

int16 temp_probe()
{
   int8 i;
   int16 temp;
   int32 result,reading;
   
   result=0;
   set_adc_channel(1,VSS);
   
   output_bit(PIN_D0, 1);
   
   for(i=1;i<51;++i){
      reading=read_adc();
      delay_ms(5);
   }
   
   for(i=1;i<101;++i){
      reading=read_adc();
      result=(((result*(i-1))+reading)/i);
      delay_ms(5);
   }   
   
   output_bit(PIN_D0, 0);

   temp=((result*4133)/100)-20513;
   
   //printf("%Ld,%Ld,%Lu\r\n",reading,result,temp);

   return(temp);
}

int16 probe_read(int8 store, int8 display){

   // get detector reading and write to a file on the SD card
   // the string to be stored is compiled into "data_buffer"
   // first the data_buffer is cleared
   // then time_stamp_str, flags, results & end-of-line are concatenated
   // append_data writes "data_buffer" to the file "all" and "new" files

   char endofline[3] = {'\r' , '\n' , '\0'};
   int16 temp = 0;

   clear_slave_reply();
   clear_data_buffer();

   time_stamp();
   strcpy(data_buffer, time_stmp_str);
   strcat(data_buffer, (char *)",Tmp,");

   //*** READ TEMP PROBE ***//
   temp=temp_probe();
   //*** PUT INTO slave_reply ***//
   //itoa(temp,10,slave_reply);
   sprintf (slave_reply, "%05.3w",temp);

   strcat(slave_reply, endofline);
   strncat(data_buffer, slave_reply, (BUFFER_LEN - strlen(data_buffer) - 1));

   if (nv_report_mode == 0 && display == TRUE) fprintf(COM_A, "%s", data_buffer);
   else if (nv_report_mode >= 4) fprintf(COM_A, "%s", data_buffer);

   if (store == TRUE && sd_status==0) {
      buffer_select = 0;
     
      heartbeat(FALSE);
         append_data(file_ptr_raw_all);
      heartbeat(TRUE);
      
      heartbeat(FALSE);
         append_data(file_ptr_raw_new);
      heartbeat(TRUE);
   }
   
   macro_flag = 900;
   
   return (temp);
}


