void blip()
{
   output_bit(BOARD_LED, ON);
   if(nv_product==AWS)output_bit(PANEL_LED, ON); 
   delay_ms(32);
   output_bit(BOARD_LED, OFF);
   if(nv_product==AWS)output_bit(PANEL_LED, OFF); 
   delay_ms(32);
}

void signon()
{
   switch(nv_product){
      case ECO : fprintf(COM_A, "ECO-2 %s\r\n", VERSION);
         break;
      case WMS4 : fprintf(COM_A, "WMS-4-SD %s\r\n", VERSION);
         break; 
      case AWS : fprintf(COM_A, "AWS-1 %s\r\n", VERSION);
         break;
      case WMS2 : fprintf(COM_A, "WMS-4-QE %s\r\n", VERSION);
         break;          
      default : fprintf(COM_A, "XXX-0 %s\r\n", VERSION);
         break;
   }
}

void cmd_inv()
{
   fprintf(COM_A, "@INV\r\n");
}

void cmd_arg()
{
   fprintf(COM_A, "@ARG\r\n");
}

void cmd_err()
{
   fprintf(COM_A, "@ERR\r\n");
}

void cmd_ok()
{
   fprintf(COM_A, "@OK!\r\n");
}

void cmd_res()
{
   fprintf(COM_A, "@RES\r\n");
}

void msg_card_ok()
{
   fprintf(COM_A, "@SD1\r\n");
}

void msg_card_fail()
{
   fprintf(COM_A, "@SD_\r\n");
}

void msg_max()
{
   fprintf(COM_A, "@MAX\r\n");
}

void msg_busy()
{
   if (busy_status == TRUE) fprintf(COM_A, "?@BSY\r\n");
   else fprintf(COM_A, "@RDY\r\n");
}

