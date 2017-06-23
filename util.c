// blink LED 3 times
void blink()
{
   int8 i;
   for (i=1;i<=3;++i)
   {
      output_bit(BOARD_LED, ON);
      if(nv_product==AWS)output_bit(PANEL_LED, ON);
      delay_ms(32);
      output_bit(BOARD_LED, OFF);
      if(nv_product==AWS)output_bit(PANEL_LED, OFF); 
      delay_ms(32);
   }
}

int16 read_supply()
{
   int8 i;
   int16 volts;
   int32 result,reading;
   
   result=0;
   set_adc_channel(0,VSS);
   
   for(i=1;i<51;++i){
      reading=read_adc();
      delay_ms(5);
   }
   
   for(i=1;i<101;++i){
      reading=read_adc();
      result=(((result*(i-1))+reading)/i);
      delay_ms(5);
   }   

   volts=((result*100)/272)+86;
   
   //printf("%Ld,%Ld,%Lu\r\n",reading,result,volts);

   return(volts);
}

void clear_time_stmp_str()
{
   memset (&time_stmp_str[0], 0x00, 30);   //blank it
}

void clear_proc_time_stmp_str()
{
   memset (&proc_time_stmp_str[0], 0x00, 30);   //blank it
}

void time_stamp()
{
   int16 v_supply;
   
   clear_time_stmp_str();
   
   v_supply = read_supply();
   RTC_read();
   
   sprintf (time_stmp_str, "%02u/%02u/%02u %02u:%02u:%02u,%04.2w",
      RTC_Mon_Reg,RTC_DOM_Reg,RTC_Yr_Reg,RTC_Hr_Reg,RTC_Min_Reg,RTC_Sec_Reg,v_supply);
}

void record_event()
{
   if(sd_status==0)
   {
      strcopy(event_buffer,"");
      time_stamp();
      strcat(event_buffer, time_stmp_str);
      strcat(event_buffer, event_str);
      buffer_select = 1;
   
      heartbeat(FALSE);
         append_data(file_ptr_events);
      heartbeat(TRUE);
   }
}

