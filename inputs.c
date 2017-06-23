#include <ctype.h>
#include <stdlib.h>

void get_string(char* s, unsigned int8 max) {
   unsigned int8 len;
   char c;

   --max;
   len=0;
   do {
     c=getc();
     if(c==8) {  // Backspace
        if(len>0) {
          len--;
          putc(c);
          putc(' ');
          putc(c);
        }
     } else if ((c>=' ')&&(c<='~'))
       if(len<=max) {
         s[len++]=c;
         putc(c);
       }
   } while(c!=13);
   s[len]=0;
}

// stdlib.h is required for the ato_ conversions
// in the following functions

signed int8 get_int() {
  char s[5];
  signed int8 i;

  get_string(s, 5);

  i=atoi(s);
  return(i);
}

signed int16 get_long() {
  char s[7];
  signed int16 l;

  get_string(s, 7);
  l=atol(s);
  return(l);
}

float get_float() {
  char s[20];
  float f;

  get_string(s, 20);
  f = atof(s);
  return(f);
}


