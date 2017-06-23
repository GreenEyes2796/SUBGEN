void init_valve_status()
{
   int v;
   
   for(v=0; v<33; ++v) valve_status[v]=0;
}

void init_aws()
{
   if(nv_product==AWS)set_tris_h(0b00000000);
   init_valve_status();
   bus_pwr_status=0;
}

void display_valve_status()
{
   int8 v;
   
   fprintf(COM_A, "@VAL ");
   for(v=1; v<9; ++v) fprintf(COM_A, "%02u[%u] ",v,valve_status[v]);
   fprintf(COM_A, "\r\n");
   
   fprintf(COM_A, "@VAL ");
   for(v=9; v<17; ++v) fprintf(COM_A, "%02u[%u] ",v,valve_status[v]);
   fprintf(COM_A, "\r\n");
   
   fprintf(COM_A, "@VAL ");
   for(v=17; v<25; ++v) fprintf(COM_A, "%02u[%u] ",v,valve_status[v]);
   fprintf(COM_A, "\r\n");
   
   fprintf(COM_A, "@VAL ");
   for(v=25; v<33; ++v) fprintf(COM_A, "%02u[%u] ",v,valve_status[v]);
   fprintf(COM_A, "\r\n");   
}

void sol_switch_cmd(int16 setting)
{
   if (nv_report_mode == 4) fprintf(COM_A, "@SOL[/1][s][%Lu]\r\n", setting);

   multidrop_on();
      fprintf(COM_B, "/1s%Lu\r",setting);
   multidrop_off();

   delay_ms(100);
}

void sol_chip_cmd(int8 chip)
{
   if (nv_report_mode == 4) fprintf(COM_A, "@SOL[/1][c][%u]\r\n", chip);
   
   multidrop_on();
      fprintf(COM_B, "/1c%u\r",chip);
   multidrop_off();
   delay_ms(100);

   multidrop_on();
     fprintf(COM_B, "/1s256\r");
   multidrop_off(); 
   delay_ms(100);
}

void sol_switch(int8 valve)
{
   switch(valve){
      case 0 : sol_chip_cmd(1);
               sol_switch_cmd(0);
               sol_chip_cmd(2);
               sol_switch_cmd(0);
               sol_chip_cmd(3);
               sol_switch_cmd(0);
               sol_chip_cmd(4);
               sol_switch_cmd(0);
               init_valve_status();
         break;
      // Bank 1
      case 1 : sol_chip_cmd(1);
               sol_switch_cmd(1);
         break;    
      case 2 : sol_chip_cmd(1);
               sol_switch_cmd(2);
         break;          
      case 3 : sol_chip_cmd(1);
               sol_switch_cmd(4);
         break; 
      case 4 : sol_chip_cmd(1);
               sol_switch_cmd(8);
         break; 
      case 5 : sol_chip_cmd(1);
               sol_switch_cmd(16);
         break; 
      case 6 : sol_chip_cmd(1);
               sol_switch_cmd(32);
         break; 
      case 7 : sol_chip_cmd(1);
               sol_switch_cmd(64);
         break; 
      case 8 : sol_chip_cmd(1);
               sol_switch_cmd(128);
         break;  
      // Bank 2
      case 9 : sol_chip_cmd(2);
               sol_switch_cmd(1);
         break;    
      case 10 : sol_chip_cmd(2);
               sol_switch_cmd(2);
         break;          
      case 11 : sol_chip_cmd(2);
               sol_switch_cmd(4);
         break; 
      case 12 : sol_chip_cmd(2);
               sol_switch_cmd(8);
         break; 
      case 13 : sol_chip_cmd(2);
               sol_switch_cmd(16);
         break; 
      case 14 : sol_chip_cmd(2);
               sol_switch_cmd(32);
         break; 
      case 15 : sol_chip_cmd(2);
               sol_switch_cmd(64);
         break; 
      case 16 : sol_chip_cmd(2);
               sol_switch_cmd(128);
         break;  
      // Bank 3
      case 17 : sol_chip_cmd(3);
               sol_switch_cmd(1);
         break;    
      case 18 : sol_chip_cmd(3);
               sol_switch_cmd(2);
         break;          
      case 19 : sol_chip_cmd(3);
               sol_switch_cmd(4);
         break; 
      case 20 : sol_chip_cmd(3);
               sol_switch_cmd(8);
         break; 
      case 21 : sol_chip_cmd(3);
               sol_switch_cmd(16);
         break; 
      case 22 : sol_chip_cmd(3);
               sol_switch_cmd(32);
         break; 
      case 23 : sol_chip_cmd(3);
               sol_switch_cmd(64);
         break; 
      case 24 : sol_chip_cmd(3);
               sol_switch_cmd(128);
         break;     
      // Bank 4
      case 25 : sol_chip_cmd(4);
               sol_switch_cmd(1);
         break;    
      case 26 : sol_chip_cmd(4);
               sol_switch_cmd(2);
         break;          
      case 27 : sol_chip_cmd(4);
               sol_switch_cmd(4);
         break; 
      case 28 : sol_chip_cmd(4);
               sol_switch_cmd(8);
         break; 
      case 29 : sol_chip_cmd(4);
               sol_switch_cmd(16);
         break; 
      case 30 : sol_chip_cmd(4);
               sol_switch_cmd(32);
         break; 
      case 31 : sol_chip_cmd(4);
               sol_switch_cmd(64);
         break; 
      case 32 : sol_chip_cmd(4);
               sol_switch_cmd(128);
         break;
   }
   if(valve>0&&valve<33) valve_status[valve]=1;  
   if(nv_report_mode==4)display_valve_status();
   
}





