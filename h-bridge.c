// *** SWITCHING VALVE MOTOR *** //

void HB_all_OFF(){
  output_bit(HB_IN1,OFF);          
  output_bit(HB_IN2,OFF);
  output_bit(HB_EN, OFF);           
  delay_ms(20);
  output_bit(VMOT,  OFF);
  output_bit(VHBRDG,OFF);
}

void HB_m1_ON(){
  output_bit(VHBRDG,ON);
  output_bit(VMOT,  ON);
  delay_ms(50);
  output_bit(HB_IN1,ON);          
  output_bit(HB_IN2,OFF);
  output_bit(HB_EN, ON);           
}

void HB_m2_ON(){
  output_bit(VHBRDG,ON);
  output_bit(VMOT,  ON);
  delay_ms(50);
  output_bit(HB_IN1,OFF);          
  output_bit(HB_IN2,ON);
  output_bit(HB_EN, ON);  
}

void h_bridge(int8 hb_cmd)
{
   switch (hb_cmd){
      case 0:  HB_all_OFF();
         break;
      case 1:  HB_m1_ON();
         break;
      case 2:  HB_m2_ON();
         break;
   }
}

