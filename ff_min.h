//   ff.h include file FatFs - FAT file system module include file

/*
   Modifications
   
   f_chmod() - removed
   f_rename() - removed

*/


#ifndef _FAT_FS
#include <string.h>
#include "media_io.h"

// Result type for fatfs application interface 
typedef unsigned char   FRESULT;
typedef char* pchar;
typedef BYTE* pBYTE;

// File system object structure 
typedef struct _FATFS 
   {
   BYTE   fs_type;      // FAT type 
   BYTE   files;         // Number of files currently opened 
   BYTE   sects_clust;   // Sectors per cluster 
   BYTE   n_fats;         // Number of FAT copies 
   WORD   n_rootdir;      // Number of root directory entry 
   BYTE   winflag;      // win[] dirty flag (1:must be written back) 
   BYTE   pad1;
   DWORD   sects_fat;      // Sectors per fat 
   DWORD   max_clust;      // Maximum cluster# + 1 
   DWORD   fatbase;      // FAT start sector 
   DWORD   dirbase;      // Root directory start sector (cluster# for FAT32) 
   DWORD   database;      // Data start sector 
   DWORD   last_clust;      // last allocated cluster
   DWORD   winsect;      // Current sector appearing in the win[] 
   BYTE   win[512];      // Disk access window for Directory/FAT area 
   } FATFS;


// Directory object structure 
typedef struct _DIR 
   {
   DWORD   sclust;      // Start cluster 
   DWORD   clust;      // Current cluster 
   DWORD   sect;      // Current sector 
   WORD   index;      // Current index 
   } DIR;


// File object structure 
typedef struct _FIL 
   {
   DWORD   fptr;         // File R/W pointer 
   DWORD   fsize;         // File size 
   DWORD   org_clust;      // File start cluster 
   DWORD   curr_clust;      // Current cluster 
   DWORD   curr_sect;      // Current sector 

   #ifndef _FS_READONLY
      DWORD   dir_sect;   // Sector containing the directory entry 
      BYTE*   dir_ptr;   // Pointer to the directory entry in the window 
   #endif

   BYTE   flag;         // File status flags 
   BYTE   sect_clust;      // Left sectors in cluster 

   #ifndef USE_FAT_LITE
      BYTE   buffer[512];   // File R/W buffer 
   #endif
   } FIL;


// File status structure 
typedef struct _FILINFO 
   {
   DWORD fsize;         // Size 
   WORD fdate;            // Date 
   WORD ftime;            // Time 
   BYTE fattrib;         // Attribute 
   char fname[8+1+3+1];   // Name (8.3 format) 
   } FILINFO;



///////////////////////////////////////////////////////////////////////////
// FatFs module application interface                  

//void f_init(void);                                // Initialise the file system data structure 
#define f_init()    memset(fs, 0, sizeof(FATFS))         // Initialise the file system data structure
FRESULT f_open (FIL *fp, char *path, BYTE mode);         // Open or create a file 
FRESULT f_read (FIL *fp, void *buff, WORD btr, WORD *br);   // Read from a file 
FRESULT f_close (FIL *fp);                           // Close a file 
FRESULT f_lseek (FIL *fp, DWORD ofs);                  // Seek file pointer 
FRESULT f_opendir (DIR *scan, char *path);               // Initialize to read a directory 
FRESULT f_readdir (DIR *scan, FILINFO *finfo);            // Read a directory item 
FRESULT f_stat (char *path, FILINFO *finfo);            // Get file status 
FRESULT f_getfree (DWORD *nclust);                     // Get number of free clusters 
FRESULT f_mountdrv (void);                           // Force initialized the file system 
void f_get_error_mesg(FRESULT Mesg, char *destination);      // Convert the FRESULT return code to a string

#ifndef _FS_READONLY
   FRESULT f_write (FIL *fp, void *buff, WORD btw, WORD *bw);   // Write file 
   FRESULT f_sync (FIL *fp);                           // Flush cached information of a writing file    
   FRESULT f_unlink (char *path);                        // Delete a file or directory 
   FRESULT f_mkdir (char *path);                        // Create a directory 
   FRESULT f_chmod (char *path, BYTE value, BYTE mask);      // Change file attriburte 
   FRESULT f_rename ( char *path_old,   char *path_new);      // Rename a file / directory
#endif


// User defined function to give a current time to fatfs module 

#ifndef _FS_READONLY
DWORD get_fattime(void);   // 31-25: Year(0-127 +1980), 24-21: Month(1-12), 20-16: Day(1-31) 
                     // 15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) 
#endif


   // File function return code 
#define   FR_OK                  0
#define   FR_NOT_READY            1
#define   FR_NO_FILE               2
#define   FR_NO_PATH               3
#define   FR_INVALID_NAME            4
#define   FR_DENIED               5
#define   FR_DISK_FULL            6
#define   FR_RW_ERROR               7
#define   FR_INCORRECT_DISK_CHANGE   9
#define   FR_WRITE_PROTECTED         10
#define   FR_NOT_ENABLED            11
#define   FR_NO_FILESYSTEM         12


   // File access control and file status flags 
#define   FA_READ            0x01
#define   FA_OPEN_EXISTING   0x00
#ifndef _FS_READONLY
#define   FA_WRITE         0x02
#define   FA_CREATE_ALWAYS   0x08
#define   FA_OPEN_ALWAYS      0x10
#define FA__WRITTEN         0x20
#define FA__DIRTY         0x40
#endif
#define FA__ERROR         0x80


   // FAT type signature (fs_type) 
#define FS_FAT12   1
#define FS_FAT16   2
#define FS_FAT32   3


   // File attribute mask for directory entry 
#define   AM_RDO      0x01   // Read Only 
#define   AM_HID      0x02   // Hidden 
#define   AM_SYS      0x04   // System 
#define   AM_VOL      0x08   // Volume Label 
#define AM_DIR      0x10   // Directory 
#define AM_ARC      0x20   // Archive 



   // Multi-byte word access macros 
#ifdef _BYTE_ACC
#define   LD_WORD(ptr)      (((WORD)*(BYTE*)(ptr+1)<<8)|*(ptr))
#define   LD_DWORD(ptr)      (((DWORD)*(BYTE*)(ptr+3)<<24)|((DWORD)*(BYTE*)(ptr+2)<<16)|((WORD)*(BYTE*)(ptr+1)<<8)|*(BYTE*)(ptr))
#define   ST_WORD(ptr,val)   *(BYTE*)(ptr)=val; *(BYTE*)(ptr+1)=val>>8
#define   ST_DWORD(ptr,val)   *(BYTE*)(ptr)=val; *(BYTE*)(ptr+1)=val>>8; *(BYTE*)(ptr+2)=val>>16; *(BYTE*)(ptr+3)=val>>24
#else
#define   LD_WORD(ptr)      (*(WORD*)(BYTE*)(ptr))
#define   LD_DWORD(ptr)      (*(DWORD*)(BYTE*)(ptr))
#define   ST_WORD(ptr,val)   *(WORD*)(BYTE*)(ptr)=(val)
#define   ST_DWORD(ptr,val)   *(DWORD*)(BYTE*)(ptr)=(val)
#endif


//#include <string.h>

    // Give a work area for FAT File System (activate module)
FATFS   FileSystem;     // Fat File System work area
FATFS   *fs = &FileSystem;



