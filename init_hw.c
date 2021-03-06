void init_hardware()
{
   set_tris_a(0b00101111);
   set_tris_b(0b00110111);
   set_tris_c(0b10010000);
   set_tris_d(0b00100000);
   set_tris_e(0b01000000);
   set_tris_f(0b00000000);
   set_tris_g(0b11100100);
   set_tris_h(0b01000000);
   set_tris_j(0b00000000);
   
   // H-BRIDGE
   output_low(HB_EN);
   output_low(HB_IN1);
   output_low(HB_IN2);
   
   // SPI
   output_low(EEP_WP);
   output_high(EEP_CS);       
   output_high(RTC_CS);
   output_high(SD1_CS);     // Req for init_sd(card)
   
   // RS232
   output_high(MD1_SHDN);   // MAX3323 ON
   output_high(MD1_REN);    // Rx enable
   output_high(MD1_TXEN);   // Tx enable
   output_low(MD2_SHDN);    // MAX3323 OFF
   output_low(MD2_REN);     // Rx high impedance (listen)
   output_low(MD2_TXEN);    // Tx disable
   delay_ms(20);  
   
   //PERIPHERALS
   output_low(VMOT);        
   output_low(VDET);    
   bus_pwr_status=0;
   
   //DAC
   output_high(DAC_CL);
   output_high(DAC_CS);
   
   //Heaters
   output_low(HSW1);
   output_low(HSW2);
   heater_stat=0;
}

void shutdown()
{
   // SPI DEVICES
   output_high(SD1_CS); 
   output_low(EEP_WP);       
   output_high(EEP_CS);       
   output_high(RTC_CS); 
   // H-BRIDGE DEVICES
   output_low(HB_EN);        
   output_low(HB_IN1);       
   output_low(HB_IN2);
   //DAC
   output_low(DAC_CL);
   output_low(DAC_CS);
   //Heaters
   output_low(HSW1);
   output_low(HSW2);
   heater_stat=0;
   // Power switches
   output_low(VMOT);        
   output_low(VDET); 
   bus_pwr_status=0;
   output_low(VENC1);        
   output_low(VENC2);        
   output_low(VHBRDG); 
   fprintf(COM_A, "@SLEEP\r\n");//Changed from off.
   delay_ms(50);
   // RS232
   output_high(MD1_SHDN);        // must be on to "hear" characters
   output_high(MD1_REN);         // enable 5k.Ohm termination
   output_low(MD1_TXEN);
   output_low(MD2_SHDN);   
   output_low(MD2_REN);
   output_low(MD2_TXEN);   

   delay_ms(30);
}

// initialise the media and filesystem
int8 init_sdcard()
{
   int8 n;   
   // FS_Status is 0 when all OK
   // error state = 1 to 12
   // this will loop until the card is found
   for (n=0; n<3; n++){
      FS_Status = f_mountdrv();
      if (FS_Status)
         {
         display_file_result(FS_Status);
         }
      else break;
   }
   
   write16(ADDR_SD_STATUS,FS_Status);
   
   return(FS_Status);
}

