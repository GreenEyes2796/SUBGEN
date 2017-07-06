
// --------------------------------------------------------------------//
#define MACEND  1024
// --------------------------------------------------------------------//
#define MACROLIST_ECO   "#/+-%rpdACDFGHiKlLPQRSTUvY;" // Valid macro commands
#define MASTERLIST_ECO  "#MZr%;"               // Master macro commands
// --------------------------------------------------------------------//
#define MACROLIST_WMS   "#+-BGPQRTY;"
#define MASTERLIST_WMS  "#JMP;"
// --------------------------------------------------------------------//
#define MACROLIST_AWS   "#+-CDPQSY;"
#define MASTERLIST_AWS  "#MP;"
// --------------------------------------------------------------------//

#include "command.h"  //Fraser 7/2/2015

int16 macro_address[21] = {MACROM,MACRO1,MACRO2,MACRO3,MACRO4,MACRO5,MACRO6,
  MACRO7,MACRO8,MACRO9,MACRO10,MACRO11,MACRO12,MACRO13,MACRO14,MACRO15,MACRO16,MACRO17,MACRO18,MACRO19,MACRO20};
  
int16 string_address[21] = {STRM,STR1,STR2,STR3,STR4,STR5,STR6,STR7,STR8,STR9,
                           STR10,STR11,STR12,STR13,STR14,STR15,STR16,STR17,STR18,STR19,STR20};

void bus_on()
{
   output_high(VDET);
   delay_ms(500);
   bus_pwr_status=1;
}

void bus_off()
{
   output_low(VDET);
   delay_ms(100);
   bus_pwr_status=0;
}

// *** UPLOAD MACRO FUNCTIONS *** //

//This variable holds strings if the print command is used
char string_arg[50];

//If the command is p than this is called rather than get_arg(). 
void get_arg_string(){
   int i = 0;
   for (i = 0; i < 50; i++){
      string_arg[i] = 0;
   }
   
   
   for (i = 0; i < 50; i++)
   {
      string_arg[i] = fgetc(COM_A);
      if(com_echo == TRUE)
      {
         fputc(string_arg[i],COM_A);
      }

      if(string_arg[i] == '\r' || string_arg[i] == '\n')
      {
         break;
      }
   }
}


int16 get_arg()
{
   int8 number;
   int16 macro_arg;
   
   number = 0;
   macro_arg = 0;
   
   while (number != CARRIAGE_RET) {                       // 13 = CR = terminator
      number = fgetc(COM_A);
      if (com_echo == TRUE)
      {
         if (number != CARRIAGE_RET) fputc(number, COM_A);
      }
      if (number > 47 && number < 58)
      {       // ASCII 0 = 48
         number = number - 48;
         macro_arg = macro_arg * 10;                    // increase significance
         macro_arg = macro_arg + number;                // for each number
      }
      else if (number == CARRIAGE_RET)
      {
         if (macro_arg < 65535) return(macro_arg);
         else return(65535);
      }
      else
      {
         return(65535);
      }
   }
}


//Looks like this is causing the exit from the upload
int8 check_macro(int8 e, int8 macro)
{
   int8 valid_macro;
   
   switch(nv_product){
      case ECO :  if (macro == 0) valid_macro = isamong (e, MASTERLIST_ECO);
                  else valid_macro = isamong (e, MACROLIST_ECO);
         break;
      case WMS2 :
      case WMS4 :
                  if (macro == 0) valid_macro = isamong (e, MASTERLIST_WMS);
                  else valid_macro = isamong (e, MACROLIST_WMS);
         break;  
      case AWS :  if (macro == 0) valid_macro = isamong (e, MASTERLIST_AWS);
                  else valid_macro = isamong (e, MACROLIST_AWS);
         break;  
   }
   return(valid_macro);
}
// returns 0 if macro command is invalid
int8 get_cmd(int8 macro)
{
   int8 e;
   int8 valid_macro;
   
   valid_macro = TRUE;
   
   do{
      e=fgetc(COM_A);
      if (com_echo == TRUE) fputc(e,COM_A);
         valid_macro = check_macro(e, macro);
      if (valid_macro == TRUE) {
         return(e);
      }
      else return(0);
   } while (valid_macro == TRUE);
}