///////////////////////////////////////////////////////////////////////////
//                    Module Private Functions
///////////////////////////////////////////////////////////////////////////

      
BOOLEAN move_window (DWORD sector)
///////////////////////////////////////////////////////////////////////////
//   Loads the specified sector into the FATFS window. If the existing sector
//   in the window is "dirty" the existing sector is written to the media
//   before the new sector is loaded to the window. If the target sector is
//   the same as the current sector in the window no chage is made. If the
//   target sector number == 0, then the existing sector, if dirty, is written
//   to the media
//
//   Entry:
//      Sector         Sector number to make apperance in the FATFS->win
//                  Move to zero writes back (flushes) the dirty window
//      FATFS->winset   Sector number of current sector in the window
//      FATFS->winflag   Dirty flag. If winflag == 1 the existing sector
//                  is written the the media
//
//   Exit:
//      Returns true on success, false otherwise
//
///////////////////////////////////////////////////////////////////////////
   {
   DWORD wsect;
   FRESULT response;
//   char mesg[32];

   wsect = fs->winsect;

   // test if the current window if the target window
   if (wsect != sector) 
      {
      // Need to load (move) the new sector into the window

      #ifndef _FS_READONLY
         BYTE n;
         // Determine if the existing sector in the window is "dirty"
         // If dirty the old entry must be written to the disk
         if (fs->winflag) 
            {
            // The existing sector must be written back to the disk
            response = disk_write(fs->win, wsect, 1);
            if (response != RES_OK)
               {
//               f_get_error_mesg(response, mesg);
//               printf("move_window FS ERROR - %s\r\n", mesg);
               return (FALSE);
               }

            // clear the dirty flag and update the file system data structure
            // for the new sector to be loaded
            fs->winflag = 0;
            if (wsect < (fs->fatbase + fs->sects_fat))
               {
               // The target sector is inside the FAT area
               // Refresh other FAT copies
               for (n = fs->n_fats; n >= 2; n--) 
                  {
                  // Reflect the change to all FAT copies
                  wsect += fs->sects_fat;
                  if (disk_write(fs->win, wsect, 1) != RES_OK)
                     break;
                  }
               }
            }
      #endif
      // new sector to load?
      if (sector) 
         {
         // load the target sector and update the winset pointer to 
         // identify this sector as the one in the window
         if (disk_read(fs->win, sector, 1) != RES_OK) 
            return FALSE;
         fs->winsect = sector;
         }
      }
   return (TRUE);
   }



DWORD get_cluster (DWORD clust)
///////////////////////////////////////////////////////////////////////////
//   Get the next cluster link information from the directory for the
//   target cluster
//
//   Entry:
//      FATFS      Must have been initialized
//      clust      Target cluster number
//
//   Exit
//      Return      Next cluster in chain or 1 on error      
//
///////////////////////////////////////////////////////////////////////////
   {
   WORD wc, bc;
   DWORD fatsect;

   // check if the cluster number is valid for the file system
   if ((clust >= 2) && (clust < fs->max_clust))
      {
      // Here we have a valid data cluster number
      fatsect = fs->fatbase;
      switch (fs->fs_type) 
         {
         case FS_FAT12 :
            bc = (WORD)clust * 3 / 2;
            if (!move_window(fatsect + bc / 512)) 
               break;
            wc = fs->win[bc % 512];
            bc++;
            if (!move_window(fatsect + bc / 512)) 
               break;
            wc |= (WORD)fs->win[bc % 512] << 8;
            return ((clust & 1) ? (wc >> 4) : (wc & 0xFFF));

         case FS_FAT16 :
            if (!move_window(fatsect + clust / 256))
               break;
            return (LD_WORD(&(fs->win[((WORD)clust * 2) % 512])));

         case FS_FAT32 :
            if (!move_window(fatsect + clust / 128))
               break;
            return (LD_DWORD(&(fs->win[((WORD)clust * 4) % 512])) &0x0FFFFFFF);
         }
      }
   return (1);   // Return with 1 means function failed 
   }


#ifndef _FS_READONLY
BOOLEAN put_cluster (DWORD clust, DWORD val)
///////////////////////////////////////////////////////////////////////////
//   Change the status of a Cluster
//
//   Entry:
//      clust      Target cluster number
//      val         New value to mark the cluster
///////////////////////////////////////////////////////////////////////////
   {
   WORD bc;
   BYTE *p;
   DWORD fatsect;

   fatsect = fs->fatbase;
   switch (fs->fs_type) 
      {
      case FS_FAT12 :
         bc = (WORD)clust * 3 / 2;
         if (!move_window(fatsect + bc / 512))
            return (FALSE);
         p = &fs->win[bc % 512];
         *p = (clust & 1) ? ((*p & 0x0F) | ((BYTE)val << 4)) : (BYTE)val;
         fs->winflag = 1;
         bc++;
         if (!move_window(fatsect + bc / 512))
            return (FALSE);
         p = &fs->win[bc % 512];
         *p = (clust & 1) ? (BYTE)(val >> 4) : ((*p & 0xF0) | ((BYTE)(val >> 8) & 0x0F));
         break;

      case FS_FAT16 :
         if (!move_window(fatsect + clust / 256)) 
            return (FALSE);
         ST_WORD(&(fs->win[((WORD)clust * 2) % 512]), (WORD)val);
         break;

      case FS_FAT32 :
         if (!move_window(fatsect + clust / 128)) 
            return (FALSE);
         ST_DWORD(&(fs->win[((WORD)clust * 4) % 512]), val);
         break;

      default :
         return (FALSE);
      }

   // flag the FATFS window as dirty
   fs->winflag = 1;
   return (TRUE);
   }
#endif // _FS_READONLY 



#ifndef _FS_READONLY
BOOLEAN remove_chain (DWORD clust)
///////////////////////////////////////////////////////////////////////////
//   Remove a cluster from the cluster chain
//
//   Entry:
//      clust      Target cluster number
///////////////////////////////////////////////////////////////////////////
   {
   DWORD nxt;

   for (nxt = get_cluster(clust); nxt >= 2; nxt = get_cluster(clust))
      {
      if (!put_cluster(clust, 0))
         return FALSE;
      clust = nxt;
      }
   return TRUE;
   }
#endif



#ifndef _FS_READONLY
DWORD create_chain (DWORD clust)
///////////////////////////////////////////////////////////////////////////
//   Create or lengthen a cluster chain
//
//   Entry:
//      clust      Target cluster number to stretch. 0 creates a new chain
///////////////////////////////////////////////////////////////////////////
   {
   DWORD cstat, ncl, scl, mcl;

   mcl = fs->max_clust;
   if (clust == 0) 
      {                        // Create new chain
      scl = fs->last_clust;         // Get last allocated cluster
      if (scl < 2 || scl >= mcl) scl = 1;
      }
   else 
      {      // Stretch existing chain
      cstat = get_cluster(clust);      // Check the cluster status
      if (cstat < 2) return 0;      // It is an invalid cluster
      if (cstat < mcl) return cstat;   // It is already followed by next cluster
      scl = clust;
      }
      ncl = scl;                  // Scan start cluster
   do 
      {
      ncl++;                     // Next cluster
      if (ncl >= mcl) 
         {         // Wrap around
         ncl = 2;
         if (scl == 1) return 0;      // No free custer was found
         }
      if (ncl == scl) return 0;      // No free custer was found
      cstat = get_cluster(ncl);      // Get the cluster status
      if (cstat == 1) return 0;      // Any error occured
      } while (cstat);            // Repeat until find a free cluster

   if (!put_cluster(ncl, 0x0FFFFFFF)) return 0;      // Mark the new cluster "in use"
   if (clust && !put_cluster(clust, ncl)) return 0;   // Link it to previous one if needed
   fs->last_clust = ncl;

   return ncl;      // Return new cluster number
}
#endif // _FS_READONLY 



DWORD clust2sect (DWORD clust)
///////////////////////////////////////////////////////////////////////////
//   Determines the sector number from the cluster number
//
//   Entry:
//      clust      Target cluster number 
///////////////////////////////////////////////////////////////////////////
   {
   clust -= 2;      // First cluster in data area is cluster 2
   if (clust >= fs->max_clust) 
      return 0;      // Invalid cluster# 
   return (clust * fs->sects_clust + fs->database);
   }



BYTE check_fs (DWORD sect)
///////////////////////////////////////////////////////////////////////////
//   Determine the FAT type
//
//   Entry:
//      clust      Target cluster number to determine if a FAT boot record
///////////////////////////////////////////////////////////////////////////
   {
   //static const char fatsign[] = "FAT12FAT16FAT32";
   static char fatsign[] = "FAT12FAT16FAT32";

   // clear the file system buffer
   memset(fs->win, 0, 512);
   if (disk_read(fs->win, sect, 1) == RES_OK) 
      {   // Load boot record 
      if (LD_WORD(&(fs->win[510])) == 0xAA55) 
         {      // Is it valid? 
         if (!memcmp(&(fs->win[0x36]), &fatsign[0], 5))
            return (FS_FAT12);
         if (!memcmp(&(fs->win[0x36]), &fatsign[5], 5))
            return (FS_FAT16);
         if (!memcmp(&(fs->win[0x52]), &fatsign[10], 5) && (fs->win[0x28] == 0))
            return (FS_FAT32);
         }
      }
   return 0;
   }



