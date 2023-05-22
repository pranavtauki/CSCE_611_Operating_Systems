/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "scheduler.H"
#include "thread.H"

bool lock;
bool test_and_set(bool *lock,bool new_lock) 
{
    bool tmp = *lock;
    *lock = new_lock;
    return tmp;
}

void lock_init(bool *lock) 
{
   *lock = false;
}

void apply_lock() 
{
   while(test_and_set(&lock,true)==true);
}

void unlock()
{
   lock = false;
}
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/
extern Scheduler* SYSTEM_SCHEDULER;
BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) 
{
  lock_init(&lock);
  block_queue_size = 0;
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/
bool BlockingDisk::is_ready()
{
  return SimpleDisk::is_ready();
}
void BlockingDisk::wait_until_ready()
{
  while(!SimpleDisk::is_ready())
  {
    #ifdef INTERRUPTS_ENABLED
      BlockingDisk::push(Thread::CurrentThread());
    #else
      SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
    #endif
      SYSTEM_SCHEDULER->yield();
  }
}
void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) 
{
apply_lock();
SimpleDisk::read(_block_no, _buf);
unlock();
}
void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) 
{
  apply_lock();
  SimpleDisk::write(_block_no, _buf);
  unlock();
}
#ifdef INTERRUPTS_ENABLED
void BlockingDisk::push(Thread * thread)
{
  Console::puts("Putting thread in Q!\n");
  blocking_queue.enQ(thread);  
}

void BlockingDisk::handle_interrupt(REGS *_r)
{
  Console::puts("Handling Interrupt 14!\n");
  Thread *t = blocking_queue.deQ();
  SYSTEM_SCHEDULER->resume(t);
}
#endif