void write_macro_line(int8 macro_cmd, int16 macro_arg, int8 macro, int16 line)
{
   int16 addr;
   //line*3 because 3 bytes but line incremented by one each time
   addr = macro_address[macro] + (line*3); 
   
   write_ext_eeprom(addr, macro_cmd);
   
   ++addr;
   
   write16_ext_eeprom(addr, macro_arg);
}

/*writes a string to the macro. The macro cmd goes to the standard memory
address, while the string is stored at a different location stored in the
string_address array*/
void write_macro_line_string(int8 macro_cmd, int8 macro, int16 line, int16 count)
{
   int16 addr;
   
   addr = macro_address[macro] + (line*3);
   write_ext_eeprom(addr, macro_cmd);
   
   int i = 0;
   addr = string_address[macro] + (count*50);
   //fprintf(COM_A,"%Lu\r\n",addr);
   if(count <= 20){
      for(i = 0; i<50; i++) {
         write_ext_eeprom(addr + i, string_arg[i]);
         if(string_arg[i] == 0){
            break;
         }
     
      }
   }else{
      fprintf(COM_A,"MEMORY OVERFLOW ERROR @ Line :%Lu\n\r",line);
   }
   
}


void write_blank_macros()
{
   init_ext_eeprom();
   
   //write_macro_line(int8 macro_cmd, int16 macro_arg, int8 macro, int16 line)
   
   write_macro_line(59, 0, 0, 0);      // 59 = ; (end of macro symbol)
   
   write_macro_line(59, 0, 1, 0);
   write_macro_line(59, 0, 2, 0);
   write_macro_line(59, 0, 3, 0);
   write_macro_line(59, 0, 4, 0);
   write_macro_line(59, 0, 5, 0);
   write_macro_line(59, 0, 6, 0);
   write_macro_line(59, 0, 7, 0);
   write_macro_line(59, 0, 8, 0);
   
   write_macro_line(59, 0, 9, 0);
   write_macro_line(59, 0, 10, 0);
   write_macro_line(59, 0, 11, 0);
   write_macro_line(59, 0, 12, 0);
   write_macro_line(59, 0, 13, 0);
   write_macro_line(59, 0, 14, 0);
   write_macro_line(59, 0, 15, 0);
   write_macro_line(59, 0, 16, 0);
}

void macro_comment()
{
   int8 mcc;
   mcc=0;
   
   heartbeat(FALSE);
   
   while(mcc != CARRIAGE_RET){
      if (kbhit(COM_A)) {
         mcc=fgetc(COM_A);
         if (com_echo == TRUE) fputc(mcc,COM_A);
      }
   }
   
   heartbeat(TRUE);
}

int8 get_macro_line(int8 macro, int16 line, int16 count)
{
   int8  macro_cmd;
   int16 macro_arg;
   int8  valid_macro;
   
   valid_macro = FALSE;
   
   macro_cmd = get_cmd(macro);
   if (macro_cmd == 35){ 
      macro_comment();        // # = 35
   }else if(macro_cmd == 112){// 112 is p
      //fprintf(COM_A,"test\r\n");
      get_arg_string();
      //fprintf(COM_A,"test\r\n");
      write_macro_line_string(macro_cmd, macro, line, count);
      valid_macro = TRUE;
   }else{
      if (macro_cmd != 0) {
         macro_arg = get_arg();
         if (macro_arg != 65535) valid_macro = TRUE;
         else return(macro_cmd);
      }else return(macro_cmd);
      
      write_macro_line(macro_cmd, macro_arg, macro, line);
      return(macro_cmd);
   }
   return(macro_cmd);
}

void upload_macro(int8 macro)
{
   int16 line;
   int8  valid_macro;
   int8  macro_cmd;
   int8  macro_cmd2;
   int16 count = 0;
   
   line = 0;
   valid_macro = TRUE;
   macro_cmd = 1;
   
   init_ext_eeprom();
   
   fprintf(COM_A, ":");
   
   // not a critical hang
   
   while(macro_cmd != 59 && macro_cmd != 0) {
   
      macro_cmd = get_macro_line(macro, line,count);
      /*count keeps track of the location of the strings just like line does.
      Since strings are stored in a seperate piece of memory we use two seperate
      variables.*/
      
      if(macro_cmd == 'p'){
         count++;
      }
      if (macro_cmd != 35) ++line;
      if (macro_cmd != 59) printf("\r\n:");
   }      
   if (macro_cmd == 0){
      fprintf(COM_A, "\r\nWARNING: INVALID COMMAND IN MACRO!\r\n");
      fprintf(COM_A, "Bad command at line: %Ld \r\n",line);
      fprintf(COM_A, "Please re-upload macro");
   }
   fprintf(COM_A, "\r\n");
   
   
   //output_low(EEP_WP);
}