BOOLEAN next_dir_ptr (DIR *scan)
///////////////////////////////////////////////////////////////////////////
//   Move Directory Pointer to Next
//
//   Entry:
//      scan      Pointer to a directory object
///////////////////////////////////////////////////////////////////////////
   {
   DWORD clust;
   WORD idx;

   idx = scan->index + 1;
   if ((idx & 15) == 0) 
      {      // Table sector changed? 
      scan->sect++;         // Next sector 
      if (!scan->clust) 
         {      // In static table 
         if (idx >= fs->n_rootdir) 
            return (FALSE);   // Reached to end of table 
         } 
      else 
         {   // In dynamic table 
         if (((idx / 16) & (fs->sects_clust - 1)) == 0) 
            {   // Cluster changed? 

            // Get next cluster 
            clust = get_cluster(scan->clust);         

            // Reached the end of table? 
            if ((clust >= fs->max_clust) || (clust < 2))
               return (FALSE);

            // Initialize for new cluster 
            scan->clust = clust;
            scan->sect = clust2sect(clust);
            }
         }
      }
   scan->index = idx;   // Lower 4 bit of scan->index indicates offset in scan->sect 
   return (TRUE);
   }



#ifndef _FS_MINIMUM
void get_fileinfo (FILINFO *finfo, BYTE *dir_ptr)
///////////////////////////////////////////////////////////////////////////
//   Fetches the file status from the Directory Entry
//
//   Entry:
//      finfo      Pointer to Store the File Information
//      dir         Pointer to the Directory Entry 
///////////////////////////////////////////////////////////////////////////
   {
   BYTE n, c, a;
   char *p;

   p = &(finfo->fname[0]);
   a = *(dir_ptr+12);            // NT flag 
   for (n = 0; n < 8; n++) 
      {   // Convert file name (body) 
      c = *(dir_ptr+n);
      if (c == ' ') 
         break;

      if (c == 0x05)
         c = 0xE5;

      if ((a & 0x08) && (c >= 'A') && (c <= 'Z'))
         c += 0x20;

      *p++ = c;
      }
   if (*(dir_ptr+8) != ' ') 
      {   // Convert file name (extension) 
      *p++ = '.';
      for (n = 8; n < 11; n++) 
         {
         c = *(dir_ptr+n);
         if (c == ' ') 
            break;
         if ((a & 0x10) && (c >= 'A') && (c <= 'Z'))
            c += 0x20;
         *p++ = c;
         }
      }
   *p = '\0';

   finfo->fattrib = *(dir_ptr+11);         // Attribute 
   finfo->fsize = LD_DWORD(dir_ptr+28);   // Size 
   finfo->fdate = LD_WORD(dir_ptr+24);      // Date 
   finfo->ftime = LD_WORD(dir_ptr+22);      // Time 
   }
#endif // _FS_MINIMUM 


char make_dirfile (pchar *path, char *dirname)
///////////////////////////////////////////////////////////////////////////
//   Extract and construct a standard format directory name
//
//   Entry:
//      path      Pointer to the file path pointer
//      dirname      Pointer to the Directory name buffer {Name(8), Ext(3), NT flag(1)}
///////////////////////////////////////////////////////////////////////////
   {
   BYTE n, t, c, a, b;
   char *ptr;

   // initialise the directory name buffer (work area)
   memset(dirname, ' ', 8+3);      // Fill buffer with spaces
   a = 0; b = 0x18;            // NT flag
   n = 0; t = 8;
   for (;;) 
      {
//      c = *(*path)++;
      ptr = *path;
      c = *ptr;
      (*path)++;

      if (c <= ' ') 
         c = 0;

      if ((c == 0) || (c == '/') || (c == '\\'))
         {   // Reached to end of str or directory separator 
         if (n == 0) 
            break;
         dirname[11] = a & b; 
            return (c);
         }
      if (c <= ' ' || c == 0x7F)
         break;   // reject invisible characters
      if (c == '.') 
         {
         if(!(a & 1) && (n >= 1) && (n <= 8))
            {   // Enter extension part 
            n = 8; t = 11; 
            continue;
            }
         break;
         }

#ifdef _USE_SJIS
      if (((c >= 0x81) && (c <= 0x9F)) ||      // Accept S-JIS code 
          ((c >= 0xE0) && (c <= 0xFC))) 
         {
         if ((n == 0) && (c == 0xE5))      // Change heading \xE5 to \x05 
            c = 0x05;
         a ^= 0x01; 
         goto md_l2;
         }
#endif
      if (c == '"')
         break;               // Reject " 

      if (c <= ')') 
         goto md_l1;            // Accept ! # $ % & ' ( ) 

      if (c <= ',')
         break;               // Reject * + , 

      if (c <= '9') 
         goto md_l1;            // Accept - 0-9 

      if (c <= '?') 
         break;               // Reject : ; < = > ? 

      if (!(a & 1)) 
         {   // These checks are not applied to S-JIS 2nd byte 
         if (c == '|') 
            break;            // Reject | 

         if ((c >= '[') && (c <= ']'))
            break;// Reject [ \ ] 

         if ((c >= 'A') && (c <= 'Z'))
            (t == 8) ? (b &= 0xF7) : (b &= 0xEF);

         if ((c >= 'a') && (c <= 'z')) 
            {      // Convert to upper case 
            c -= 0x20;
            (t == 8) ? (a |= 0x08) : (a |= 0x10);
            }
         }
   md_l1:
      a &= 0xFE;
   md_l2:
      if (n >= t) 
         break;
      dirname[n++] = c;
      }
   return 1;
   }



FRESULT trace_path (DIR *scan, char *fn, char *path, pBYTE *win_dir)
///////////////////////////////////////////////////////////////////////////
//   Trace the file path for the specified object
//
//   Entry:
//      *scan,         Pointer to directory object to return last directory
//      *fn,         Pointer to last segment name to return
//      *path,         Full-path string to trace a file or directory
//      *win_dir      Directory pointer in Win[] to return
///////////////////////////////////////////////////////////////////////////
   {
   DWORD clust;
   char ds;
   BYTE *dptr;

   dptr = NULL;

   // Initialize directory object
   // Point to the start of the directory area on the media
   clust = fs->dirbase;
   if (fs->fs_type == FS_FAT32) 
      {
      scan->clust = scan->sclust = clust;
      scan->sect = clust2sect(clust);
      }
   else 
      {
      scan->clust = scan->sclust = 0;
      scan->sect = clust;
      }
   scan->index = 0;

   // Skip leading spaces
   while ((*path == ' ') || (*path == '/') || (*path == '\\'))
       path++;

   if ((BYTE)*path < ' ') 
      {   // Null path means the root directory 
      *win_dir = NULL; 
      return (FR_OK);
      }

   for (;;) 
      {
      // extract the next short file name from the path parameter
      ds = make_dirfile(&path, fn);
      if (ds == 1) 
         return (FR_INVALID_NAME);

      // load the FATFS window with the first sector of the current object
      for (;;) 
         {
         if (!move_window(scan->sect)) 
            return (FR_RW_ERROR);

         dptr = &(fs->win[(scan->index & 15) * 32]);   // Pointer to the directory entry
         if (*dptr == 0)                        // Has it reached to end of dir?
            return (!ds ? FR_NO_FILE : FR_NO_PATH);
         if (    (*dptr != 0xE5)                  // Matched?
            && !(*(dptr+11) & AM_VOL)
            && !memcmp(dptr, fn, 8+3) )
             break;
         if (!next_dir_ptr(scan))               // Next directory pointer
            return (!ds ? FR_NO_FILE : FR_NO_PATH);
         }
      if (!ds) 
         { 
         *win_dir = dptr; 
         return (FR_OK);
          }   // Matched with end of path 

      if (!(*(dptr+11) & AM_DIR)) 
         return FR_NO_PATH;                     // Cannot trace because it is a file

      clust = ((DWORD)LD_WORD(dptr+20) << 16) | LD_WORD(dptr+26); // Get cluster# of the directory
      scan->clust = scan->sclust = clust;            // Restart scan with the new directory
      scan->sect = clust2sect(clust);
      scan->index = 0;
      }
   }




