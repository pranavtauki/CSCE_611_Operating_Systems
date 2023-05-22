/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
#define FREE 'f'
#define USED 'u'
#define INODE_BLOCK 1
#define FREE_BLOCK 0

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/





/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
    disk = NULL;
    size = 0;
    free_blocks = new unsigned char[free_block_count];
    inode_counter= 0;
    inodes = new Inode[MAX_INODES];
  
}


Inode::Inode()
{
  fs = NULL;
  inode_free = true;
  file_size= 0;
  id = -1;
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the free list are saved. */
    disk->write(FREE_BLOCK,free_blocks);
    unsigned char* tmp = (unsigned char*) inodes;
    disk->write(INODE_BLOCK,tmp);
    
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/


bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");

    /* Here you read the inode list and the free list into memory */
    
    disk = _disk;
    unsigned char* tmp;
    _disk->read(FREE_BLOCK,free_blocks);
    _disk->read(INODE_BLOCK,tmp);
    inodes = (Inode *) tmp;
    inode_counter= 0;

    for (int i=0 ;i<MAX_INODES ;i++){
      if (!inodes[i].inode_free){
        inode_counter++;
      }
    }

    free_block_count = SimpleDisk::BLOCK_SIZE/sizeof(unsigned char) ;
    return true;

}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
    int count_of_free_blocks = _size/SimpleDisk::BLOCK_SIZE;
    unsigned char* arr_free_blocks = new unsigned char[count_of_free_blocks];
    
    arr_free_blocks[0] = USED;
    arr_free_blocks[1] = USED;
    
    int i =2;
    while(i<count_of_free_blocks)
    {
        arr_free_blocks[i]=FREE;
        i++;
    }

    _disk->write(FREE_BLOCK,arr_free_blocks);
    Inode* tmp_inodes = new Inode[MAX_INODES];
    unsigned char* tmp_write = (unsigned char*) tmp_inodes;
    _disk->write(INODE_BLOCK,tmp_write);
    return true;

}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    // Console::puts("\n");
    for (int i = 0;i<MAX_INODES;i++){
      if (inodes[i].id==_file_id){
        // Console::puti(inodes[i].id);
        // Console::puts("\n");
        return &inodes[i];
      }

    }
    //Console::puts("going to return null\n");
    return NULL;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    if(LookupFile(_file_id)==NULL)
    {
    int free_inode_index = -1;
    int i =0;

    while(i<MAX_INODES)
    {
     if (inodes[i].inode_free){
        free_inode_index = i;
        break;
      }
      i++;   
    }


    int free_block_index = -1;
    int j =0;

    // while(j<free_block_count)
    // {
    //   if (free_blocks[j]==FREE)
    //   {
    //     free_block_index = j;
    //     break;
    //   }
    //   j++;  
    // }

    free_block_index = GetFreeBlock();
       
    if(free_inode_index==-1 || free_block_index==-1)
        return false;
    inodes[free_inode_index].inode_free = false;
    inodes[free_inode_index].fs = this;
    inodes[free_inode_index].block_no = free_block_index;
    inodes[free_inode_index].id = _file_id;
    free_blocks[free_block_index] = USED;
    }
    return true;

    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
    
}

bool FileSystem::DeleteFile(int _file_id) {
    
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */
    
    Inode *inode = LookupFile(_file_id);

    int blk_no = inode->block_no;
    inode->inode_free = true;
    inode->file_size = 0;
    inode->block_no = NULL;
    inode->id = NULL;


    free_blocks[blk_no] = FREE;
    disk->write(blk_no,NULL);
    return true;

}

int FileSystem::GetFreeBlock()
{
    
    int j =0;

    while(j<free_block_count)
    {
      if (free_blocks[j]==FREE)
      {
        break;
      }
      j++;  
    }
    return j;

}
