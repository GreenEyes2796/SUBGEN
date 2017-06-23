/* Downloads firmware over RS232 and writes it to flash 
 
   Based on the CCS supplied example loader.c, modified to only work 
   for 18 series 
 
   After each good line, the loader sends an ACKLOD character.  The 
   driver uses XON/XOFF flow control.  Also, any buffer on the PC  
   UART must be turned off, or to its lowest setting, otherwise it 
   will miss data.  
*/ 

// Some useful debugging lines 
//fprintf(SENSM_STRM,"Loader End 0x%lx, Loader Size 0x%lx, Loader Addr 0x%lx\r", LOADER_END, LOADER_SIZE, LOADER_ADDR); 
//fprintf(SENSM_STRM,"Flash erase size 0x%lx, Flash write size 0x%lx\r", getenv("FLASH_ERASE_SIZE"), getenv("FLASH_WRITE_SIZE")); 

// Define the size of the loader in ROM and the address to write it to 
//#ifndef LOADER_END 
   #define LOADER_END   getenv("PROGRAM_MEMORY")-1                         ///< Get the end of the program memory and put the loader there 
   #define LOADER_SIZE  0x7FF                                              ///< Size of the loader functions 
//#endif 
#define LOADER_ADDR     ( LOADER_END - LOADER_SIZE )                       ///< Address of the loader 

// Set all the functions following this directive to be included in the 
// loader ROM area 
#pragma ORG LOADER_ADDR+10, LOADER_END default

// Serial port stream specific to this area to make the compiler create 
// specific specific serial functions stored in the #ORG 
#pragma use rs232(baud=19200, parity=N, UART1, bits=8, stream=LOADER_STRM) 

// Definitions 
#define BUFFER_LEN_LOD  64     //< Length of a line in an Intel 8-bit hex file 
#define ACKLOD 0x06            //< Acknowledge the last line 
#define XON    0x11            //< Turn transmission on 
#define XOFF   0x13            //< Turn transmission off 

#define uint1_t unsigned short
#define uint8_t unsigned int
#define int8_t  signed int
#define char_t  char
#define uint16_t unsigned long 
#define uint32_t unsigned long long

//****************************************************************************** 
/// Convert two hex chars to a byte 
/*! 
* \param[in] s String 2 chars long 
* \return Byte value from hex 
*/ 
#pragma SEPARATE                  // The SEPARATE directive tells the compiler not to inline this function, this reduces the ROM space required 
uint8_t atoi_b16(char_t *s) 
{  
   uint8_t result = 0; 
   uint8_t i; 

   for (i=0; i<2; i++,s++) 
   { 
      if (*s >= 'A') 
      { 
         result = 16*result + (*s) - 'A' + 10; 
      } 
      else 
      { 
         result = 16*result + (*s) - '0'; 
      } 
   } 

   return(result); 
} 

//****************************************************************************** 
/// Copy of the string compare function 
/*! 
* This does not get inlined by the compiler so I have made a copy of the CCS supplied 
* library function that gets included in the #org section 
* \param[in] s1 Pointer to the first string 
* \param[in] s2 Pointer to the second string 
* \param[in] n Number of characters to compare 
* \return 0 for equal, negative or positive for not equal 
*/ 
int8_t ldr_strncmp(char_t *s1, char_t *s2, uint8_t n) 
{ 
   for (; n > 0; s1++, s2++, n--) 
   { 
      if (*s1 != *s2) return((*s1 <*s2) ? -1: 1); 
      else if (*s1 == '\0') return(0); 
   } 
   return(0); 
} 