#ifndef _FS_READONLY
static
BYTE* reserve_direntry (DIR *scan)
///////////////////////////////////////////////////////////////////////////
//   Reserve a Directory Entry 
//
//   Entry:
//      DIR *scan         Target directory to create new entry
///////////////////////////////////////////////////////////////////////////
   {
   DWORD clust, sector;
   BYTE c, n, *dptr;

   // Re-initialize directory object 
   clust = scan->sclust;
   if (clust) {   // Dyanmic directory table 
      scan->clust = clust;
      scan->sect = clust2sect(clust);
   } else {      // Static directory table 
      scan->sect = fs->dirbase;
   }
   scan->index = 0;

   do 
      {
      if (!move_window(scan->sect)) 
         return (NULL);
      dptr = &(fs->win[(scan->index & 15) * 32]);      // Pointer to the directory entry 
      c = *dptr;
      if ((c == 0) || (c == 0xE5)) 
         return (dptr);      // Found an empty entry! 
   } while (next_dir_ptr(scan));                  // Next directory pointer 
   // Reached to end of the directory table 

   // Abort when static table or could not stretch dynamic table 
   if (!clust) 
       return (NULL);

   clust = create_chain(scan->clust);
   if (!(clust)) 
      return (NULL);

   // flush the contents of the current FATFS Window
   if (!move_window(0)) 
      return (0);

   // point to the first sector of the target cluster
   fs->winsect = sector = clust2sect(clust);         // Cleanup the expanded table 

   // flush the contents of the FATFS window
   memset(fs->win, 0, 512);

   // flush the contents of all sectors in the cluster
   for (n = fs->sects_clust; n; n--) 
      {
      if (disk_write(fs->win, sector, 1) != RES_OK) 
         return (NULL);
      sector++;
      }

   // flag the FATFS window as dirty
   // probably should flush it instead
   fs->winflag = 1;
   return (fs->win);
   }
#endif // _FS_READONLY 




FRESULT check_mounted (void)
///////////////////////////////////////////////////////////////////////////
//   Checks to ensure that the file system is mounted. The disk status is
//   tested to determine if it is in the initialised state. If the disk is
//   not in the initialised state the file system is checked to ensure
//   there are no open files. Open files with a drive state of
//   uninitialised is a fatal error indicating possible media change.
//
//   If the drive is not mounted and there are no apparent open files the
//   drive will be mounted (initialized).
//
///////////////////////////////////////////////////////////////////////////
   {
   if (!fs) 
      return (FR_NOT_ENABLED);      // Has the FileSystem been enabled? 

   if (disk_status() & STA_NOINIT) 
      {   // The drive has not been initialized 
      if (fs->files)               // Drive was uninitialized with any file left opend 
         return (FR_INCORRECT_DISK_CHANGE);
      else
         return (f_mountdrv());      // Initialize file system and return result  
      } 
   else 
      {                        // The drive has been initialized 
      if (!fs->fs_type)            // But the file system has not been initialized 
         return (f_mountdrv());      // Initialize file system and return resulut 
      }
   return FR_OK;                  // File system is valid 
   }



///////////////////////////////////////////////////////////////////////////
//         ********   Module Public Functions    ********
///////////////////////////////////////////////////////////////////////////

/*
void f_init(void)
///////////////////////////////////////////////////////////////////////////
//   Initialize the File System FAT Data Structure 
///////////////////////////////////////////////////////////////////////////
   {
   // initialize the File System Data Structure
//   fs = &FileSystem;
    memset(fs, 0, sizeof(FATFS));
   }
*/

void f_get_error_mesg(FRESULT Mesg, char *destination)
///////////////////////////////////////////////////////////////////////////
//   Returns string representation of File function return code 
///////////////////////////////////////////////////////////////////////////
   {
   switch (Mesg)
      {
      case FR_OK :
         strcpy(destination, "No Error");
         break;

      case FR_INVALID_NAME :
         strcpy(destination, "ILLEGAL FILE NAME");
         break;

      case FR_NOT_READY :
         strcpy(destination, "MEDIA NOT READY");
         break;

      case FR_NO_FILE :
         strcpy(destination, "FILE NOT FOUND");
         break;

      case FR_NO_PATH :
         strcpy(destination, "PATH NOT FOUND");
         break;

      case FR_DENIED :
         strcpy(destination, "ACCESS DENIED");
         break;

      case FR_DISK_FULL : 
         strcpy(destination, "MEDIA FULL");
         break;

      case FR_RW_ERROR :
         strcpy(destination, "R/W ERROR");
         break;

      case FR_INCORRECT_DISK_CHANGE :
         strcpy(destination, "INCORRECT MEDIA CHANGE");
         break;

      case FR_WRITE_PROTECTED : 
         strcpy(destination, "WRITE PROTECTED");
         break;

      case FR_NOT_ENABLED :
         strcpy(destination, "FS NOT ENABLED");
         break;

      case FR_NO_FILESYSTEM :
         strcpy(destination, "NO FILESYSTEM");
         break;

      default : 
         strcpy(destination, "UNKNOWN ERROR");
      }
   }



FRESULT f_mountdrv(void)
///////////////////////////////////////////////////////////////////////////
//   Initialise the SPI bus to the SD/MMC card
//   Initialise the SD/MMC card
//   Initialize file system data structure
//   Load File System Information
//
//   Entry
//      Card_CS      don't care      SD Card Chip Select condition
///////////////////////////////////////////////////////////////////////////
//   Return Values
//   FR_OK (0)
//      The function succeeded. *nclust contains the number of free clusters
//
//   FR_NOT_READY
//      Media not ready
//
//   FR_RW_ERROR
//      Low level media access error
//
//   FR_NOT_ENABLED
//      FatFs module is not enabled
//
//   FR_NO_FILESYSTEM
//      No valid FAT partition on the disk.
/////////////////////////////////////////////////////////////////////////// 

   {
   BYTE fat;
   DWORD sect, fatend, maxsect;

   if (!fs) 
      return (FR_NOT_ENABLED);

   // Initialize file system object (data structure) 
   f_init();

   // Initialize disk drive 
   Media_Status = disk_initialize();
   if (Media_Status & STA_NOINIT) {             // work-around to re-initialize 
      Media_Status = disk_initialize();         // if 1st attepmt fails
   }
   if (Media_Status & STA_NOINIT) return (FR_NOT_READY);

   // Search FAT partition 
   // Get Master Boot Record
   fat = check_fs(sect = 0);      // Check sector 0 as an SFD format 
   if (!fat) 
      {  
      // Not a FAT boot record, it will be an FDISK format
      // Check a partition listed in top of the partition table
      if (fs->win[0x1C2]) 
         {
         // Is the partition existing?
         sect = LD_DWORD(&(fs->win[0x1C6]));   // Partition offset in LBA 
         fat = check_fs(sect);            // Check the partition 
         }
      else
         printf("FAT at sector 0\r\n");
      }
   if (!fat) 
      return (FR_NO_FILESYSTEM);   // No FAT patition 

   // Initialize file system object 
   fs->fs_type = fat;                        // FAT type 

   // get the number of sectors per FAT
   if (fat == FS_FAT32)
      fs->sects_fat = LD_DWORD(&(fs->win[0x24]));
   else
      fs->sects_fat = LD_WORD(&(fs->win[0x16]));

   fs->sects_clust = fs->win[0x0D];            // Sectors per cluster 
   fs->n_fats = fs->win[0x10];                  // Number of FAT copies 
   fs->fatbase = sect + LD_WORD(&(fs->win[0x0E]));   // FAT start sector (physical) 
   fs->n_rootdir = LD_WORD(&(fs->win[0x11]));      // Number of root directory entries 

   fatend = fs->sects_fat * fs->n_fats + fs->fatbase;

   // find the directory starting cluster
   if (fat == FS_FAT32) 
      {
      fs->dirbase = LD_DWORD(&(fs->win[0x2C]));   // FAT32: Directory start cluster 
      fs->database = fatend;                   // FAT32: Data start sector (physical) 
      } 
   else 
      {
      fs->dirbase = fatend;                  // Directory start sector (physical) 
      fs->database = fs->n_rootdir / 16 + fatend;   // Data start sector (physical) 
      }
   maxsect = LD_DWORD(&(fs->win[0x20]));         // Calculate maximum cluster number 
   if (!maxsect) 
      maxsect = LD_WORD(&(fs->win[0x13]));
   fs->max_clust = (maxsect - fs->database + sect) / fs->sects_clust + 2;

   return (FR_OK);
   }