// *** READ MACRO FUNCTIONS *** //

int8 read_macro_line(int8 macro, int16 line)
{
   int16 addr;
   int8  macro_cmd;
   int16 macro_arg;
   
   addr = macro_address[macro] + (line*3);
   
   macro_cmd = read_ext_eeprom(addr);
   ++addr;
   macro_arg = read16_ext_eeprom(addr);
   fprintf(COM_A, "%c%Lu\r\n", macro_cmd, macro_arg);
   
   return(macro_cmd);
}

int8 read_macro_line_silent(int8 macro, int16 line)
{
   int16 addr;
   int8  macro_cmd;
   int16 macro_arg;
   
   addr = macro_address[macro] + (line*3);
   
   macro_cmd = read_ext_eeprom(addr);
   ++addr;
   macro_arg = read16_ext_eeprom(addr);
   fprintf(COM_A, "%c%Lu\r\n", macro_cmd, macro_arg);
   
   return(macro_cmd);
}

/* the corresponding function for strings. See the equivalent write function
above for details*/
void read_macro_line_string(int8 macro, int16 line, int16 count)
{
   int16 addr;
   addr = string_address[macro] + (count*50);
   
   int i = 0;
   for(i = 0; i < 50; i++){
      string_arg[i] = 0;
   } 
   for(i = 0; i < 50; i++){
      string_arg[i] = read_ext_eeprom(addr + i);
      //efficiency
      if(string_arg[i] == 0){
         break;
      }
   }  
   
}


void read_macro(int8 macro)
{
   int16 line;
   int8  macro_cmd;
   int count = 0;
   
   line = 0;   
   macro_cmd = 1;
   
   init_ext_eeprom();
   
   while (line < MACEND && macro_cmd != 59){      // 59 = ;
      macro_cmd = read_macro_line(macro, line);
      if(macro_cmd == 'p'){
         read_macro_line_string(macro,line,count);
         count++;
      }
      ++line;
   }
}   

// *** PLAY MACRO FUNCTIONS *** //

void delay_sec(int16 sec)
{
   int16 j;
   
   for (j=0; j<sec; ++j){
      delay_ms(1000);
   } 
}

int8 step_err_status()
{
   int8 macro_status;
   
   if (m_error[motor] == TRUE)
   {
      macro_status = 'e';
   }
   else
   {
      macro_status = 'a';
   }
   return(macro_status);
}

void get_reading()
{
   if(nv_det_type==1)
   {
       get_det_read(TRUE,TRUE);    // store, display
   }
   else if(nv_det_type==2 || nv_det_type==3)
   {
      read_colorimeter(200,TRUE,TRUE,FALSE);
   }
}

void set_heat(int8 macro_cmd, int16 macro_arg)
{
   if(nv_det_type==1) {
      cmd = macro_cmd;
      arg = macro_arg;
      det_cmd();
   }
   else if(nv_det_type==2 || nv_det_type==3) {
      set_heaters(macro_arg);
   }
}

void set_light(int8 macro_cmd, int16 macro_arg)
{
   //fprintf(COM_A,"%Lu\r\n",nv_det_type);
   if(nv_det_type==1) {
      cmd = macro_cmd;
      arg = macro_arg;
      det_cmd();
   }
   else if(nv_det_type==2 || nv_det_type==3) {
      set_LED(macro_arg);
   }
}

