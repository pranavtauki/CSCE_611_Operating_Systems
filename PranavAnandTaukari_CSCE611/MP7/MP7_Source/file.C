/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id) {
    
    cur_pos = 0;
    
    fs = _fs;
    
    int i =0;
    bool block_found = false;
    while(i<fs->MAX_INODES)
    {
        if (fs->inodes[i].id==_id)
        {
            Console::puts("Found inode.\n");
            inode_index = i;
            file_size = fs->inodes[i].file_size;
            block_found=true;
            block_no = fs->inodes[i].block_no;
            fs->disk->read(fs->inodes[i].block_no,block_cache);
        }
        i++;
    }
    if(block_found==false)
    {
        assert(false);
    }
}

File::~File() {
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
    fs->disk->write(fs->inodes[inode_index].block_no,block_cache);
    fs->inodes[inode_index].file_size = file_size;
    
    Inode * tmp_inodes = fs->inodes;
    unsigned char * tmp = (unsigned char *)tmp_inodes;
    fs->disk->write(1,tmp); 
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");
    int char_read = 0;
    int i = cur_pos;
    while(i<file_size)
    {
      if (char_read==_n){
            break;
        }
        _buf[char_read]=block_cache[i];
        char_read++;
        i++;
    }
    return char_read;
    
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
    int j = 0;
    int st = cur_pos;
    for (int i=st;i<st+_n;i++){
        if (i==SimpleDisk::BLOCK_SIZE)
        {
            break;
        }

        block_cache[i] = _buf[j];
        j++;
        cur_pos++;
        file_size++;

    }
    return j;
}

void File::Reset() {
    Console::puts("resetting file\n");
    cur_pos = 0;
}

bool File::EoF() {
    Console::puts("checking for EoF\n");
    if (cur_pos<file_size){
        return false;
    }
    return true;
}