FRESULT f_getfree (DWORD *nclust)
///////////////////////////////////////////////////////////////////////////
//   Returns the number of free clusters
//
//   Entry
//      *nclust   Pointer to the double word to return number of free clusters 
///////////////////////////////////////////////////////////////////////////
//   Return Values
//   FR_OK (0)
//      The function succeeded. *nclust contains the number of free clusters
//
//   FR_NOT_READY
//      Media not ready
//
//   FR_RW_ERROR
//      Low level media access error
//
//   FR_INCORRECT_DISK_CHANGE
//      Incorrect disk removal/change has occured
//
//   FR_NOT_ENABLED
//      FatFs module is not enabled
//
//   FR_NO_FILESYSTEM
//      No valid FAT partition on the disk.
///////////////////////////////////////////////////////////////////////////
   {
   DWORD n, clust, sect;
   BYTE fat, f, *p;
   FRESULT result;

   // check and mount the media if required
   result = check_mounted();
   if (result != FR_OK) 
      return (result);

   // Count number of free clusters 
   fat = fs->fs_type;
   n = 0;
   if (fat == FS_FAT12) 
      {
      clust = 2;
      do 
         {
         if ((WORD)get_cluster(clust) == 0) 
            n++;
         } while (++clust < fs->max_clust);
      }
   else 
      {
      clust = fs->max_clust;
      sect = fs->fatbase;
      f = 0; p = 0;
      do 
         {
         if (!f) 
            {
            if (!move_window(sect++)) 
               return (FR_RW_ERROR);
            p = fs->win;
            }
         if (fat == FS_FAT16) 
            {
            if (LD_WORD(p) == 0) 
               n++;
            p += 2;
            f += 1;
            } 
         else 
            {
            if (LD_DWORD(p) == 0)
               n++;
            p += 4;
            f += 2;
            }
         } while (--clust);
      }

   *nclust = n;
   return (FR_OK);
   }



FRESULT f_open (FIL *fp, char *path, BYTE mode)
///////////////////////////////////////////////////////////////////////////
//   Open or Create a File
//
//   Entry
//      *fp,      Pointer to the buffer of new file object to create
//      *path      Pointer to the filename
//      mode      Access mode and file open mode flags
//
///////////////////////////////////////////////////////////////////////////
//   mode flags:
//    FA_READ
//      Specifies read access to the object. Data can be read from the file.
//      Combine with FA_WRITE for read-write access.
//
//   FA_WRITE   
//      Specifies write access to the object. Data can be written to the file.
//      Combine with FA_READ for read-write access
//
//   FA_OPEN_EXISTING
//      Opens the file. The function fails if the file does not exist
//
//   FA_CREATE_ALWAYS
//      Creates a new file. If the file exists, it is truncated and overwritten
//
//   FA_OPEN_ALWAYS   
//      Opens the file, if it exists, creates it otherwise
//
///////////////////////////////////////////////////////////////////////////
//   Return Values
//   FR_OK (0)
//      The function succeeded. The FileObject structure is used for subsequent 
//      calls to refer to the file. Use function f_close() to close
//
//   FR_NO_FILE
//       File not found
//
//   FR_NO_PATH
//      Could not find the path.
//
//   FR_INVALID_NAME
//      Invalid Filename
//
//   FR_DENIED
//      File Access Denied. This error is generated under the following conditions:
//         write mode open of a file that has a read-only attribute, 
//         file creation when a file of the same name already exists in the directory
//         attemp to create or write to a read-only file,
//         directory or disk full
//
//   FR_NOT_READY
//      Media not ready
//
//   FR_WRITE_PROTECTED
//      Attempt to open a file for write access on write protected media
//
//   FR_RW_ERROR
//      Low level media access error
//
//   FR_INCORRECT_DISK_CHANGE
//      Incorrect disk removal/change has occured
//
//   FR_NOT_ENABLED
//      FatFs module is not enabled
//
//   FR_NO_FILESYSTEM
//      No valid FAT partition on the disk.
///////////////////////////////////////////////////////////////////////////
{
   DIR dirscan;
   FRESULT result;
   BYTE *dir_ptr;

   char fn[8+3+1];

   // mount the media and initialize the file system if necessary
   result = check_mounted();
   if (result != FR_OK) 
      return (result);

   #ifndef _FS_READONLY
      // if opened in write mode, check the media is not write protected
      if ((mode & (FA_WRITE|FA_CREATE_ALWAYS)) && (disk_status() & STA_PROTECT))
         return (FR_WRITE_PROTECTED);
   #endif

   // test if the file exists
   result = trace_path(&dirscan, fn, path, &dir_ptr);   // Trace the file path 

   #ifndef _FS_READONLY
      // Create or Open a File 
   if (mode & (FA_CREATE_ALWAYS|FA_OPEN_ALWAYS)) 
   {
      DWORD dw;
      // check to see if the file exists
      if (result != FR_OK) 
      {
         // file does not exist
         mode |= FA_CREATE_ALWAYS;
         if (result != FR_NO_FILE)
            return (result);

         // reserve an entry in the directory for this file
         dir_ptr = reserve_direntry(&dirscan);
         if (dir_ptr == NULL)
            return (FR_DENIED);

         // initialise the new directory entry
         memcpy(dir_ptr, fn, 8+3);
         *(dir_ptr+12) = fn[11];
         memset(dir_ptr+13, 0, 32-13);
      } 
      else 
      {
         // the file already exists - chaeck file access rights
         if ((dir_ptr == NULL) || (*(dir_ptr+11) & (AM_RDO|AM_DIR)))   // Could not overwrite (R/O or DIR) 
            return (FR_DENIED);

         // if mode is alway create then reset the file to zero length
         // and remove the cluster chain
         if (mode & FA_CREATE_ALWAYS) 
         {
            // remove the cluster chain
            // get the current sector in the FATFS window
            // this will point to the first cluster of the file
            // as a result of the trace_path()
            dw = fs->winsect;

            if (!remove_chain(((DWORD)LD_WORD(dir_ptr+20) << 16) | LD_WORD(dir_ptr+26)) || !move_window(dw) )
               return (FR_RW_ERROR);

            ST_WORD(dir_ptr+20, 0); 
            ST_WORD(dir_ptr+26, 0);   // cluster = 0 
            ST_DWORD(dir_ptr+28, 0);               // size = 0 
         }
      }

      if (mode & FA_CREATE_ALWAYS) 
      {
         *(dir_ptr+11) = AM_ARC;
         dw = get_fattime();
         ST_DWORD(dir_ptr+14, dw);   // Created time 
         ST_DWORD(dir_ptr+22, dw);   // Updated time 
         fs->winflag = 1;
      }
   }
   // Open a File 
   else 
   {
   #endif // _FS_READONLY 

      if (result != FR_OK) 
         return (result);      // Trace failed 

      if ((dir_ptr == NULL) || (*(dir_ptr+11) & AM_DIR))   // It is a directory 
         return (FR_NO_FILE);

   #ifndef _FS_READONLY
      if ((mode & FA_WRITE) && (*(dir_ptr+11) & AM_RDO)) // R/O violation 
         return (FR_DENIED);
   }
   #endif

   #ifdef _FS_READONLY
      fp->flag = mode & FA_READ;
   #else
      fp->flag = mode & (FA_WRITE|FA_READ);
      fp->dir_sect = fs->winsect;            // Pointer to the current directory sector in the FATFS window 
      fp->dir_ptr = dir_ptr;               // pointer to directory offset in the window
   #endif
   fp->org_clust =   ((DWORD)LD_WORD(dir_ptr+20) << 16) | LD_WORD(dir_ptr+26);   // File start cluster 
   fp->fsize = LD_DWORD(dir_ptr+28);      // File size
   fp->fptr = 0;                     // File ptr  - point to the start of the file
   fp->sect_clust = 1;                  // Remaining sectors in cluster (forces new cluster)
   fs->files++;                     // update the number of files open count
   return (FR_OK);
}

