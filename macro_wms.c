// Executes only hard macro commands 
int8 play_wms_macro_cmd (int8 macro_cmd, int16 macro_arg)
{
   int8  macro_status;
   
   switch(macro_cmd){
      //!      case 'L' : switch_valve(macro_arg);
      //!                 macro_status = 'a';
      //!         break;      
      
      // stepper commands - move_motor(use dir/shortest,dir,steps);
      case '+' : motor=1;
                 move_motor(0,0,macro_arg,1);
                 macro_status=step_err_status();
         break;
      case '-' : motor=1;
                 move_motor(0,1,macro_arg,1);
                 macro_status=step_err_status();
         break;
      case 'B' : h_bridge(macro_arg);
                 macro_status = 'a';
         break;
      case 'G' : motor=0;
                 align(0);
                 macro_status=step_err_status();
         break ;          
      case 'P' : nv_port = macro_arg; // macro port will be recorded
                 motor=0;
                 move_motor(0,0,macro_arg,1);  
                 macro_status=step_err_status();
         break;
      case 'Q' : motor=0;
                 move_motor(0,0,macro_arg,1);
                 macro_status=step_err_status();
         break;
      case 'R' : motor=0;
                 move_motor(0,1,macro_arg,1);
                 macro_status=step_err_status();
         break;
      // end stepper commands
      case 'T' : delay_sec(macro_arg);
                 macro_status = 'a';
         break;
         // bus power ON/OFF
      case 'Y' : if (macro_arg == 1) bus_on();
                 else if (macro_arg == 0) bus_off();
                 macro_status = 'a';
         break;          
      case ';' : macro_status = 'f';
         break;           
   }
   // macro_status =  97(a): command complete
   //                 98(b): running (not possible)
   //                 99(c): crap response
   //                100(d): time-out
   //                101(e): error
   //                102(f): finished   
   return(macro_status);
}

void play_wms_hard_macro(int16 port)
{
   int8  macro_status;
   
   if((nv_port>1 && nv_port<49 && nv_product==WMS4) ||
      (nv_port>1 && nv_port<51 && nv_product==WMS2))
   {
      //store_wms_data(0); // moved to below macro execution
      macro_status = 'a';
   }
   else {
      macro_status = 'e';
      goto end_macro;      
   }
   
   macro_status = play_wms_macro_cmd('Y',1);
   if (macro_status != 'a') goto end_macro;
   macro_status = play_wms_macro_cmd('G',1);
   if (macro_status != 'a') goto end_macro;
   macro_status = play_wms_macro_cmd('-',nv_volume);
   if (macro_status != 'a') goto end_macro;
   macro_status = play_wms_macro_cmd('P',port);
   if (macro_status != 'a') goto end_macro;   
   macro_status = play_wms_macro_cmd('+',nv_volume);
   if (macro_status != 'a') goto end_macro;
   macro_status = play_wms_macro_cmd('Y',0);
   if (macro_status != 'a') goto end_macro;
   macro_status='f'; 
   
   end_macro:

   store_wms_data(0); // write data to uart and SD card
  
   switch(macro_status){
      case 'a': if (nv_report_mode > 0) fprintf(COM_A, "@LNE\r\n");
         break;
      case 'b': fprintf(COM_A, "@RUN\r\n");
         break;
      case 'c': fprintf(COM_A, "@BAD\r\n");
         break;
      case 'd': fprintf(COM_A, "@T/O\r\n");
         break;
      case 'e': fprintf(COM_A, "@ERR\r\n");
         break;   
      case 'f': fprintf(COM_A, "@END\r\n");
         break;          
   }
   
   
}

int8 master_macro_wms(int16 step){

   int16 addr;
   
   int8  macro_cmd[3];
   int16 macro_arg[3];
   int8  macro_end;
   
   --step;
   addr = macro_address[0] + (step*9);
   
   init_ext_eeprom();
   
   // interval
   macro_cmd[0] = read_ext_eeprom(addr);
   ++addr;
   macro_arg[0] = read16_ext_eeprom(addr);
   ++addr;
   ++addr;
   // port
   macro_cmd[1] = read_ext_eeprom(addr);
   ++addr;
   macro_arg[1] = read16_ext_eeprom(addr);
   ++addr;
   ++addr;
   // macro
   macro_cmd[2] = read_ext_eeprom(addr);
   ++addr;
   macro_arg[2] = read16_ext_eeprom(addr);   
   ++addr;
   ++addr;
   
   macro_end = read_ext_eeprom(addr);
   
   // check basic structure
   if (macro_cmd[0] == 'J' && macro_cmd[1] == 'P' && macro_cmd[2] == 'M') {
      
      fprintf(COM_A, "%c%Lu,%c%Lu,%c%Lu\r\n", 
         macro_cmd[0], macro_arg[0],            // int
         macro_cmd[1], macro_arg[1],            // port
         macro_cmd[2], macro_arg[2],);          // macro

      // set interval
      nv_interval = macro_arg[0];

      if (macro_arg[2] > 0 && macro_arg[2] < 17) {
         play_macro(macro_arg[2], macro_arg[1]);   // [2] casts to int8 / [1] = port
      }
   }
   else {
      cmd_err();
      macro_end = 59;
   }
   
   return (macro_end);
}