// plays soft macros (search for "hard macro")
int16 count1 = 0;
int8 play_macro_line2(int8 macro, int16 line, int16 mmacro_var)
{
   int16 addr;
   int8  macro_cmd;
   int16 macro_arg;
   int8  macro_status;
   
   
   macro_status = 'e'; // error
   
   addr = macro_address[macro] + (line*3);

   init_ext_eeprom();
   macro_cmd = read_ext_eeprom(addr);
   ++addr;
   //if(macro_cmd == 'p'){
     // read_macro_line_string(macro,line,count1);
     // count1++;
   //}else{
   macro_arg = read16_ext_eeprom(addr);
   //}
   heartbeat(TRUE);
   
   //if (nv_report_mode >= 3) fprintf(COM_A, "@CMD[%c]ARG[%Lu]VAR[%Lu]\r\n", macro_cmd, macro_arg, mmacro_var);
   
   if(nv_product==ECO){
      switch(macro_cmd)
      {
         case 'A' :
            calc_abs_data();
            store_rel_data();
            macro_status = 'a';
            break;
         case 'p' :
            fprintf(COM_A,"%s\r\n",string_arg);
            macro_status = 'a';
            break;
         case 'C' :
            calc_abs_data();
            break;
         case 'D' : //arg = macro_arg; //Test this later
            //commandD();
            delay_sec(macro_arg);
            macro_status = 'a';
            break;
         case 'F' : macro_flag = macro_arg;
                    macro_status = 'a';
            break ;
         // detector commands
         case '/' : detector = macro_arg;
                    macro_status = 'a';
            break ;
         case 'd' : set_heat(macro_cmd,macro_arg);
                    macro_status = 'a';
            break;
         case 'H' : set_heat(macro_cmd,macro_arg);
                    macro_status = 'a';
            break;
         case 'i' : m_stp_int[1] = (macro_arg/100);  // Change syringe motor speed
                    macro_status = 'a';
            break;   
         case 'l' : LightTargetFlag = 1;
                    set_light(macro_cmd,macro_arg);
                    delay_ms(10000);
                    macro_status = 'a';
            break;       
         case 'L' : LightTargetFlag = 0;
                    set_light(macro_cmd,macro_arg);
                    macro_status = 'a';
            break;
         case 'K' : // set detector channel 1 or 2
                    cmd = macro_cmd;
                    arg = macro_arg;
                    det_cmd();
                    macro_status = 'a';
            break ;
         case 'S' : if(macro_arg==1) get_reading();     // store & display
                    if(macro_arg==2) probe_read(TRUE,TRUE);
                    macro_status = 'a';
            break ; 
         case 'T' : det_temp();
                    macro_status = 'a';
            break;
         // stepper commands - move_motor(use dir/shortest,dir,steps);
         case '+' : motor=1;
                    move_motor(0,0,macro_arg,1);
                    macro_status=step_err_status();
            break;
         case '-' : motor=1;
                    move_motor(0,1,macro_arg,1);
                    macro_status=step_err_status();
            break;
         case 'G' : motor=0;
                    align(0);
                    macro_status=step_err_status();
            break ;          
         case 'P' : motor=0;
                    move_motor(1,0,macro_arg,1);  
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
         // bus power ON/OFF
         case 'Y' : if (macro_arg == 1) bus_on();
                    else if (macro_arg == 0) bus_off();
                    macro_status = 'a';
            break;
         case 'v':
            arg = macro_arg;
            command_v();
            macro_status = 'a';
            /*if(macro_arg == 00){
               output_bit(PIN_D0,0);
            }else if(macro_arg == 01){
               output_bit(PIN_D0,1);
            }else if(macro_arg == 10){
               output_bit(PIN_D1,0);
            }else if(macro_arg == 11){
               output_bit(PIN_D1,1);
            }else if(macro_arg == 20){
               output_bit(PIN_D2,0);
            }else if(macro_arg == 21){
               output_bit(PIN_D2,1);
            }else if(macro_arg == 30){
               output_bit(PIN_D3,0);
            }else if(macro_arg == 31){
               output_bit(PIN_D3,1);
            }else if(macro_arg == 40){
               output_bit(PIN_D4,0);
            }else if(macro_arg == 41){
               output_bit(PIN_D4,1);
            }else if(macro_arg == 50){
               output_bit(PIN_D5,0);
            }else if(macro_arg == 51){
               output_bit(PIN_D5,1);
            }else if(macro_arg == 60){
               output_bit(PIN_D6,0);
            }else if(macro_arg == 61){
               output_bit(PIN_D6,1);
            }else if(macro_arg == 70){
               output_bit(PIN_D7,0);
            }else if(macro_arg == 71){
               output_bit(PIN_D7,1);
            }*/     
            break;
         // end of macro   
         case ';' : macro_status = 'f';
            break ;
      }
   }
      
   // macro_status =  97(a): command complete
   //                 98(b): running (not possible)
   //                 99(c): crap response
   //                100(d): time-out
   //                101(e): error
   //                102(f): finished   
   return(macro_status);
}

