/*
 File: vm_pool.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "page_table.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
   base_address = _base_address;
   size = _size;
   frame_pool = _frame_pool;
   page_table = _page_table;
   vm_pool_next_ptr= NULL;
   reg_cnt= 0;
   rem_size = _size;
   
   page_table->register_pool(this);
   
   vm_region *ptr_region = (vm_region*)base_address;
   ptr_region[0].base_address= base_address;
   ptr_region[0].len= PageTable::PAGE_SIZE;
   regs = ptr_region;
   rem_size-=PageTable::PAGE_SIZE; 
   reg_cnt++; 
     
   Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    Console::puti(rem_size);
    if(_size > rem_size)
    {
    Console::puts("VMPOOL: No enough region space \n");
    assert(false);
    }
    
    unsigned long num_pages = (_size /PageTable::PAGE_SIZE) + (( _size %PageTable::PAGE_SIZE) > 0 ? 1 : 0);
    regs[reg_cnt].base_address = regs[reg_cnt-1].base_address +  regs[reg_cnt-1].len;
    regs[reg_cnt].len = num_pages*PageTable::PAGE_SIZE;
    reg_cnt++;
    rem_size-=num_pages*PageTable::PAGE_SIZE;
 
    Console::puts("Allocated region of memory.\n");
    return regs[reg_cnt-1].base_address;
}

void VMPool::release(unsigned long _start_address) {
    int region = -1;
    for(int  i=1;i<reg_cnt;i++)
    {
        if(regs[i].base_address  == _start_address)
           region =i;   
    }    

/*To free all the page entries */   
    unsigned long num_pages = regs[region].len / PageTable::PAGE_SIZE ;
     while(num_pages >0)
     {
        page_table->free_page(_start_address);
        num_pages--;
        _start_address+=PageTable::PAGE_SIZE;
     }
     
/*Removing the region contents in the region array, to use them later for new region allocation*/
    for(int i = region; i<reg_cnt;i++ )
            regs[i]=regs[i+1];    
    reg_cnt--;
    rem_size+=regs[region].len;
    
    Console::puts("Released region of memory.\n");  
}

bool VMPool::is_legitimate(unsigned long _address) {
   if((_address > (base_address + size)) || (_address <  base_address))
     return false;
    return true;
}

