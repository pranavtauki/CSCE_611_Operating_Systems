/*
 File: ContFramePool.C
 
 Author:
 Date  : 
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

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

ContFramePool* ContFramePool::frame_pool_list;

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

ContFramePool::FrameState ContFramePool::get_state(unsigned long _frame_no) {
    unsigned int bitmap_index = _frame_no / 4;
    unsigned int shift_size = (_frame_no % 4)*2;
    unsigned char mask = 0x03 << shift_size;
    
    switch((bitmap[bitmap_index] & mask)>>shift_size) {
        case 0x0: return FrameState::Used; 
        case 0x2: return FrameState::HoS;
        case 0x3: return FrameState::Free;
    }
    
}

void ContFramePool::set_state(unsigned long _frame_no, FrameState _state) {
    unsigned int bitmap_index = _frame_no / 4;
    unsigned char mask = 0x03 << (_frame_no % 4)*2;
    unsigned char head_mask = 0x02 <<(_frame_no % 4)*2;
    

    switch(_state) {
      case FrameState::Used:
      bitmap[bitmap_index] ^= mask;
      break;
    case FrameState::Free:
      bitmap[bitmap_index] |= mask;
      break;
    case FrameState::HoS:
      bitmap[bitmap_index] &= head_mask;
      break;
    }
    
}

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no)
{
    assert(_n_frames <= FRAME_SIZE * 4);
    
    base_frame_no = _base_frame_no;
    nframes = _n_frames;
    nFreeFrames = _n_frames;
    info_frame_no = _info_frame_no;
    
    // If _info_frame_no is zero then we keep management info in the first
    //frame, else we use the provided frame to keep management info
    if(info_frame_no == 0) {
        bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
    } else {
        bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
    }
    
    assert ((nframes % 4 ) == 0);
    // Everything ok. Proceed to mark all frame as free.
    for(int fno = 0; fno < _n_frames; fno++) {
        set_state(fno, FrameState::Free); 
    }
    
    // Mark the first frame as being used if it is being used
    if(_info_frame_no == 0) {
        set_state(0, FrameState::Used);
        nFreeFrames--;
    }
    if (ContFramePool::frame_pool_list==NULL)
        ContFramePool::frame_pool_list=this;
    else
        frame_pool_list->next = this;
    
    
    Console::puts("ContframePool::Constructor implemented!\n");
    
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    assert(nFreeFrames > 0);

    unsigned int needed_frames = _n_frames;
    unsigned int frame_no = 0;
    int found = 0;
    

    if(_n_frames > nFreeFrames) {
        Console::puts("These many frames not available");
        Console::puts("nFreeFrames = "); Console::puti(nFreeFrames);Console::puts("\n");
        Console::puts("_n_frames = "); Console::puti(_n_frames);Console::puts("\n");
    }

    for (unsigned int i = 0; i<nframes; i++) {
        if(get_state(i)==FrameState::Free){
         if (needed_frames==_n_frames)
            frame_no = i;
            needed_frames--;
         if(needed_frames==0)
            { 
                found =1;
                break;
            }

         }
         else{
            needed_frames = _n_frames;
         }

    }
    set_state(frame_no,FrameState::HoS);       //Setting first  frame as HoS
    for(int i = frame_no+1;i<_n_frames;i++)    //Setting all other frames as Used 
        set_state(i,FrameState::Used);
    return(frame_no+base_frame_no);            // Returning the absolute address
    
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
    for(int fno = _base_frame_no; fno < _base_frame_no + _n_frames; fno++){

        set_state(fno - base_frame_no, FrameState::HoS);
    }
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    unsigned long frame_no = _first_frame_no;
    
    ContFramePool *curr = ContFramePool::frame_pool_list;
    while(curr!=NULL)
    {
        int start = curr->base_frame_no;
        int end = curr->base_frame_no + curr->nframes;

        if(_first_frame_no>=start&&_first_frame_no<end)    // Check if the frame is in the given pool
            {//Console::puts("found the pool");
              break;}
    curr = curr->next;
    }
    
    if(curr->get_state(_first_frame_no-curr->base_frame_no)!=FrameState::HoS)  // Check if the first frame is the head of sequence 
        Console::puts("Wrong free since not head");
    unsigned long i = frame_no-curr->base_frame_no;
    curr->set_state(i,FrameState::Free);                                       //Setting head frame as free
    i++;
    while(curr->get_state(i)!=FrameState::HoS && i < curr->nframes)
    {
        curr->set_state(i,FrameState::Free);                                   // Releasing all other fraames
        i++;

    }

}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    return _n_frames / (16*1024) + (_n_frames % (16*1024) > 0 ? 1 : 0);
}
