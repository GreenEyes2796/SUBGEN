//   media_io.h include file with low level SD/MMC Card I/O functions

#ifndef _DISKIF

///////////////////////////////////////////////////////////////////////////
//      Debug Defines
///////////////////////////////////////////////////////////////////////////
//#define SD_INIT_TRACE TRUE

///////////////////////////////////////////////////////////////////////////

   // Define the SD/MMS assignments
#define SelectSD bit_clear(SD_CS) 
#define DeselectSD bit_set(SD_CS) 

typedef unsigned char   DSTATUS;
typedef unsigned char   DRESULT;

#ifndef DWORD
   #define DWORD int32
#endif

#ifndef WORD
   #define WORD int16
#endif

enum FAT_type {FatUnknown, Fat12, Fat16, Fat32};
enum Card_type {None, Unknown, MMC, SDv1, SDSC, SDHC};

///////////////////////////////////////////////////////////////////////////
//   External platform specific user function to return a FAT Time
///////////////////////////////////////////////////////////////////////////
//   31-25: Year(0-127 +1980), 24-21: Month(1-12), 20-16: Day(1-31)
//   15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2)
///////////////////////////////////////////////////////////////////////////
DWORD get_fattime(void);


///////////////////////////////////////////////////////////////////////////
// Prototypes for disk control functions 
///////////////////////////////////////////////////////////////////////////
#define SD_spi_read spi_read
#define SD_spi_write spi_write

#separate
DSTATUS disk_initialize(void);

#separate
DSTATUS disk_shutdown(void);

#separate
DSTATUS disk_status(void);

#separate
DRESULT disk_read (BYTE* Buffer, DWORD SectorNumber, BYTE SectorCount);

#separate
BYTE SD_cmd(BYTE sdcmd, DWORD address);

#ifndef   _READONLY
   #separate
   DRESULT disk_write (BYTE *Buffer, DWORD SectorNumber, BYTE SectorCount);
#endif
#separate
DRESULT disk_ioctl (BYTE ctrl, void *buff);

#separate
BOOLEAN SD_write_data(BYTE *ptr, BYTE token);

BYTE wait_ready (void);

///////////////////////////////////////////////////////////////////////////
//      Define constants
///////////////////////////////////////////////////////////////////////////
#define SD_MAX_DIR_NAME_LENGTH      64         // maximum directory name length
   
   // Results of Disk Functions (DRESULT) 
#define RES_OK         0      // Successful 
#define   RES_ERROR      1      // R/W Error 
#define   RES_WRPRT      2      // Write Protected 
#define   RES_NOTRDY      3      // Not Ready 
#define   RES_PARERR      4      // Invalid Parameter 


   // Disk Status Bits (DSTATUS) 
#define STA_NOINIT      0x01   // Drive not initialized 
#define STA_NODISK      0x02   // No medium in the drive 
#define STA_PROTECT      0x04   // Write protected 

// Disk Response Codes
#define SD_GOOD_RESPONSE         0x00
#define SD_IN_IDLE_STATE         0x01
#define SD_INVALID_BLOCK_SIZE      0x40
#define SD_INVALID_RESPONSE         0xFF

   // Command code for disk_ioctrl() 
   // Generic Commands
#define CTRL_SYNC         0
#define GET_SECTOR_COUNT   1
#define GET_SECTOR_SIZE      2
#define GET_BLOCK_SIZE      3
   // SD/MMC commands
#define SD_GET_TYPE         10
#define SD_GET_CSD         11
#define SD_GET_CID         12
#define SD_GET_OCR         13
#define SD_GET_SDSTAT      14
   // ATA/CF commands
#define ATA_GET_REV         20
#define ATA_GET_MODEL      21
#define ATA_GET_SN         22


