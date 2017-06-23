void ADC_average(int16 scans)
{
   int8 n;
   int16 j;
   int32 ADC_mean[8] = {0,0,0,0,0,0,0,0};
   int32 ADC_last_sample[8] = {0,0,0,0,0,0,0,0};
   
   for (j=1; j<(scans+1); ++j)
   {
      for (n=1; n<5; ++n)
      {
         set_adc_channel(n,VSS);
         ADC_last_sample[n] = read_adc();
         ADC_mean[n] = (ADC_mean[n] + ADC_last_sample[n]);
      }
   }
   
   for (n=1; n<5; ++n)
   {
      ADC_val[n] = (ADC_mean[n]/scans);
   }
}