//****************************************************************************** 
/// The firmware loader 
/*! 
* Real load function could be sat anywhere inside the #org area 
*/ 
void real_load_program (void) 
{ 
   uint1_t  do_ACKLOD, done=FALSE; 
   uint8_t  checksum, line_type, dataidx, i, count, buffidx; 
   uint16_t l_addr, h_addr=0; 
   uint32_t addr; 
    
   // Buffers 
   uint8_t  data[32]; 
   uint8_t buffer[BUFFER_LEN_LOD]; 

   // Only required for parts where the flash erase and write sizes are different
   #if (getenv("FLASH_ERASE_SIZE") > getenv("FLASH_WRITE_SIZE")) 
      uint32_t next_addr = 0; 
   #endif 
   
   setup_uart(19200, LOADER_STRM);
   delay_ms(5);

   while (!done)  // Loop until the entire program is downloaded 
   { 
      buffidx = 0;  // Read into the buffer until 0x0D ('\r') is received or the buffer is full 
      do 
      { 
         buffer[buffidx] = fgetc(LOADER_STRM); 
      } while ( (buffer[buffidx++] != 0x0D) && (buffidx <= BUFFER_LEN_LOD) ); 

      fputc(XOFF, LOADER_STRM);                                                  // Suspend sender 
      do_ACKLOD = TRUE;                                                          // Flag to indicate this is a sentence we should acknowledge 

      // Only process data blocks that start with ':' 
      if (buffer[0] == ':') 
      { 
         count = atoi_b16(&buffer[1]);                                           // Get the number of bytes from the buffer 
         l_addr = make16(atoi_b16(&buffer[3]),atoi_b16(&buffer[5]));             // Get the lower 16 bits of address 
         line_type = atoi_b16(&buffer[7]);                                       // Get the line type code from the string 
         addr = make32(h_addr,l_addr);                                           // At the first time through h_addr is zero as we are assuming the high bytes of the addr are zero until we get a type 4 command 

         if (line_type == 1)                                                     // If the line type is 1, then data is done being sent 
         { 
            done = TRUE; 
            fprintf(LOADER_STRM,"DN\r"); 
            do_ACKLOD = FALSE; 
         } 
         else if ((addr < LOADER_ADDR || addr > LOADER_END) && addr < 0x300000)  // Don't try to overwrite the loader 
         { 
            checksum = 0;                                                        // Sum the bytes to find the check sum value 
            for (i=1; i<(buffidx-3); i+=2) 
            { 
               checksum += atoi_b16 (&buffer[i]); 
            } 
            checksum = 0xFF - checksum + 1; 

            if (checksum != atoi_b16 (&buffer[buffidx-3])) 
            { 
               fprintf(LOADER_STRM,"CS\r");                                      // Test the CheckSum and report failure 
               do_ACKLOD = FALSE; 
            } 
            else 
            { 
               if (line_type == 0) 
               { 
                  // Loops through all of the data and stores it in data 
                  // The last 2 bytes are the check sum, hence buffidx-3 
                  for (i = 9,dataidx=0; i < buffidx-3; i += 2) 
                  { 
                     data[dataidx++] = atoi_b16(&buffer[i]); 
                  } 

                  #if (getenv("FLASH_ERASE_SIZE") > getenv("FLASH_WRITE_SIZE")) 
                     fprintf(LOADER_STRM,"ES\r"); 
                     if ((addr!=next_addr)&&(addr&(getenv("FLASH_ERASE_SIZE")/2-1)!=0)) 
                     { 
                        erase_program_eeprom(addr); 
                     } 
                     next_addr = addr + 1; 
                  #endif 

                  fprintf(LOADER_STRM,"WR, 0x%lx, %u, ", addr, count); 
                  write_program_memory(addr, data, count);                             // Attempt a write to the program memory 

                  read_program_memory(addr, buffer, count);                            // Read the program memory we just wrote into the incoming string buffer to avoid having two data buffers 
                  if( ldr_strncmp(data, buffer, count) == 0) fprintf(LOADER_STRM,"ACK\r"); // Test the data data 
                  else fprintf(LOADER_STRM,"NACK\r"); 
                  do_ACKLOD = FALSE; 
               } 
               else if (line_type == 4) 
               { 
                  h_addr = make16(atoi_b16(&buffer[9]), atoi_b16(&buffer[11])); 
                  fprintf(LOADER_STRM,"HA, 0x%x\r", h_addr); 
                  do_ACKLOD = FALSE; 
               } 
            } 
         } 
      } 

      if (do_ACKLOD)                     // Only do this for sentences we have not already responded to 
      { 
         fputc(ACKLOD, LOADER_STRM); 
      } 
      fputc(XON, LOADER_STRM);           // Renable transmission from the terminal program 
      //restart_wdt(); 
   } 

   fputc(ACKLOD, LOADER_STRM); 
   fputc(XON, LOADER_STRM); 

   printf("\r\n@FWU\r\n\n");
   delay_ms(4000);
   reset_cpu();                          // After writing a new program we always want to reset the CPU
} 

// This #ORG ends the section holding the loader (default causes all functions within 
// the declaration to be put in the ROM section) 
#pragma ORG default 

//****************************************************************************** 
/// Stub load function 
/*! 
* Set a stub function at a specific address so we can jump to it by changing the PC 
* We must always use this as the new application version that overwrites the code won't 
* necessarily have the same layout 
*/ 
#pragma ORG LOADER_ADDR, LOADER_ADDR+9 
void load_program(void) 
{ 
   real_load_program(); 
} 

