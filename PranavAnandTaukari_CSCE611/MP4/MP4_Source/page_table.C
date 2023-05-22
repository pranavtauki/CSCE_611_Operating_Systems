#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

#define PAGE_DIRECTORY_FRAME_SIZE 1


PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;
VMPool  * PageTable::Head_VMPoolList = NULL;


void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
    //assert(false);
    kernel_mem_pool = _kernel_mem_pool;
    process_mem_pool = _process_mem_pool;
    shared_size = _shared_size;
    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
    //assert(false);
    page_directory =  (unsigned long *)(kernel_mem_pool->get_frames(PAGE_DIRECTORY_FRAME_SIZE) * PAGE_SIZE);
    page_directory[1023] = (unsigned long)(page_directory )| 3; 
    
   unsigned long *page_table = (unsigned long *) (process_mem_pool->get_frames(PAGE_DIRECTORY_FRAME_SIZE) * PAGE_SIZE);
   unsigned long address = 0;
   for(unsigned int i =0; i<1024; i++)
   {
       page_table[i] = address |3;  
       address += PAGE_SIZE;        
   }
   page_directory[0] = (unsigned long)page_table;  
   page_directory[0] = page_directory[0] |3;

    for(unsigned int i = 1; i<1023; i++)
   {
     page_directory[i]= 0|2;  
   }   
  
 paging_enabled = 0;    
 Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
    current_page_table = this;
    Console::putui((unsigned long)(current_page_table->page_directory[1]));
    write_cr3((unsigned long)(current_page_table->page_directory));
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    paging_enabled = 1;
   write_cr0(read_cr0() | 0x80000000);
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    unsigned long address = read_cr2();
    Console::putui(address);
    unsigned int address_present = 0;
    VMPool *vmptr = PageTable::Head_VMPoolList;
    for(;vmptr!=NULL;vmptr=vmptr->vm_pool_next_ptr)
    {
        if(vmptr->is_legitimate(address) == true)
        {
            address_present = 1;
            break;
        }
    }
    if(address_present == 0 && vmptr!= NULL)
    {
      Console::puts("INVALID ADDRESS \n");
      assert(false);        
    }
    unsigned long* page_dir_pt = current_page_table->page_directory;
    
    unsigned long page_dirc_index = PDE_address(address);
    unsigned long page_table_index = PTE_address(address);
    
    unsigned long *page_table; 
    unsigned long *page_table_entry; 
    
    if((page_dir_pt[page_dirc_index] & 1) == 0) 
    {
        
       page_table = (unsigned long *)(process_mem_pool->get_frames(1) * PAGE_SIZE);
       unsigned long *PD_entry = (unsigned long *)(0xFFFFF<<12);               
       PD_entry[page_dirc_index] = (unsigned long)(page_table)|3;
    }
    
    page_table_entry = (unsigned long *) (process_mem_pool->get_frames(1) * PAGE_SIZE);
    unsigned long *page_entry = (unsigned long *)((0x3FF<< 22)| (page_dirc_index <<12)); 
    page_entry[page_table_index] = (unsigned long)(page_table_entry)|3;
            
    Console::puts("handled page fault\n");

}

void PageTable::register_pool(VMPool * _vm_pool)
{
   if( PageTable::Head_VMPoolList == NULL ) 
    {
        PageTable::Head_VMPoolList= _vm_pool;
    }
    else 
    {
        VMPool *vmptr = PageTable::Head_VMPoolList;
        for(;vmptr->vm_pool_next_ptr!=NULL;vmptr= vmptr->vm_pool_next_ptr);
        vmptr->vm_pool_next_ptr= _vm_pool;
    }
    
    Console::puts("registered VM pool\n");  
}

void PageTable::free_page(unsigned long _page_no) {
    unsigned long page_dirc_index =  ( _page_no & 0xFFC)>> 22; 
    unsigned long page_table_index = (_page_no & 0x003FF ) >>12 ;

    unsigned long *page_table_entry= (unsigned long *) ( (0x000003FF << 22) | (page_dirc_index << 12) );
    unsigned long frame_number = (page_table_entry[page_table_index] & 0xFFFFF000)/ PAGE_SIZE;
    
    process_mem_pool->release_frames(frame_number);
    page_table_entry[page_table_index] |= 2; 
    Console::puts("freed page\n");
    
    /*Flushing TLB*/
    load();
}

unsigned long PageTable::PDE_address(unsigned long addr)
{ 
    return (addr >>22);    
}
unsigned long PageTable::PTE_address(unsigned long addr)
{
    return (addr & (0x03FF << 12) ) >>12;
}