// File System Lite Mode = USE_FAT_LITE defined
FRESULT f_read(FIL *fp, void *buff, WORD btr, WORD *br)
   ///////////////////////////////////////////////////////////////////////////
   //   Read a file
   //
   //   Entry:
   //      fp         Pointer to the file object
   //      buff      Pointer to the data buffer
   //      btw         Number of bytes to read
   //      bw         Pointer to number of bytes to read
   ///////////////////////////////////////////////////////////////////////////
   //   Return Values
   //   FR_OK (0)
   //      The function succeeded. The FileObject structure is used for subsequent 
   //      calls to refer to the file. Use function f_close() to close
   //
   //   FR_DENIED
   //      File Access Denied. File already open in write access mode
   //
   //   FR_NOT_READY
   //      Media not ready
   //
   //   FR_RW_ERROR
   //      Low level media access error
   //
   //   FR_NOT_ENABLED
   //      FatFs module is not enabled
   ///////////////////////////////////////////////////////////////////////////
   
      {
      DWORD clust, sect, ln;
      WORD rcnt;
      BYTE cc, *rbuff;
   
      rbuff = buff;
      *br = 0;
      if (!fs) 
         return (FR_NOT_ENABLED);
   
      if ((disk_status() & STA_NOINIT) || !fs->fs_type) 
         return (FR_NOT_READY);   // Check disk ready 
   
      if (fp->flag & FA__ERROR)
         return (FR_RW_ERROR);   // Check error flag 
   
      if (!(fp->flag & FA_READ))
         return (FR_DENIED);      // Check access mode 
   
      ln = fp->fsize - fp->fptr;
      if (btr > ln)
         btr = ln;            // Truncate read count by number of bytes left 
   
      for ( ;  btr;               // Repeat until all data transferred 
         rbuff += rcnt, fp->fptr += rcnt, *br += rcnt, btr -= rcnt) 
         {
         if ((fp->fptr % 512) == 0) 
            {   // On the sector boundary 
            if (--(fp->sect_clust)) 
               {   // Decrement sector counter 
               sect = fp->curr_sect + 1;         // Next sector 
               } 
            else 
               {   // Next cluster 
               clust = (fp->fptr == 0) ? fp->org_clust : get_cluster(fp->curr_clust);
               if ((clust < 2) || (clust >= fs->max_clust))
                  goto fr_error;
               fp->curr_clust = clust;            // Current cluster 
               sect = clust2sect(clust);         // Current sector 
               fp->sect_clust = fs->sects_clust;   // Re-initialize the sector counter 
               }
   
            fp->curr_sect = sect;               // Update current sector 
            cc = btr / 512;                     // When left bytes >= 512 
            if (cc) 
               {   // Read maximum contiguous sectors 
               if (cc > fp->sect_clust) 
                  cc = fp->sect_clust;
               if (disk_read(rbuff, sect, cc) != RES_OK) 
                  goto fr_error;
               fp->sect_clust -= cc - 1;
               fp->curr_sect += cc - 1;
               rcnt = cc * 512; 
               continue;
               }
            }
   
         if (!move_window(fp->curr_sect))         // Move the sector window 
               goto fr_error;
         rcnt = 512 - (fp->fptr % 512);      // Copy fractional bytes from file I/O buffer 
         if (rcnt > btr)
            rcnt = btr;
         memcpy(rbuff, &fs->win[fp->fptr % 512], rcnt);
         }
      return (FR_OK);
   
   fr_error:   // Abort this file due to an unrecoverable error 
      fp->flag |= FA__ERROR;
      return (FR_RW_ERROR);
      }
   
   
   
   #ifndef _FS_READONLY
   FRESULT f_write (FIL *fp, void *buff, WORD btw, WORD *bw)
   ///////////////////////////////////////////////////////////////////////////
   //   Write to a file
   //
   //   Entry:
   //      fp         Pointer to the file object
   //      buff      Pointer to the data to be written
   //      btw         Number of bytes to write
   //      bw         Pointer to number of bytes written
   ///////////////////////////////////////////////////////////////////////////
   //   Return Values
   //   FR_OK (0)
   //      The function succeeded
   //
   //   FR_DENIED
   //      File Access Denied. Attempt to write to a read-only file
   //
   //   FR_NOT_READY
   //      Media not ready
   //
   //   FR_RW_ERROR
   //      Low level media access error
   //
   //   FR_NOT_ENABLED
   //      FatFs module is not enabled
   ///////////////////////////////////////////////////////////////////////////
      {
      DWORD clust, sect;
      WORD wcnt;
      BYTE cc;
      BYTE *wbuff;
   
      wbuff = buff;
      *bw = 0;
      wcnt = 0;      // bug fix to original 0.2 code
   
      // check the fs pointer to the FATFS data structure is valid
      if (!fs) 
         return (FR_NOT_ENABLED);
   
      // check the card is in the initialized state and the File System is recognized
      if ((disk_status() & STA_NOINIT) || !fs->fs_type)
         return (FR_NOT_READY);
   
      // if an existing error in the file pointer then exit
      if (fp->flag & FA__ERROR)
         return (FR_RW_ERROR);         // Check error flag 
   
      // ensure the file is openned for write operations
      if (!(fp->flag & FA_WRITE)) 
         return (FR_DENIED);            // Check access mode 
   
      // ensure no pointer rollover will occur
      if (fp->fsize + btw < fp->fsize) 
         btw = 0;                  // File size cannot reach 4GB 
   
      // write to Media loop
      for ( ;  btw; wbuff += wcnt, fp->fptr += wcnt, *bw += wcnt, btw -= wcnt) 
         // Repeat until all data transferred
         {
         if ((fp->fptr % 512) == 0) 
            {
            // On the sector boundary, decrement sector counter
            if (--(fp->sect_clust)) 
               {
               sect = fp->curr_sect + 1;   // point to the next sector
               } 
            else 
               {
               // finished this cluster, get next cluster
               // at the beginning of the file?
               if (fp->fptr == 0) 
                  {
                  // at start of the file
                  clust = fp->org_clust;
                  if (clust == 0)            // No cluster is created 
                     fp->org_clust = clust = create_chain(0);   // Create a new cluster chain
                  }
               else 
                  {   // Middle or end of file 
                  clust = create_chain(fp->curr_clust);   // Trace or stretch cluster chain 
                  }
               if ((clust < 2) || (clust >= fs->max_clust))
                  break;
               fp->curr_clust = clust;            // Current cluster 
               sect = clust2sect(clust);         // Current sector 
               fp->sect_clust = fs->sects_clust;      // Re-initialize the sector counter 
               }
   
            fp->curr_sect = sect;               // Update current sector 
            cc = btw / 512;                  // When left bytes >= 512 
            if (cc) 
               {   // Write maximum contiguous sectors 
               if (cc > fp->sect_clust) 
                  cc = fp->sect_clust;
               if (disk_write(wbuff, sect, cc) != RES_OK) 
                  goto fw_error;
               fp->sect_clust -= cc - 1;
               fp->curr_sect += cc - 1;
               wcnt = cc * 512;
               continue;
               }
   
            if (fp->fptr >= fp->fsize) 
               {
               if (!move_window(0))        // Flush R/W window if needed
                  goto fw_error;
               fs->winsect = fp->curr_sect;
               }
            }
   
         if (!move_window(fp->curr_sect)) 
            goto fw_error;               // Move sector window
   
         wcnt = 512 - (fp->fptr % 512);         // Copy fractional bytes to file I/O buffer 
         if (wcnt > btw)
            wcnt = btw;
         memcpy(&fs->win[fp->fptr % 512], wbuff, wcnt);
         fs->winflag = 1;
         }
   
      if (fp->fptr > fp->fsize) 
         fp->fsize = fp->fptr;      // Update file size if needed 
      fp->flag |= FA__WRITTEN;      // Set file changed flag 
      return (FR_OK);
   
   fw_error:   // Abort this file due to an unrecoverable error 
      fp->flag |= FA__ERROR;
      return (FR_RW_ERROR);
      }
   #endif // _FS_READONLY 
   
   
   
   FRESULT f_lseek (FIL *fp, DWORD ofs)
   ///////////////////////////////////////////////////////////////////////////
   //   Seek File Pointer 
   //
   //   Entry:
   //      *fp      Pointer to the file object
   //      ofs      File pointer from top of file
   ///////////////////////////////////////////////////////////////////////////
   //   Return Values
   //   FR_OK (0)
   //      The function succeeded
   //
   //   FR_INCORRECT_DISK_CHANGE
   //      Incorrect disk removal/change has occured
   //
   //   FR_RW_ERROR
   //      Low level media access error
   //
   //   FR_NOT_ENABLED
   //      FatFs module is not enabled
   ///////////////////////////////////////////////////////////////////////////
      {
      DWORD clust;
      BYTE sc;
   
      if (!fs)
         return (FR_NOT_ENABLED);
   
      if ((disk_status() & STA_NOINIT) || !fs->fs_type) 
         return (FR_NOT_READY);
   
      if (fp->flag & FA__ERROR)
         return (FR_RW_ERROR);
   
      if (ofs > fp->fsize) 
         ofs = fp->fsize;               // Clip offset by file size 
      fp->fptr = ofs; 
      fp->sect_clust = 1;                // Re-initialize file pointer 
   
      // Seek file pointer if needed 
      if (ofs) 
         {
         ofs = (ofs - 1) / 512;            // Calcurate current sector 
         sc = fs->sects_clust;            // Number of sectors in a cluster 
         fp->sect_clust = sc - (ofs % sc);      // Calcurate sector counter 
         ofs /= sc;                  // Number of clusters to skip 
         clust = fp->org_clust;            // Seek to current cluster 
   
         while (ofs--)
            clust = get_cluster(clust);
   
         if ((clust < 2) || (clust >= fs->max_clust)) 
            goto fk_error;
   
         fp->curr_clust = clust;
         fp->curr_sect = clust2sect(clust) + sc - fp->sect_clust;   // Current sector 
         }
      return (FR_OK);
   
   fk_error:   // Abort this file due to an unrecoverable error 
      fp->flag |= FA__ERROR;
      return (FR_RW_ERROR);
      }
   
    
