#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

#define PAGE_DIRECTORY_FRAME_SIZE 1
#define PAGE_LEVEL          4

#define PD_SHIFT            22
#define PT_SHIFT            12

#define PDE_MASK            0xFFFFF000
#define PT_MASK             0x3FF


PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   PageTable::kernel_mem_pool = _kernel_mem_pool;
   PageTable::process_mem_pool = _process_mem_pool;
   PageTable::shared_size = _shared_size;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
   page_directory = (unsigned long *)(kernel_mem_pool->get_frames(PAGE_DIRECTORY_FRAME_SIZE)*PAGE_SIZE);

   unsigned long * direct_map_page_table = (unsigned long *)(kernel_mem_pool->get_frames(PAGE_DIRECTORY_FRAME_SIZE)*PAGE_SIZE);

   unsigned long address = 0;//hold where the physical address of the page 
   for(int i = 0;i<1024;i++)
   {
      direct_map_page_table[i] = address | 3; //mapping first 4MB of memory as supervisor level,read/write, Present 
      address = address + 4096; //4kB = 4096  
   }

   page_directory[0] = (unsigned long)direct_map_page_table;
   page_directory[0] = (unsigned long)direct_map_page_table | 3;

   for(int i =1;i<1024;i++)
      page_directory[i] = 0 | 2;
   
   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   //assert(false);
   current_page_table = this;
   write_cr3((unsigned long)page_directory);
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   //assert(false);
   paging_enabled = 1;
   write_cr0(read_cr0() | 0x80000000);
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
  // read the base page directory address
    unsigned long * cur_pg_dir = (unsigned long *) read_cr3();

    // read the page address where fault occurred
    unsigned long page_addr = read_cr2();

    unsigned long PD_address   = page_addr >> PD_SHIFT;
    unsigned long PT_address   = page_addr >> PT_SHIFT;

    unsigned long * page_table = NULL;
    unsigned long error_code = _r->err_code;

    unsigned long mask_addr = 0;

    /*
     * --10bits for PD-- --10bits for PT-- --12bit offset--
     * 0000 0000 00       0000 0000 00      00 0000 0000
     */

    if ((error_code & 1) == 0 ) {
        if ((cur_pg_dir[PD_address] & 1 ) == 1) {  //fault in Page table
            page_table = (unsigned long *)(cur_pg_dir[PD_address] & PDE_MASK);
            page_table[PT_address & PT_MASK] = (PageTable::process_mem_pool->get_frames(PAGE_DIRECTORY_FRAME_SIZE)*PAGE_SIZE) | 3 ;

        } else {
            cur_pg_dir[PD_address] = (unsigned long) ((kernel_mem_pool->get_frames(PAGE_DIRECTORY_FRAME_SIZE)*PAGE_SIZE) | 3);

            page_table = (unsigned long *)(cur_pg_dir[PD_address] & PDE_MASK);

            for (int i = 0; i<1024; i++) {
                page_table[i] = mask_addr | PAGE_LEVEL ; // set the pages as user page
            }

            page_table[PT_address & PT_MASK] = (PageTable::process_mem_pool->get_frames(PAGE_DIRECTORY_FRAME_SIZE)*PAGE_SIZE) | 3 ;

        }
    }


  Console::puts("handled page fault\n");
}

