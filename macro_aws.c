int8 master_macro_aws(int16 step){

   int16 addr;
   
   int8  macro_cmd[2];
   int16 macro_arg[2];
   int8  macro_end;
   
   --step;
   addr = macro_address[0] + (step*6);
   
   init_ext_eeprom();
   
   // port
   macro_cmd[0] = read_ext_eeprom(addr);
   ++addr;
   macro_arg[0] = read16_ext_eeprom(addr);
   ++addr;
   ++addr;
   // macro
   macro_cmd[1] = read_ext_eeprom(addr);
   ++addr;
   macro_arg[1] = read16_ext_eeprom(addr);
   ++addr;
   ++addr;
   
   macro_end = read_ext_eeprom(addr);
   
   // check basic structure
   if (macro_cmd[0] == 'P' && macro_cmd[1] == 'M') {
      
      RTC_read();
      RTC_display();
      fprintf(COM_A, "%c%Lu,%c%Lu\r\n",
         macro_cmd[0], macro_arg[0],            // port
         macro_cmd[1], macro_arg[1]);           // macro

      if (macro_arg[1] > 0 && macro_arg[1] < 17) {
         play_macro(macro_arg[1], macro_arg[0]);   // [1] casts to int8 / [0] = port
      }
   }
   else {
      cmd_err();
      macro_end = 59;
   }
   
   return (macro_end);
}


   
 