#ifndef _FS_READONLY
FRESULT f_append(char *fname)
   ///////////////////////////////////////////////////////////////////////////
   // Opens the data file passed in fname. If the file does not exist then it
   //  is created, if the file exists then the file is appended
   ///////////////////////////////////////////////////////////////////////////
{
   FRESULT result;       // file system result code
   FIL fdata;
    
   // open the file - if it does not already exist, then create the file
   result = f_open(&fdata, fname, FA_OPEN_ALWAYS | FA_WRITE);
    
   // if the file already exists then append to the end of the file
   if (!result)
   {
      if (fdata.fsize != 0)
         result = f_lseek(&fdata, fdata.fsize);
   }

   return(result);
}

FRESULT f_sync (FIL *fp)
   ///////////////////////////////////////////////////////////////////////////
   //   Synchronize between File and Disk without Close 
   //
   //   Entry:
   //      *fp      Pointer to the file object
   ///////////////////////////////////////////////////////////////////////////
   //   Return Values
   //   FR_OK (0)
   //      The function succeeded
   //
   //   FR_RW_ERROR
   //      Low level media access error
   //
   //   FR_INCORRECT_DISK_CHANGE
   //      Incorrect disk removal/change has occured
   //
   //   FR_NOT_ENABLED
   //      FatFs module is not enabled
   ///////////////////////////////////////////////////////////////////////////
{
   BYTE *ptr;

   if (!fs)
      return (FR_NOT_ENABLED);

   if ((disk_status() & STA_NOINIT) || !fs->fs_type)
      return (FR_INCORRECT_DISK_CHANGE);

   // Has the file been written? 
   if (fp->flag & FA__WRITTEN)
   {
      // Update the directory entry
      if (!move_window(fp->dir_sect))
         return (FR_RW_ERROR);
      ptr = fp->dir_ptr;
      *(ptr+11) |= AM_ARC;                   // Set archive bit 
      ST_DWORD(ptr+28, fp->fsize);           // Update file size 
      ST_WORD(ptr+26, fp->org_clust);        // Update start cluster 
      ST_WORD(ptr+20, fp->org_clust >> 16);
      ST_DWORD(ptr+22, get_fattime());       // Updated time 
      fs->winflag = 1;
      fp->flag &= ~FA__WRITTEN;
   }

   if (!move_window(0)) 
      return (FR_RW_ERROR);

   return (FR_OK);
}
      
#endif // _FS_READONLY 

FRESULT f_close (FIL *fp)
///////////////////////////////////////////////////////////////////////////
//   Close a file
//
//   Entry:
//      *fp      Pointer to the file object to be closed 
///////////////////////////////////////////////////////////////////////////
//   Return Values
//   FR_OK (0)
//      The file has been sucessfully closed
//
//   FR_RW_ERROR
//      Low level media access error
//
//   FR_INCORRECT_DISK_CHANGE
//      Incorrect disk removal/change has occured
//
//   FR_NOT_ENABLED
//      FatFs module is not enabled
///////////////////////////////////////////////////////////////////////////
   {
   FRESULT res;

   #ifndef _FS_READONLY
      res = f_sync(fp);
   #else
      res = FR_OK;
   #endif

   if (res == FR_OK) 
      {
      fp->flag = 0;
      if (fs->files)
         fs->files--;
      }
   return (res);
   }

#ifndef _FS_READONLY
FRESULT f_unlink (char *path)
///////////////////////////////////////////////////////////////////////////
//   Delete a file or directory
//
//   Entry:
//      path      Pointer to the file or directory path 
///////////////////////////////////////////////////////////////////////////
//   Return Values
//   FR_OK (0)
//      The function succeeded
//
//   FR_NO_FILE
//       File not found
//
//   FR_NO_PATH
//      Could not find the path.
//
//   FR_INVALID_NAME
//      Invalid Filename
//
//   FR_DENIED
//      File Access Denied. The file or directory has a read only attribue or the
//      directory is not empty.
//
//   FR_NOT_READY
//      Media not ready
//
//   FR_WRITE_PROTECTED
//      Attempt to open a file for write access on write protected media
//
//   FR_RW_ERROR
//      Low level media access error
//
//   FR_INCORRECT_DISK_CHANGE
//      Incorrect disk removal/change has occured
//
//   FR_NOT_ENABLED
//      FatFs module is not enabled
//
//   FR_NO_FILESYSTEM
//      No valid FAT partition on the disk.
///////////////////////////////////////////////////////////////////////////
   {
   FRESULT res;
   BYTE *dir_ptr, *sdir;
   DWORD dclust, dsect;
   DIR dirscan;
   char fn[8+3+1];

   res = check_mounted();
   if (res != FR_OK) 
      return (res);

   if (disk_status() & STA_PROTECT)
      return (FR_WRITE_PROTECTED);

   res = trace_path(&dirscan, fn, path, &dir_ptr);   // Trace the file path 

   if (res != FR_OK) 
      return (res);            // Trace failed 

   if (dir_ptr == NULL) 
      return (FR_NO_FILE);      // It is a root directory 

   if (*(dir_ptr+11) & AM_RDO) 
      return (FR_DENIED);         // It is a R/O item 

   dsect = fs->winsect;
   dclust = ((DWORD)LD_WORD(dir_ptr+20) << 16) | LD_WORD(dir_ptr+26);

   if (*(dir_ptr+11) & AM_DIR) 
      {   // It is a sub-directory 
      dirscan.clust = dclust;               // Check if the sub-dir is empty or not 
      dirscan.sect = clust2sect(dclust);
      dirscan.index = 0;
      do 
         {
         if (!move_window(dirscan.sect))
            return (FR_RW_ERROR);
         sdir = &(fs->win[(dirscan.index & 15) * 32]);
         if (*sdir == 0)
            break;

         if (!((*sdir == 0xE5) || (*sdir == '.')) && !(*(sdir+11) & AM_VOL))
            return (FR_DENIED);   // The directory is not empty 
         } while (next_dir_ptr(&dirscan));
      }

   if (!move_window(dsect))
      return (FR_RW_ERROR);   // Mark the directory entry 'deleted' 

   *dir_ptr = 0xE5; 
   fs->winflag = 1;
   if (!remove_chain(dclust)) 
      return (FR_RW_ERROR);   // Remove the cluster chain 

   if (!move_window(0)) 
      return (FR_RW_ERROR);

   return (FR_OK);
   }
#endif // _FS_READONLY 

