void reset_event()
{
   switch ( restart_cause() )
   {
      case WDT_TIMEOUT:
      {
         printf("@WDT\r\n");
         sprintf(event_str, ",restart cause,WD time-out\r\n");
         record_event();
         recovery();
         break;
      }
      case WDT_FROM_SLEEP:
      {
         printf("@WDS\r\n");
         sprintf(event_str, ",restart cause,WD from sleep\r\n");
         record_event();
         recovery();
         break;
      }      
      case NORMAL_POWER_UP:
      {
         printf("@NPU\r\n");
         sprintf(event_str, ",id#[%Lu],power applied,SD initialized\r\n", nv_serial);
         record_event();
         break;
      }
      case RESET_INSTRUCTION:
      {
         printf("@RST\r\n");
         sprintf(event_str, ",restart cause,reset instruction\r\n");
         record_event();
         break;
      }
      case BROWNOUT_RESTART:
      {
         printf("@BOR\r\n");
         sprintf(event_str, ",restart cause,brown-out\r\n");
         record_event();
         recovery();
         break;
      } 
      case MCLR_FROM_SLEEP:
      {
         printf("@MRS\r\n");
         sprintf(event_str, ",restart cause,MCLR from sleep\r\n");
         record_event();
         recovery();
         break;
      }   
      case MCLR_FROM_RUN:
      {
         printf("@MRR\r\n");
         sprintf(event_str, ",restart cause,MCLR when running\r\n");
         record_event();
         recovery();
         break;
      }        
   }
}