// Define SD command constants
#define SD_CMD_GO_IDLE_STATE      0x40   // CMD0
#define SD_CMD_SEND_OP_COND         0x41   // CMD1
#define SD_CMD_SEND_IF_COND         0x48   // CMD8
#define SD_CMD_SEND_CSD            0x49   // CMD9
#define SD_CMD_SEND_CID            0x4A   // CMD10
#define SD_CMD_STOP_TX            0x4C   // CMD12
#define SD_CMD_SEND_STATUS         0x4D   // CMD13
#define SD_CMD_SET_BLOCKLEN         0x50   // CMD16
#define SD_CMD_READ_BLOCK         0x51   // CMD17
#define SD_CMD_READ_MULTIBLOCK      0x52   // CMD18
#define SD_CMD_SET_BLOCK_COUNT      0x57   // CMD23 (MMC)
#define SD_CMD_WRITE_BLOCK         0x58   // CMD24
#define SD_CMD_WRITE_MULTIBLOCK      0x59   // CMD25
#define SD_CMD_SD_SEND_OP_COND      0x69   // CMD41
#define SD_CMD_APPL_CMD            0x77   // CMD55 SD application command prefix
#define SD_CMD_SEND_OCR            0x7A   // CMD58


#define SD_ACMD13               0x4D    // ACMD13 SD_STATUS (SDC)
#define SD_ACMD23               0x57   // ACMD23 SET_WR_BLK_ERASE_COUNT (SDC)
#define SD_ACMD41               0x69   // ACMD41 SEND_OP_COND (SDC)


#define SD_BlockSize 512
#define MaxSectorCount 1

///////////////////////////////////////////////////////////////////////////
//      Declare Driver Variables
///////////////////////////////////////////////////////////////////////////
Card_type Card = None;
FAT_type fat;            // type of FAT system installed
DWORD block_size;          // current MMC block size
static volatile DSTATUS Media_Status = STA_NOINIT;   // Media Status 
///////////////////////////////////////////////////////////////////////////

#separate
BYTE SD_cmd(BYTE sdcmd, DWORD address)
//////////////////////////////////////////////////////////////////////////
//      Send commands to the SD card via the SPI bus
//
//   Revision History
//      20/01/09   SDHC support added
//
// Entry
//      The SD must be selected (CS must be asserted) prior to calling
//      this function
//////////////////////////////////////////////////////////////////////////
   {
   BYTE i;
   BYTE response;
   BYTE *value;

   value = (BYTE *)&address;

   // dummy write to ensure SD/MMC in sync with SPI bus
   SD_spi_read(0xFF);
   SD_spi_read(sdcmd);
   SD_spi_read(value[3]);
   SD_spi_read(value[2]);
   SD_spi_read(value[1]);
   SD_spi_read(value[0]);
   if (sdcmd == SD_CMD_SEND_IF_COND)
      SD_spi_read(0x87);   // valid CRC for SD_CMD_SEND_IF_COND 0x1aa (CMD8)
   else
      SD_spi_read(0x95); // valid crc for put card in SPI command (0x40)

   // NCR - Maximum number of cycles between command and response is 64 clock cycles (8 bytes)
   i = 0;
   response = SD_spi_read(0xFF);
   while ((response == 0xFF) && (i < 100))
      {
      i++;
      response = SD_spi_read(0xFF);
      }


   // Error free response should be 0x00 to acknowledge the 
   // command or 0xFF if no response was detected from the card
   return(response);
   }


#separate
BYTE SD_set_BLOCKLEN( DWORD size)
//////////////////////////////////////////////////////////////////////////
//      Set the media block length
//
// Entry:
//      size in the range of 1 to 512
//////////////////////////////////////////////////////////////////////////
   {
   switch (SD_cmd(SD_CMD_SET_BLOCKLEN, size))
      {   
      case 0x00 : // done:
         block_size=size; //// assign global block size
         return(TRUE);

      case 0x40 : // invalid block size request
         // Parameter Rejected
         return(FALSE);

      default :
         // Unexpected response from SET_BLOCKLEN
         return(FALSE);
      }
   }