#ifndef _FS_READONLY
FRESULT f_mkdir (char *path)
///////////////////////////////////////////////////////////////////////////
//   Create a directory
//
//   Entry:
//      path      Pointer to the directory path 
///////////////////////////////////////////////////////////////////////////
//   Return Values
//   FR_OK (0)
//      The function succeeded. The FileObject structure is used for subsequent 
//      calls to refer to the file. Use function f_close() to close
//
//   FR_NO_FILE
//       File not found
//
//   FR_NO_PATH
//      Could not find the path.
//
//   FR_INVALID_NAME
//      Invalid Filename
//
//   FR_DENIED
//      File Access Denied. File or directoy already exists or the
//         directory or disk is full
//
//   FR_NOT_READY
//      Media not ready
//
//   FR_WRITE_PROTECTED
//      Attempt to open a file for write access on write protected media
//
//   FR_RW_ERROR
//      Low level media access error
//
//   FR_INCORRECT_DISK_CHANGE
//      Incorrect disk removal/change has occured
//
//   FR_NOT_ENABLED
//      FatFs module is not enabled
//
//   FR_NO_FILESYSTEM
//      No valid FAT partition on the disk.
///////////////////////////////////////////////////////////////////////////
   {
   FRESULT res;
   BYTE *dir_ptr, *w, n;
   DWORD sect, dsect, dclust, pclust, tim;
   DIR dirscan;
   char fn[8+3+1];

   // check if the file system is mounted
   res = check_mounted();
   if (res != FR_OK) 
      return (res);

   if (disk_status() & STA_PROTECT)
      return (FR_WRITE_PROTECTED);

   res = trace_path(&dirscan, fn, path, &dir_ptr);   // Trace the file path 

   if (res == FR_OK) 
      return (FR_DENIED);      // Any file or directory is already existing 

   if (res != FR_NO_FILE) 
      return (res);

   dir_ptr = reserve_direntry(&dirscan);      // Reserve a directory entry 
   if (dir_ptr == NULL) 
      return (FR_DENIED);

   sect = fs->winsect;
   dsect = clust2sect(dclust = create_chain(0));   // Get a new cluster for new directory 

   if (!dsect) 
      return (FR_DENIED);

   if (!move_window(0)) 
      return (0);

   w = fs->win;
   memset(w, 0, 512);                  // Initialize the directory table 
   for (n = fs->sects_clust - 1; n; n--) 
      {
      if (disk_write(w, dsect+n, 1) != RES_OK) 
         return (FR_RW_ERROR);
      }

   fs->winsect = dsect;               // Create dot directories 
   memset(w, ' ', 8+3);
   *w = '.';
   *(w+11) = AM_DIR;
   tim = get_fattime();
   ST_DWORD(w+22, tim);

   ST_WORD(w+26, dclust);
   ST_WORD(w+20, dclust >> 16);
   memcpy(w+32, w, 32); *(w+33) = '.';
   pclust = dirscan.sclust;
   if (fs->fs_type == FS_FAT32 && pclust == fs->dirbase) pclust = 0;
   ST_WORD(w+32+26, pclust);
   ST_WORD(w+32+20, pclust >> 16);
   fs->winflag = 1;

   if (!move_window(sect)) 
      return (FR_RW_ERROR);

   memcpy(dir_ptr, fn, 8+3);         // Initialize the new entry 
   *(dir_ptr+11) = AM_DIR;
   *(dir_ptr+12) = fn[11];
   memset(dir_ptr+13, 0, 32-13);
   ST_DWORD(dir_ptr+22, tim);         // Crated time 
   ST_WORD(dir_ptr+26, dclust);      // Table start cluster 
   ST_WORD(dir_ptr+20, dclust >> 16);
   fs->winflag = 1;

   if (!move_window(0)) 
      return (FR_RW_ERROR);

   return (FR_OK);
   }
#endif // _FS_READONLY 



FRESULT f_opendir (DIR *scan, char *path)
///////////////////////////////////////////////////////////////////////////
//   Open directory. Checks and, if necessary mounts the media.
//
//   Entry:
//      *scan      Pointer to the directory object to initialize
//      *path      Pointer to the directory path, null str means the root
//      SD_CS      don't care 
//
// Note: The directory separator is '/'. Because the FatFs module does not 
// have a concept of current directory, a full-path name that followed from
// the root directory must be used. Leading space charactors are skipped if 
// exist and heading '/' can be exist or omitted. The target directory name 
// cannot be followed by a '/'. When open the root directory, specify "" or "/"
//
///////////////////////////////////////////////////////////////////////////
//   Return Values
//   FR_OK (0)
//      The function succeeded. The FileObject structure is used for subsequent 
//      calls to refer to the file. Use function f_close() to close
//
//   FR_NO_FILE
//       File not found
//
//   FR_NO_PATH
//      Could not find the path.
//
//   FR_INVALID_NAME
//      Invalid Filename
//
//   FR_NOT_READY
//      Media not ready
//
//   FR_RW_ERROR
//      Low level media access error
//
//   FR_INCORRECT_DISK_CHANGE
//      Incorrect disk removal/change has occured
//
//   FR_NOT_ENABLED
//      FatFs module is not enabled
//
//   FR_NO_FILESYSTEM
//      No valid FAT partition on the disk.
///////////////////////////////////////////////////////////////////////////
   {
   FRESULT result;
   BYTE *dir_ptr;
   char fn[8+3+1];

   result = check_mounted();
   if (result != FR_OK) 
      return (result);

   result = trace_path(scan, fn, path, &dir_ptr);   // Trace the directory path 
   if (result == FR_OK) 
      {         // Trace completed 
      if (dir_ptr != NULL) 
         {      // It is not a root dir 
         if (*(dir_ptr+11) & AM_DIR) 
            {      // The entry is a directory 
            scan->clust = ((DWORD)LD_WORD(dir_ptr+20) << 16) | LD_WORD(dir_ptr+26);
            scan->sect = clust2sect(scan->clust);
            scan->index = 0;
            } 
         else 
            {   // The entry is a file 
            result = FR_NO_PATH;
            }
         }
      }
   return (result);
   }



FRESULT f_readdir ( DIR *scan, FILINFO *finfo)
///////////////////////////////////////////////////////////////////////////
//   Read Directory Entry in Sequense
//
//   Entry:
//      *scan      Pointer to the directory object
//      *finfo      Pointer to file information to return
///////////////////////////////////////////////////////////////////////////
//   Return Values
//   FR_OK (0)
//      The function succeeded. The FileObject structure is used for subsequent 
//      calls to refer to the file. Use function f_close() to close
//
//   FR_NOT_READY
//      Media not ready
//
//   FR_RW_ERROR
//      Low level media access error
//
//   FR_NOT_ENABLED
//      FatFs module is not enabled
///////////////////////////////////////////////////////////////////////////
   {
   BYTE *dir_ptr, c;

   if (!fs) 
      return (FR_NOT_ENABLED);
   finfo->fname[0] = 0;

   if ((disk_status() & STA_NOINIT) || !fs->fs_type)
      return (FR_NOT_READY);

   while (scan->sect) 
      {
      if (!move_window(scan->sect))
         return (FR_RW_ERROR);

      dir_ptr = &(fs->win[(scan->index & 15) * 32]);      // pointer to the directory entry 
      c = *dir_ptr;
      if (c == 0)    // Has it reached to end of dir? 
         break;         

      // test to ensure the entry has not been erased            
      if ((c != 0xE5) && (c != '.') && !(*(dir_ptr+11) & AM_VOL))   // Is it a valid entry? 
         get_fileinfo(finfo, dir_ptr);

      if (!next_dir_ptr(scan)) 
         scan->sect = 0;      // Next entry 

      if (finfo->fname[0])
         // Found valid entry
          break;            
      }
   return FR_OK;
   }



FRESULT f_stat ( char *path, FILINFO *finfo)
///////////////////////////////////////////////////////////////////////////
//   Get the file status
//
//   Entry:
//      *path      Pointer to the file path
//      *finfo      Pointer to file information to return 
///////////////////////////////////////////////////////////////////////////
//   Return Values
//   FR_OK (0)
//      The function succeeded
//
//   FR_NO_FILE
//       File not found
//
//   FR_NO_PATH
//      Could not find the path.
//
//   FR_INVALID_NAME
//      Invalid Filename
//
//   FR_NOT_READY
//      Media not ready
//
//   FR_WRITE_PROTECTED
//      Attempt to open a file for write access on write protected media
//
//   FR_RW_ERROR
//      Low level media access error
//
//   FR_INCORRECT_DISK_CHANGE
//      Incorrect disk removal/change has occured
//
//   FR_NOT_ENABLED
//      FatFs module is not enabled
//
//   FR_NO_FILESYSTEM
//      No valid FAT partition on the disk.
///////////////////////////////////////////////////////////////////////////
   {
   DIR dirscan;
   FRESULT res;
   BYTE *dir_ptr;

   char fn[8+3+1];

   // test if the file system is mounted
   res = check_mounted();
   if (res != FR_OK) 
      return (res);

   res = trace_path(&dirscan, fn, path, &dir_ptr);      // Trace the file path 
   if (res == FR_OK)
      {
      if (dir_ptr)               
         get_fileinfo(finfo, dir_ptr);   // Trace completed
      else
         res = FR_INVALID_NAME;      // It is a root directory
      }
   return (res);
   }

#define _FAT_FS
#endif
