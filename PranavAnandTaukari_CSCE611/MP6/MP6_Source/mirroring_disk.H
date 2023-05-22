#ifndef _MIRRORING_DISK_H_
#define _MIRRORING_DISK_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "simple_disk.H"
#include "blocking_disk.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* B l o c k i n g D i s k  */
/*--------------------------------------------------------------------------*/

class MirroringDisk : public BlockingDisk {

private:
    BlockingDisk * MASTER_DISK;
    BlockingDisk * DEPENDENT_DISK;
    
public:
   MirroringDisk(DISK_ID _disk_id, unsigned int _size); 
 

   /* DISK OPERATIONS */

   virtual void read(unsigned long _block_no, unsigned char * _buf);
   /* Reads 512 Bytes from the given block of the disk and copies them 
      to the given buffer. No error check! */

   virtual void write(unsigned long _block_no, unsigned char * _buf);
   /* Writes 512 Bytes from the buffer to the given block on the disk. */

};

#endif