//Duplicate of above function inserted because recursion not permitted so calls copy of self instead.
int8 play_macro_line(int8 macro, int16* line, int16 mmacro_var)
{
   int16 addr;
   int8  macro_cmd;
   int16 macro_arg;
   int8  macro_status;
   
   
   macro_status = 'e'; // error
   
   addr = macro_address[macro] + ((*line)*3);

   init_ext_eeprom();
   macro_cmd = read_ext_eeprom(addr);
   ++addr;
   if(macro_cmd == 'p'){
      //count1 keeps track of where in the string memory we are.
      read_macro_line_string(macro,line,count1);
      count1++;
   }else{
   macro_arg = read16_ext_eeprom(addr);
   }
   heartbeat(TRUE);
   
  // if (nv_report_mode >= 3) fprintf(COM_A, "@CMD[%c]ARG[%Lu]VAR[%Lu]\r\n", macro_cmd, macro_arg, mmacro_var);
   
   if(nv_product==ECO){
      switch(macro_cmd){
         case 'A' :
            calc_abs_data();
            store_rel_data();
            macro_status = 'a';
            break;
         case 'p' :
            fprintf(COM_A,"%s\r\n",string_arg);
            macro_status = 'a';
            break;
         case 'C' :
            calc_abs_data();
            break;
         //r is the repeat function
         case 'r' :
                  int32 count = 0;
                  //Using a pointer for line so that its value is shared 
                  //accross functions.
                  (*line)++;
                  int32 start_line = (*line);
                  int32 end_line = 0;
                  
                  while(count < macro_arg){
                     char curr = 'A';
                     while(curr != '%'){
                        //This is added in because recursion is not supported.
                        play_macro_line2(macro,(*line),mmacro_var);
                        (*line)++;
                        //not actually silent, dunno why
                        output_low(MD1_TXEN); // Turn off RS232 TX line to stop output of commands
                        curr = read_macro_line_silent(macro,(*line));
                        output_high(MD1_TXEN); // Turn RS232 Tx back on
                     }
                     end_line = (*line);
                     count++;
                     printf("Iterations left: %Lu\r\n",macro_arg - count);
                     (*line) = start_line;
                  }
                  (*line) = end_line + 1;
                  macro_status = 'a';
            break;
         case 'D' : //arg = macro_arg; //Test this later
                    //commandD();
                    delay_sec(macro_arg);
                    macro_status = 'a';
            break;
         case 'F' : macro_flag = macro_arg;
                    macro_status = 'a';
            break ;
         // detector commands
         case '/' : detector = macro_arg;
                    macro_status = 'a';
            break ;
         case 'd' : set_heat(macro_cmd,macro_arg);
                    macro_status = 'a';
            break;
         case 'H' : set_heat(macro_cmd,macro_arg);
                    macro_status = 'a';
            break;
         case 'i' : m_stp_int[1] = (macro_arg/100);  // Change syringe motor speed
                    macro_status = 'a';
            break;   
         case 'l' : LightTargetFlag = 1;
                    set_light(macro_cmd,macro_arg);
                    delay_ms(10000);
                    macro_status = 'a';
            break;       
         case 'L' : LightTargetFlag = 0;
                    set_light(macro_cmd,macro_arg);
                    macro_status = 'a';
            break;
         case 'K' : // set detector channel 1 or 2
                    cmd = macro_cmd;
                    arg = macro_arg;
                    det_cmd();
                    macro_status = 'a';
            break ;
         case 'S' : if(macro_arg==1) get_reading();     // store & display
                    if(macro_arg==2) probe_read(TRUE,TRUE);
                    macro_status = 'a';
            break ; 
         case 'T' : det_temp();
                    macro_status = 'a';
            break;
         // stepper commands - move_motor(use dir/shortest,dir,steps);
         case '+' : motor=1;
                    move_motor(0,0,macro_arg,1);
                    macro_status=step_err_status();
            break;
         case '-' : motor=1;
                    move_motor(0,1,macro_arg,1);
                    macro_status=step_err_status();
            break;
         case 'G' : motor=0;
                    align(0);
                    macro_status=step_err_status();
            break ;          
         case 'P' : motor=0;
                    move_motor(1,0,macro_arg,1);  
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
         // bus power ON/OFF
         case 'Y' : if (macro_arg == 1) bus_on();
                    else if (macro_arg == 0) bus_off();
                    macro_status = 'a';
            break;
         case 'v': 
            arg = macro_arg;
            command_v();
            macro_status = 'a';
            /*if(macro_arg == 00){
               output_bit(PIN_D0,0);
            }else if(macro_arg == 01){
               output_bit(PIN_D0,1);
            }else if(macro_arg == 10){
               output_bit(PIN_D1,0);
            }else if(macro_arg == 11){
               output_bit(PIN_D1,1);
            }else if(macro_arg == 20){
               output_bit(PIN_D2,0);
            }else if(macro_arg == 21){
               output_bit(PIN_D2,1);
            }else if(macro_arg == 30){
               output_bit(PIN_D3,0);
            }else if(macro_arg == 31){
               output_bit(PIN_D3,1);
            }else if(macro_arg == 40){
               output_bit(PIN_D4,0);
            }else if(macro_arg == 41){
               output_bit(PIN_D4,1);
            }else if(macro_arg == 50){
               output_bit(PIN_D5,0);
            }else if(macro_arg == 51){
               output_bit(PIN_D5,1);
            }else if(macro_arg == 60){
               output_bit(PIN_D6,0);
            }else if(macro_arg == 61){
               output_bit(PIN_D6,1);
            }else if(macro_arg == 70){
               output_bit(PIN_D7,0);
            }else if(macro_arg == 71){
               output_bit(PIN_D7,1);
            }*/     
            break;
         // end of macro   
         case ';' : macro_status = 'f';
            break ;
      }
   }
   else if (nv_product==WMS4 || nv_product==WMS2){
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
         case 'P' : motor=0;
                    move_motor(0,0,mmacro_var,1);  
                    macro_status=step_err_status();
            break;
         case 'Q' : motor=0;
                    move_motor(0,0,mmacro_var,1);
                    macro_status=step_err_status();
            break;
         case 'R' : motor=0;
                    move_motor(0,1,mmacro_var,1);
                    macro_status=step_err_status();
            break;
         // end stepper commands
         case 'T' : delay_sec(macro_arg);
                    macro_status = 'a';
            break;
      case 'Y'    : if (macro_arg == 1) bus_on();
                    else if (macro_arg == 0) bus_off();
                    macro_status = 'a';
            break;
         case ';' : macro_status = 'f';
            break;           
      }
   }
   else if (nv_product==AWS){
      switch(macro_cmd){
         // stepper commands - move_motor(use dir/shortest,dir,steps);
         case '+' : motor=1;
                    move_motor(0,0,macro_arg,1);
                    macro_status=step_err_status();
            break;
         case '-' : motor=1;
                    move_motor(0,1,macro_arg,1);
                    macro_status=step_err_status();
            break;
         // end stepper commands
         case 'C' : sol_chip_cmd(macro_arg);
                    macro_status = 'a'; 
            break;         
         case 'D' : delay_sec(macro_arg);
                    macro_status = 'a';
            break;
         case 'P' : sol_switch(mmacro_var);
                    macro_status = 'a';
            break;             
         case 'Q' : sol_switch(macro_arg);
                    macro_status = 'a';
            break;            
         case 'S' : sol_switch_cmd(macro_arg);
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
   }   
   // macro_status =  97(a): command complete
   //                 98(b): running (not possible)
   //                 99(c): crap response
   //                100(d): time-out
   //                50(e): error
   //                102(f): finished   
   return(macro_status);
}