#define MaxReceiveDataTimeout 10000
BYTE SD_receive_data(BYTE *ptr, DWORD size) 
///////////////////////////////////////////////////////////////////////////
//   Read data block from Card to memory
//   Maximum block access time = 100ms
//
// Entry
//      Ensure Memory Card is deselected before calling
//      Read class command issued to card and acknowledged
//
//   Exit
//      Returns 0 on success,
//
///////////////////////////////////////////////////////////////////////////
   {
   WORD i;
   BYTE response;

    // poll for start token
   response = SD_spi_read(0xFF);
   for(i=0; (i < MaxReceiveDataTimeout) && (response == 0xFF); i++)
      {
      delay_us(10);
      response = SD_spi_read(0xFF); 
      }

   // start token?
   if (response != 0xFE)
      {
      if (!response)
         response = 0xFE;
      return(response);
      }

   // found start token, read the data
   for (i=0; i < size; i++) 
      ptr[i]=SD_spi_read(0xFF);

   // Discard the CRC
   SD_spi_read(0xFF);
   SD_spi_read(0xFF);

   // clear the card
   SD_spi_read(0xFF);
   return(0);
   }


// Wait for card ready 
BYTE wait_ready (void)
   {
   BYTE response;
   WORD Timer;

   Timer = 50000;         // Maximum wait for ready in timeout of 500ms 
   SD_spi_read(0xFF);
   do
      {
      delay_us(10);
      response = SD_spi_read(0xFF);
      Timer--;
      }
   while ((response != 0xFF) && Timer)
      ;

   return (response);
   }


#ifndef _READONLY
#separate
BOOLEAN SD_write_data(BYTE *ptr, BYTE token) 
///////////////////////////////////////////////////////////////////////////
//   Write a data block from memory to the Card
//   Maximum block access time = 100ms
//
// Entry
//      Ensure Memory Card is deselected before calling
//      Write class command issued to card and acknowledged
//
//   Exit
//      Returns TRUE on success,
///////////////////////////////////////////////////////////////////////////
   {
   BYTE response;
   WORD WriteCount = 512;

   if (wait_ready() != 0xFF) 
      return (FALSE);

   SD_spi_write(token);               // transmit data token
   if (token != 0xFD)
      {
      // here if a data token   
      while (WriteCount--)             // transmit the 512 byte data block tothe card
         SD_spi_write(*ptr++);

      SD_spi_write(0xFF);               // CRC (not used)
      SD_spi_write(0xFF);               // Dummy clocks to force card to process the command
      response = SD_spi_read(0xFF);      // Receive data response


      for( WriteCount=0; WriteCount < 50000; WriteCount++)
         {
         delay_us(10);
         response = SD_spi_read(0xFF);   // digest prior operation
         if (response != 0x00)
            break;
         }

      }
   return(TRUE);
   }



#separate
DRESULT disk_write (BYTE *Buffer, DWORD SectorNumber, BYTE SectorCount)
///////////////////////////////////////////////////////////////////////////
//   Writes SectorCount Sectors to the SD Card starting at the address
//   SectorNumber
//
//   Revision History
//      20/01/09   SDHC support added
//
// Entry
//  Buffer             Pointer to the write buffer
//  SectorNumber      Sector number to write to
//  SectorCount        Number of sectors to write (SHOULD BE 1)
///////////////////////////////////////////////////////////////////////////
   {
   if (Media_Status & STA_NOINIT) 
      return (RES_NOTRDY);

   if (Media_Status & STA_PROTECT)
      return (RES_WRPRT);

   if (!SectorCount)
      return (RES_PARERR);

   SelectSD;

   if (Card != SDHC)
      SectorNumber *= 512;

   if (SectorCount == 1)   // Single block write 
      {
      if (SD_cmd(SD_CMD_WRITE_BLOCK, SectorNumber) == 0)
         if (SD_write_data(Buffer, 0xFE))
            SectorCount = 0;
      }
   else 
      {   // Multiple block write 
      if (Card & (SDv1 || SDHC || SDSC)) 
         {
         SD_cmd(SD_CMD_APPL_CMD,0);
         SD_cmd(SD_ACMD23, SectorCount);
         }
      if (SD_cmd(SD_CMD_WRITE_MULTIBLOCK, SectorNumber) == 0) 
         {
         do 
            {
            if (!SD_write_data(Buffer, 0xFC)) 
               break;
            Buffer += 512;
            } while (--SectorCount);

         if (!SD_write_data(0, 0xFD))   // STOP_TRAN token 
            SectorCount = 1;
         }
      }

   DeselectSD;
   SD_spi_read(0xFF);
   return SectorCount ? RES_ERROR : RES_OK;
}
#endif





