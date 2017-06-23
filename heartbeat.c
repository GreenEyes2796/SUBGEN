/* 
   Stops timer isr from servicing WD. If heartbeat(TRUE) is not called
   within 4 seconds WD will bark (=reset_cpu)
*/
void suspend_heartbeat()
{
   disable_interrupts(INT_TIMER0);
   restart_wdt();
}

//enable timer1 int, clear t1 int, pre-set timer
void setup_led_pulse()
{
   set_timer1(0);
   output_high(BOARD_LED);                // LED is turned OFF by timer1_isr()
   if(nv_product==AWS)output_high(PANEL_LED); 
   clear_interrupt(INT_TIMER1);
   enable_interrupts(INT_TIMER1);
}

/*
   Restarts servicing of WD with timer0
*/
void restart_heartbeat()
{
   // pre-sets timer0, resets WDT, LED ON, clear t0 int
   set_timer0(TIMER0_PRESET);
   restart_wdt();
   clear_interrupt(INT_TIMER0);
   enable_interrupts(INT_TIMER0);
   setup_led_pulse();
}

void heartbeat(int8 beating)
{
   switch(beating){
      case 0 : suspend_heartbeat();
         break;
      case 1 : restart_heartbeat();
         break;
   }
}

void kill_wd()
{
   disable_interrupts(INT_TIMER0);
   setup_wdt(WDT_OFF);
}

void start_heartbeat()
{
   set_timer0(TIMER0_PRESET);     // Preset e.g. 3036 for 1.000000 sec intervals
   clear_interrupt(INT_TIMER0);
   enable_interrupts(INT_TIMER0);
   setup_wdt(WDT_ON);
}