// pass macro number and variable used in macro
void play_macro(int8 macro, int16 mmacro_var)
{
   int16 line;
   int8  macro_status;
   count1=0;
   
   clear_time_stmp_str(); // function store_rel_data() uses this to test for a reading
   clear_data_arrays();
   
   line = 0; 
   macro_status = 'a';
   data_available = FALSE;
   macro_flag = 900;
   
   sprintf(event_str, ",macro[%u],start\r\n", macro);
   record_event();   
  
  // moved to below macro execution for proper port labeling
  /* switch(nv_product){
      case WMS4 : 
      case WMS2 : store_wms_data(macro);
         break;
      case AWS : store_aws_data(macro,mmacro_var);
         break;
   } */
   
   
   // runs until line > end of memory AND macro status is 
   // ... NOT finished or something bad
   while (line < MACEND && macro_status == 'a')
   {      
      if(nv_report_mode == 4) fprintf (COM_A, "@MCL[%c][%Lu]\r\n", 
                                               macro_status, (line + 1));
      macro_status = play_macro_line(macro, &line, mmacro_var);
      ++line;
     
      if (user_quit == TRUE) {           // 17 = CTRL-Q (quit)
         macro_status = 'g';
         break;   
      }
   }
   
   // writes data to uart and SD card
   switch(nv_product){
      case WMS4 : 
      case WMS2 : store_wms_data(macro);
         break;
      case AWS : store_aws_data(macro,mmacro_var);
         break;
   }
   
   switch(macro_status){
      case 'a': fprintf(COM_A, "@LNE\r\n");
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
      case 'g': fprintf(COM_A, "@HLT\r\n");
         break;          
   }
   
   sprintf(event_str, ",macro[%u],status[%c],line[%Lu]\r\n", 
                           macro, macro_status, line);
   record_event();
   
//   if (nv_product==ECO) calculate_data();  Now only done with A command VK 10/19/2015
   
   // f & g are the only two natural ways to end a macro
   // if error then recover the situation
   if (macro_status != 'f' && macro_status != 'g') recovery();
}