#separate
DRESULT disk_read (BYTE* Buffer, DWORD SectorNumber, BYTE SectorCount)
///////////////////////////////////////////////////////////////////////////
//   Reads SectorCount Sectors from the SD Card starting at the address
//   SectorNumber
//
//   Revision History
//      20/01/09   SDHC support added
//
// Entry
//  Buffer             Pointer to the read buffer
//  SectorNumber      Sector number to read from
//  SectorCount        Number of sectors to read (SHOULD BE 1)
///////////////////////////////////////////////////////////////////////////
   {
   if (Media_Status & STA_NOINIT) 
      return (RES_NOTRDY);

   // check sector count is valid
   if ((SectorCount > MaxSectorCount) || !SectorCount)
      return(RES_PARERR);

   SelectSD;

   // set the block size
   if (!SD_set_BLOCKLEN(512))
      {
      // Cannot set block length;
      DeselectSD;
      return(RES_ERROR);
      }

   if (Card != SDHC)
      SectorNumber *= 512;

   if (SectorCount == 1)
      {
      // read block command
      if(SD_cmd(SD_CMD_READ_BLOCK, SectorNumber))
         {
         // Read_Block Error: couldn't set the read address
         DeselectSD;
         return(RES_PARERR);
         }

      if (SD_receive_data(Buffer, (DWORD) 512))
         {
         DeselectSD;
         return(RES_ERROR);
         }
      }
   else
      {
      // here to transfer multiple blocks
      // send multi block read command
      if(SD_cmd(SD_CMD_READ_MULTIBLOCK, SectorNumber))
         {
         // Read_Block Error: couldn't set the read address
         DeselectSD;
         return(RES_PARERR);
         }

      do
         {      
         if (SD_receive_data(Buffer, (DWORD) 512))
            {
            DeselectSD;
            return(RES_ERROR);
            }

         // Update the buffer pointer
         Buffer += 512;
         } while (--SectorCount);
      SD_cmd(SD_CMD_STOP_TX,0);      // stop transmission
      }

   // finished sucessfully
   DeselectSD;
   return(RES_OK);
   }


#separate
DSTATUS disk_shutdown(void)
///////////////////////////////////////////////////////////////////////////
//   Deselects the drive and sets the Media_Status to STA_NOINIT
///////////////////////////////////////////////////////////////////////////
   {
   DeselectSD;
   Media_Status = STA_NOINIT;
   return(Media_Status);
   }


#separate
DSTATUS disk_status(void)
///////////////////////////////////////////////////////////////////////////
//   Tests and returns the status of the Media
///////////////////////////////////////////////////////////////////////////
   {
   // test for presence of the card
   #ifdef SD_CD
      if (bit_test(SD_CD))
         {
         // no card, go clean up
         Media_Status = disk_shutdown() | STA_NODISK;
         return(Media_Status);
         }
      else
   #endif
      // card found
      Media_Status &= ~STA_NODISK;

   // check write protect status
   #ifdef SD_WP
      if (bit_test(SD_WP))
         Media_Status |= STA_PROTECT;
      else
   #endif

      Media_Status &= ~STA_PROTECT;

   return(Media_Status);
   }


