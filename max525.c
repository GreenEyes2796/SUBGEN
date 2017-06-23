
void DAC_set(int8 channel, int16 setting)
{
   int16 data;
   int8 DAC_MS_byte;
   int8 DAC_LS_byte;
   
   setup_spi2(SPI_MASTER|SPI_H_TO_L|SPI_CLK_DIV_64);
   
   data = 0;
   
   DAC_MS_byte = 0;
   DAC_LS_byte = 0;
   
   data = setting & 0b0000111100000000;
   data = data >> 8;
   DAC_MS_byte = data;
   
   data = setting & 0b0000000011111111;
   DAC_LS_byte = data;

   // ChA = 0011xxxx
   // ChB = 0111xxxx
   // ChC = 1011xxxx
   // ChD = 1111xxxx
   
   switch(channel)
   {
      case 1: (DAC_MS_byte = DAC_MS_byte | 0b00110000);
         break;
      case 2: (DAC_MS_byte = DAC_MS_byte | 0b01110000);
         break;
      case 3: (DAC_MS_byte = DAC_MS_byte | 0b10110000);
         break;
      case 4: (DAC_MS_byte = DAC_MS_byte | 0b11110000);
         break;
   }

   output_bit(DAC_CL, DISABLE);  // CL HIGH
   output_bit(DAC_CS, ENABLE);   // CS-DAC LOW

   spi_write2(DAC_MS_byte);
   spi_write2(DAC_LS_byte);

   output_bit(DAC_CS, DISABLE);   // CS-DAC LOW
}