int32 master_macro_loop_count = 0;
int32 master_macro_loop_start = 0;

int8 master_macro_eco(){
   int16 addr;
   int8  macro_cmd;
   int16 macro_arg;
   int8  macro_batch;   
   macro_batch = TRUE;   
   while (macro_batch == TRUE){
      // the step in the master macro (including loops) is passed from the caller
      addr = macro_address[0] + (nv_macro_step*3);
      init_ext_eeprom();
      // get the sub-macro
      macro_cmd = read_ext_eeprom(addr);
      ++addr;
      macro_arg = read16_ext_eeprom(addr);
      ++addr;
      ++addr;     
      //if (nv_report_mode >= 3) fprintf(COM_A, "@MAC[%c]ARG[%Lu]\r\n", macro_cmd, macro_arg);
      
      if (macro_cmd == 'M' && macro_arg>0) // VK 5/22/17 Only ouput on M commands
         {
         RTC_reset_HT();
         RTC_read();
         RTC_display();
         
         fprintf(COM_A, "@Running %c%Lu \r\n", macro_cmd,macro_arg);
         fprintf(COM_A, "@Macro %04Lu of %04Lu\r\n", (nv_sample+1),nv_max_samples);
         }
      
      if (nv_sample >= nv_max_samples)
         {macro_cmd = 'Z';}
      
         // run macro, sleep, end ....
         switch (macro_cmd) {
            //case 'p':
              // fprintf(COM_A, "test command \r\n");
               //++nv_macro_step;
               //break;
            case 'r':
               ++nv_macro_step;
               master_macro_loop_start = nv_macro_step;
               master_macro_loop_count = macro_arg;
            break;
            case '%':
               if (master_macro_loop_count > 1){
                  nv_macro_step = master_macro_loop_start;
                  master_macro_loop_count--;
               }else{
                  nv_macro_step++;
               }
            break;
            case 'M' :
               // run sub-macro (if valid)
               if (macro_arg < 21) {
                  play_macro(macro_arg,0);     // casts to int8
                  delay_ms(100);
                  ++nv_sample;
                  ++nv_macro_step;
               }
               break;
            case 'Z' : ++nv_macro_step;
                       macro_batch = FALSE;
               break;
            case ';' : nv_macro_step = 0;
                       // macro_batch = FALSE;
               break;
            default  : cmd_err();
                       macro_cmd = 'e';
                       macro_batch = FALSE;
               break;
         }
         if (user_quit == TRUE) {
            macro_cmd = ';';
            break;         // 17 = CTRL-Q (quit)
         }
   }
   return (macro_cmd);
}