#separate
DSTATUS disk_initialize(void)
///////////////////////////////////////////////////////////////////////////
//   Initialize the SPI bus and Memory card internal logic
///////////////////////////////////////////////////////////////////////////
{
   BYTE i, response, ocr[4];
   WORD Timer;
   DSTATUS SDCardStatus = 0;

   // initialise the card type
   Card = None;

   DeselectSD;

   // test if the card is present
   #ifdef SD_CD
      if (nv_report_mode == 4) printf("@FS:Testing card\r\n");
      if (bit_test(SD_CD))
         return(STA_NODISK | STA_NOINIT);
   #endif

   delay_ms(10);

   // prepare to flush the SPI bus for the MMC and SD Memory Cards. 
   // sets the SPI bus speed low and initiates 80 SPI clock pulses
   // SPI Mode 0
   setup_spi(SPI_MASTER | SPI_L_TO_H | SPI_CLK_DIV_64 | SPI_XMIT_L_TO_H);
   clear_interrupt(INT_SSP);

   // flush the SPI bus
   for(i=0; i<10; i++)     // generate 80 clocks to get SD/MMC card ready
      SD_spi_read(0xFF);

   //printf("Testing write protect status of the card\r\n");
   // get the write protect status of the card
   // active low indicates the card is write protected
   #ifdef SD_WP
      if (bit_test(SD_WP))
         SDCardStatus = STA_PROTECT;
   #endif

   // put the card into the idle state
   SelectSD;
   if (nv_report_mode == 4) printf("@FS:Discovery\r\n");
   response = SD_cmd(SD_CMD_GO_IDLE_STATE,0);       // CMD0
   if (response > 1 )
   {
      if (nv_report_mode == 4) printf("@FS:Unable to put card in idle state, response = %02x\r\n", response);
      SDCardStatus |= STA_NOINIT;
      goto Exit_disk_initialize;
   }

   // Here the card is in idle state
   if (nv_report_mode == 4) printf("@FS:Card idle, response = %02x\r\n",response);   

   //    checking for SD ver 2 - sending CMD8
   response = SD_cmd(SD_CMD_SEND_IF_COND, 0x1AA);
   if (nv_report_mode == 4) printf("@FS:Response to SD ver 2 test CMD8 = %x\r\n",response);
   if (response == 1) 
   { 
      if (nv_report_mode == 4) printf("@FS:Possible SDv2 card detected\r\n");
      // Response to CMD8 - indicates possible SD ver 2 card
      for (i = 0; i < 4; i++) 
         ocr[i] = SD_spi_read(0xFF);      // Get trailing return value of R7 resp

      // check the voltage range of the card andcheck the pattern
      if (nv_report_mode == 4) printf("@FS:Voltage range check, ocr = %02x%02x%02x%02x\r\n", ocr[0],ocr[1],ocr[2],ocr[3]);
      if (ocr[2] == 0x01 && ocr[3] == 0xAA) 
      {
         if (nv_report_mode == 4) printf("@FS:Card Vdd range = 2.7-3.6 volts\r\n");
         // The card can work at vdd range of 2.7-3.6V
         if (nv_report_mode == 4) printf("@FS:Sending ACMD41 command to SD ver 2 card\r\n");
         // send Application Specific Command identifier (CMD55)
         // followed by ACMD41
         Timer = 0;
         response = SD_cmd(SD_CMD_APPL_CMD,0);
         response = SD_cmd(SD_ACMD41, 1UL << 30);
         while (response && (Timer < 50000))
         {
            delay_us(20);
            Timer++;
            response = SD_cmd(SD_CMD_APPL_CMD,0);
            response = SD_cmd(SD_ACMD41, 1UL << 30);
         } 

         if (nv_report_mode == 4) printf("@FS:Response from sending ACMD41 = %02x, Timer = %lu\r\n",response,Timer);
         if (!response)
         {
            // send CMD58 Get CCS
            response = (SD_cmd(SD_CMD_SEND_OCR, 0x40000000));
            if (nv_report_mode == 4) printf("@FS:Response to CMD58 = %x\r\n",response);
            if (response == 0)   // CMD58 
            {
               // Check CCS bit in the OCR
               for (i = 0; i < 4; i++) 
                  ocr[i] = SD_spi_read(0xFF);
               if (ocr[0] & 0x40)
                  Card = SDHC;
               else
                  Card = SDSC;

               if (nv_report_mode == 4)printf("@FS:Sent CMD 58, Timer value = %lu, ocr = %02x%02x%02x%02x\r\n", Timer, ocr[0],ocr[1],ocr[2],ocr[3]);
            }
         }
      }
      else
      {
         if (nv_report_mode == 4) printf("@FS:Card CANNOT work Vdd range of 2.7-3.6 volts\r\n");
      }
   } 
   
   // test to see if we now have a valid card
   // if not perform legacy SD and MMC card detection
   if (Card == None)
   {
      // Invalid response to Command 8, SD Version 1 or MMC 
      if (nv_report_mode == 4) printf("@FS:Not an SDSC or SDHC card, Testing for SD Ver1 or MMC Card\r\n");

      // reinitialise the card
      response = SD_cmd(SD_CMD_GO_IDLE_STATE,0); 
      if (nv_report_mode == 4) printf("@FS:Response from GO_IDLE = %02x\r\n",response);

      if ((response != 0x01) && (response != 0))
         {
         SDCardStatus |= STA_NOINIT;
         goto Exit_disk_initialize;
         }

      // Wait for the card to become ready
      // Maximum 500ms CMD1 (SD_CMD_SEND_OP_COND) to Ready
      Timer = 0;
      do
         {
         response = SD_cmd(SD_CMD_SEND_OP_COND,0);   
         if (response != 0x00 )
            delay_us(10);
         Timer++;
         }
      while ((!(response==0x00)) && (Timer < 50000)); 

      if (nv_report_mode == 4) printf("@FS:Response from SD_CMD_SEND_OP_COND = %02x, Timer = %lu\r\n",response, Timer);
      if(response)
         {
         SDCardStatus |= STA_NOINIT;
         goto Exit_disk_initialize;
         }

      // test for SD card
      // send an SD specific command
      response = SD_cmd(SD_CMD_APPL_CMD,0);
      if (nv_report_mode == 4) printf("@FS:Attempting SDv1 versus MMC ID, Response from SD_CMD_APPL_CMD = %02x\r\n",response);
      if (response)
         {
         // if any response then the card cannot support application commands
         // and therefore cannot be an SD card
         Card = MMC;
         }
      else
         {
         // send SD_SEND_OP_COND
         response = SD_cmd(SD_ACMD41,0);
         if (response <= 1 )
            Card = SDv1;
         else
            {
            if (nv_report_mode == 4) printf("@FS:Invalid response to SD_ACMD41, response = %02X, reinitializing as MMC\r\n",response);
            // Invalid response to SD Application command - trying MMC init sequence
            // reinitialise as MMC card
            response = SD_cmd(SD_CMD_GO_IDLE_STATE,0); 
            if ((response !=0x01) && (response != 0))
               {
               // Card reset failure - aborting SD card initialization;
               if (nv_report_mode == 4) printf("@FS:Failed to initialize as MMC, response = %02X, exiting\r\n",response);
               SDCardStatus |= STA_NOINIT;
               goto Exit_disk_initialize;
               }

            // Wait for the card to become ready
            // Maximum 500ms CMD1 (SD_CMD_SEND_OP_COND) to Ready
            Timer = 0;
            do
               {
               response = SD_cmd(SD_CMD_SEND_OP_COND,0);   
               if (response != 0x00 )
                  delay_us(10);
               Timer++;
               }
            while ((!(response==0x00)) && (Timer < 50000)); 
   
            if(response)
               {
               if (nv_report_mode == 4) printf("@FS:Card failed to respond correcly after SD_CMD_SEND_OP_COND, response = %02X, Timer = %lu\r\n",response, Timer);
               SDCardStatus |= STA_NOINIT;
               goto Exit_disk_initialize;
               }

            Card = MMC;
            }
         }

      if (nv_report_mode == 4) printf("@FS:Setting block length\r\n");
      // CMD16 Set R/W block length to 512
      response = SD_cmd(SD_CMD_SET_BLOCKLEN,512);
      if ((Timer == 50000) || (response > 1))   
      {
         Card = None;
         if (nv_report_mode == 4) printf("@FS:Error setting block length, response = %02x, timer = %lu\r\n",response, Timer);
      }
   }
   
   if (Card == None)
   {
      if (nv_report_mode == 4)
         printf("@FS:Card Type Discovery Error\r\n");

      SDCardStatus |= STA_NOINIT;
      goto Exit_disk_initialize;
   }


   // Completed card identification
   switch (Card)
   {
      case MMC :
         if (nv_report_mode == 4) printf("@FS:MMC Card found\r\n");
         break;

      case SDv1 :
         if (nv_report_mode == 4) printf("@FS:SDv1 Card found\r\n");
         break;

      case SDSC :
         if (nv_report_mode == 4) printf("@FS:SDSC Card found\r\n");
         break;

      case SDHC :
         if (nv_report_mode == 4) printf("@FS:SDHC Card found\r\n");
         break;

      default :
         if (nv_report_mode == 4) printf("@FS:Card Type Discovery Error, Card = %02X\r\n",Card);
         SDCardStatus |= STA_NOINIT;
         goto Exit_disk_initialize;
   }

   SDCardStatus &= ~STA_NOINIT;

   if (nv_report_mode == 4) printf("@FS:Card reset success - Cmd to Ready count = %lu\r\n", Timer);

   // set the SPI bus speed to high

   DeselectSD;

   // SPI Mode 0
   setup_spi(SPI_MASTER | SPI_L_TO_H | SPI_CLK_DIV_4 | SPI_XMIT_L_TO_H);
   clear_interrupt(INT_SSP);
   SelectSD;

   if (nv_report_mode == 4) printf("@FS:SPI bus speed set to high\r\n");

   // Wait for the card to become ready 
   Timer = 0;
   do
   {
      response = SD_cmd(SD_CMD_SEND_OP_COND,0);   
      if (response != 0x00 )
         delay_us(10);

      Timer++;
   } while ((!(response==0x00)) && (Timer < 50000));

   if (response != 0x00)
   {
      if (nv_report_mode == 4) printf("@FS:Card activate failure, response = %02X, Timer = %lu\r\n", response, Timer);
      SDCardStatus |= STA_NOINIT;
      goto Exit_disk_initialize;
   }

   if (nv_report_mode == 4)
      printf("@FS:Card activate success on attempt %lu\r\n",Timer);

   msg_card_ok();


Exit_disk_initialize:
   DeselectSD;
   return(SDCardStatus);
}

