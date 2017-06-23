//   Main File for the SD/MMC File System Demonstration

/*
   Modifications:

   dir_list() - removed
   SD_Explore() - removed
   etc.
*/

///////////////////////////////////////////////////////////////////////////
// Specify the file system mode
///////////////////////////////////////////////////////////////////////////
#define USE_FAT_LITE // configure the filesystem for FAT Lite operation

#ifndef DWORD
   #define DWORD int32
#endif

#ifndef WORD
   #define WORD int16
#endif

// define Software RTC data structure

struct _rtc {
   BYTE   ms100;      // number of 100ms ticks 0..9
   BYTE   sec;        // 0..59
   BYTE   min;        // 0..59
   BYTE   hour;       // 0..23
   BYTE   day;        // 1..31
   BYTE   month;      // 1..12
   BYTE   year;       // 0..99
   WORD   DOY;        // 1..366
   } rtc;

   // define constants to derive the 1ms system clock. This clock will
   // eventually need to be synchronized to the hardware RTC when implemented
#define C_TMR0_1ms   256 - (XTAL_FREQ/(64*4*1000))
#define CTMR_Activity 2
///////////////////////////////////////////////////////////////////////////
// include the FAT file system
#include <ff_min.h>               // include the SD Card file system
///////////////////////////////////////////////////////////////////////////

FRESULT   FS_Status;

// prototypes
char * strupr(char * source);
// @@@@ DEBUGGING PROTOTYPES @@@@
BYTE append_data (char *target);

/* strlen modified to test strings > 256 chars long
   standard template: size_t strlen(const char *s).
   Computes length of s1 (preceding terminating 0) 
*/
unsigned int16 strlen16(char *s)
{
   char *sc;

   for (sc = s; *sc != 0; sc++);
   return(sc - s);
}

void init_rtc (void)
///////////////////////////////////////////////////////////////////////////
// Initialise the Software RTC
///////////////////////////////////////////////////////////////////////////
   {
   rtc.DOY=1;
   rtc.year=10;
   rtc.month=1;
   rtc.day=1;
   rtc.hour=0;
   rtc.min=0;
   rtc.sec=0;
   rtc.ms100=0;
   }

DWORD get_fattime(void)
///////////////////////////////////////////////////////////////////////////
//   Platform specific user function to return a FAT time partameter
//   USed by the FAT file system
//
//   31-25: Year(0-127 +1980), 24-21: Month(1-12), 20-16: Day(1-31)
//   15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2)
///////////////////////////////////////////////////////////////////////////
   {
   DWORD work;

   work =  ((DWORD)rtc.year + 20) << 25;
   work |= ((DWORD)rtc.month & 0x0f) << 21;
   work |= ((DWORD)rtc.day & 0x1f) << 16;
   work |= ((DWORD)rtc.hour & 0x1f) << 11;
   work |= ((DWORD)rtc.min & 0x3f) << 5;
   work |= ((DWORD)rtc.sec & 0x3f) >> 1;
   return (work);
   }   

void file_list(char *ptr)
///////////////////////////////////////////////////////////////////////////
//   Lists the contents of a text file
///////////////////////////////////////////////////////////////////////////
{
   FIL fsrc;
   FRESULT result;      // FatFs function common result code
   char mesg[32];

   result = f_open(&fsrc, ptr, FA_OPEN_EXISTING | FA_READ);

    // display the contents of the file
   if (result == FR_OK)
   {
      WORD i, br;

      // Display the file's FIL data structure
      // f_show_FIL_structure(&fsrc);

      do
      {
         result = f_read(&fsrc, mesg, sizeof(mesg), &br);
         for (i = 0; i < br; i++)
         {
            putc(mesg[i]);
         }
      } while ((result == FR_OK) && br);

      // Display the file's FIL data structure
      // f_show_FIL_structure(&fsrc);
      if (result != FR_OK)
      {
         printf("TYPE command ERROR\r\n");
         f_get_error_mesg(result,mesg);
         printf("@FS: %s\r\n",mesg);
      }

      // Close all files
      f_close(&fsrc);
      printf("\r\n");      
   }
   else
   {
      f_get_error_mesg(result,mesg);
      printf("@FS: %s\r\n",mesg);
   }
}

void display_file_result(FRESULT status)
///////////////////////////////////////////////////////////////////////////
//   Display the result of file operations 
///////////////////////////////////////////////////////////////////////////
   {
   printf("** @FS: ERROR\r\n");
   if (status & FR_OK)
      printf("   NoError\r\n");
   if (status & FR_NOT_READY)
      printf("   Media Not Ready\r\n");
   if (status & FR_NO_FILE)
      printf("   File Not Found\r\n");
   if (status & FR_NO_PATH)
      printf("   Invalid Path\r\n");
   if (status & FR_INVALID_NAME)
      printf("   Invalid Name\r\n");
   if (status & FR_DENIED)
      printf("   Access Denied\r\n");
   if (status & FR_DISK_FULL)
      printf("   Disk Full\r\n");
   if (status & FR_RW_ERROR)
      printf("   Read/Write Error\r\n");
   if (status & FR_INCORRECT_DISK_CHANGE)
      printf("   Incorrect Media Change\r\n");
   if (status & FR_WRITE_PROTECTED)
      printf("   Write Protected\r\n");
   if (status & FR_NOT_ENABLED)
      printf("   Not Enabled\r\n");
   if (status & FR_NO_FILESYSTEM)
      printf("   No File System\r\n");
   }

BYTE append_data (char *target)
///////////////////////////////////////////////////////////////////////////
//   Opens the file to be appended and writes to it. If the file exists it 
//   is opened. If it does not exist it is created. The file this
//   then closed.
///////////////////////////////////////////////////////////////////////////
{
   FIL fdst;           // file structures
   FRESULT result;     // FatFs function common result code
   WORD btw, bw;       // File R/W count
   //char target[16];
   char mesg[BUFFER_LEN];

   memset (&mesg[0], 0x00, BUFFER_LEN);  //blank it

   // Create destination file
   //strcpy(target,"data_all.txt");
   result = f_open(&fdst, target, FA_OPEN_ALWAYS | FA_WRITE);
   if (result)
   {
      printf("append: FS ERROR on file_open\r\n");
   }
   else
   {
      // if the file already exists then append to the end of the file
      if (fdst.fsize != 0)
         result = f_lseek(&fdst, fdst.fsize); 

      // write a short string to destination file
      if (buffer_select == 0) strncpy(mesg, data_buffer, BUFFER_LEN - 1);
      if (buffer_select == 1) strncpy(mesg, event_buffer, BUFFER_LEN - 1);  
      if (nv_report_mode > 4) printf("@FS:Writing\r\n[%s]\r\n", mesg);

      btw = strlen16(mesg);
      result = f_write(&fdst, mesg, btw, &bw);
      if (result)
      {
         printf("@FS:ERROR[append]\r\n");
      }
      else
      {
         if (nv_report_mode > 4) printf("@FS:Closing\r\n");
      }

      f_close(&fdst);
   }

   return (result);
}
   

