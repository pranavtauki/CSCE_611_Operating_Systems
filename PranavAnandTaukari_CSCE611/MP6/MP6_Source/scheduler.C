/*
 File: scheduler.C
 
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

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "machine.H"
//#include "stdio.h"

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
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
  
  Qsize = 0; // initializing size of ready queue  
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  //perform deque from ready queue and call dispatch to function to give the control
  //if (Machine::interrupts_enabled())  
   //Machine::disable_interrupts();
  if(Qsize)
  {
  Thread *topThread = readyQ.deQ();
  Qsize--;
   //if (!Machine::interrupts_enabled())  
   //Machine::enable_interrupts();
  Thread::dispatch_to(topThread);
  }
  else 
    Console::puts("Queue is empty");
}

void Scheduler::resume(Thread * _thread) {
  //perform enqueue of the thread 
    //if (Machine::interrupts_enabled()) 
    //Machine::disable_interrupts();
   readyQ.enQ(_thread);
   Qsize++;
    //if (!Machine::interrupts_enabled()) 
     //Machine::enable_interrupts();
}

void Scheduler::add(Thread * _thread) {
  resume(_thread);
}

void Scheduler::terminate(Thread * _thread) {
  int i = 0;
  bool found = false;
   //if (Machine::interrupts_enabled())  
   //Machine::disable_interrupts();
  while(i<Qsize)
  {
    Thread *temp = readyQ.deQ();
    if(temp->ThreadId()==_thread->ThreadId())
      {found = true;delete (void*) temp;break;}
    else 
      readyQ.enQ(temp);
  i++;
  }
if(found)
  Qsize--;
 //if (!Machine::interrupts_enabled())  
   //Machine::enable_interrupts();
yield();
}