#separate
DRESULT disk_ioctl (BYTE ctrl, void *buff)
///////////////////////////////////////////////////////////////////////////
//   Low Level SD function
//
//   Revision History
//      20/01/09   SDHC support added
//
//   Entry
//      ctrl      control code
//      buff      pointer to send / receive block
///////////////////////////////////////////////////////////////////////////
   {
   DRESULT Response;
   BYTE n, csd[16], *ptr;
   WORD csize;

   ptr = buff;
   if (Media_Status & STA_NOINIT) 
      return (RES_NOTRDY);

   SelectSD;

   Response = RES_ERROR;
   switch (ctrl)
   {
      case CTRL_SYNC :      // Flush dirty buffer if present
         if (wait_ready() == 0xFF)
            Response = RES_OK;
         break;


      case GET_SECTOR_COUNT :   // Get number of sectors on the disk (unsigned long) 
         if (SD_cmd(SD_CMD_SEND_CSD,0x00000000) == 0)
            if (SD_receive_data(ptr, (DWORD) 16) == 0)
               {
               // Calculate disk size 
               if ((csd[0] >> 6) == 1) 
                  { 
                  // SDC ver 2.00
                  csize = csd[9] + ((WORD)csd[8] << 8) + 1;
                  *(DWORD*)buff = (DWORD)csize << 10;
                  } 
               else 
                  { 
                  // MMC or SDC ver 1.XX
                  n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                  csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
                  *(DWORD*)buff = (DWORD)csize << (n - 9);
                  }
               Response = RES_OK;
               }
         break;


      case GET_SECTOR_SIZE :   // Get sectors on the disk (WORD)
            *(WORD*)buff = 512;
            Response = RES_OK;
         break;


      case GET_BLOCK_SIZE :   // Get erase block size in unit of sectors (DWORD)
         if (Card & (SDv1 || SDSC || SDHC)) 
            { 
            // SDC ver 2.00
            SD_cmd(SD_CMD_APPL_CMD,0);
            if (SD_cmd(SD_ACMD13, 0) == 0) 
               { 
               // read SD status
               SD_spi_read(0xFF);
               if (SD_receive_data(csd, (DWORD) 16)) 
                  {
                  // Read partial block
                  for (n = 64 - 16; n; n--) 
                     SD_spi_read(0xFF);      // Purge trailing data
                  *(DWORD*)buff = 16UL << (csd[10] >> 4);
                  Response = RES_OK;
                  }
               }
            } 
         else 
            { 
            // SDC ver 1.XX or MMC
            if ((SD_cmd(SD_CMD_SEND_CSD, 0) == 0) && SD_receive_data(csd, (DWORD) 16)) 
               {
               // read CSD
               if (Card & SDv1) 
                  {
                  // SDC ver 1.XX
                  *(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
                  } 
               else 
                  {            
                  // MMC 
                  *(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
                  }
               Response = RES_OK;
               }
            }
         break;


      case SD_GET_TYPE :      // Get card type (1 byte)
         *ptr = Card;
         Response = RES_OK;
         break;


      case SD_GET_CSD :   // Receive CSD as a data block (16 bytes) 
         if (SD_cmd(SD_CMD_SEND_CSD,0x00000000) == 0)
            if (SD_receive_data(ptr, (DWORD) 16) == 0)
               Response = RES_OK;
         break;


      case SD_GET_CID :   // Receive CID as a data block (16 bytes) 
         if (SD_cmd(SD_CMD_SEND_CID,0x00000000) == 0)
            if (SD_receive_data(ptr, (DWORD) 16) == 0)
               Response = RES_OK;
         break;


      case SD_GET_OCR :   // Receive OCR as an R3 resp (4 bytes) 
         if (SD_cmd(SD_CMD_SEND_OCR, 0) == 0) 
            {   // READ_OCR 
            for (n = 0; n < 4; n++)
               *ptr++ = SD_spi_read(0xFF);
            Response = RES_OK;
            }
         break;


      case SD_GET_SDSTAT :   // Receive SD statsu as a data block (64 bytes)
         SD_cmd(SD_CMD_APPL_CMD,0);
         if (SD_cmd(SD_ACMD13, 0) == 0) 
            {   
            // SD status
            SD_spi_read(0xFF);
            if (SD_receive_data(buff, (DWORD) 64))
            Response = RES_OK;
            }
         break;


      default:
         Response = RES_PARERR;
   }
   DeselectSD;   
   SD_spi_read(0xFF);         // Idle (Release DO) 
   return (Response);
}
#define _DISKIF
#endif
