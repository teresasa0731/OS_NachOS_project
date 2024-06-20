// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"
#include "list.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------




//<TODO_Teresa>
// Declare sorting rule of SortedList for L1 & L2 ReadyQueue
// Hint: Funtion Type should be "static int"
//<TODO_Teresa>

// Function 1. Function definition of sorting rule of L1 ReadyQueue
static int RemainingTimecmp(Thread* t1,Thread* t2){
    return t1->getRemainingBurstTime() - t2->getRemainingBurstTime(); // preemptive SRTN
}

// Function 2. Function definition of sorting rule of L2 ReadyQueue
static int PIDcmp(Thread* t1,Thread* t2){
    return t1->getID() - t2->getID(); // smaller pid do first 
}

Scheduler::Scheduler()
{
    //<TODO_Teresa>
    // Initialize L1, L2, L3 ReadyQueue
    //<TODO_Teresa>
    L1ReadyQueue = new SortedList<Thread *>(RemainingTimecmp);
    L2ReadyQueue = new SortedList<Thread *>(PIDcmp);
    L3ReadyQueue = new List<Thread *>; 
	toBeDestroyed = NULL;
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    //<TODO_Teresa>
    // Remove L1, L2, L3 ReadyQueue
    //<TODO_Teresa>
    // delete readyList; 
    delete L1ReadyQueue; 
    delete L2ReadyQueue; 
    delete L3ReadyQueue; 
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread)
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());

    Statistics* stats = kernel->stats;
    thread->setStatus(READY);
    //<TODO_Teresa>
    // According to priority of Thread, put them into corresponding ReadyQueue.
    // After inserting Thread into ReadyQueue, don't forget to reset some values.
    // Hint: L1 ReadyQueue is preemptive SRTN(Shortest Remaining Time Next).
    // When putting a new thread into L1 ReadyQueue, you need to check whether preemption or not.
    //<TODO_Teresa>
    // L1 queue: priority between 100 - 149
    if (thread->getPriority() < 150 && thread->getPriority() >= 100){ 
        DEBUG('z' , "[InsertToQueue] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[1]");
        L1ReadyQueue->Insert(thread);
    }
    // L2 queue: priority between 50 - 99
    else if (thread->getPriority() < 100 && thread->getPriority() >= 50) {  
        DEBUG('z' , "[InsertToQueue] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[2]");
        L2ReadyQueue->Insert(thread);
    }
    //L3 queue: priority between 0 - 49
    else if (thread->getPriority() < 50) {   
        DEBUG('z' , "[InsertToQueue] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[3]");
        L3ReadyQueue->Append(thread);
    }
}
//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{
    Thread *thread = NULL;
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    //<TODO_Teresa>
    // a.k.a. Find Next (Thread in ReadyQueue) to Run
    //<TODO_Teresa>
    if(!L1ReadyQueue->IsEmpty()){
        thread = L1ReadyQueue->RemoveFront();
        thread->setWaitTime(0);
        DEBUG('z', "[RemoveFromQueue] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is remove from queue L[1]");
    }
    else if(!L2ReadyQueue->IsEmpty()){
        thread = L2ReadyQueue->RemoveFront();
        thread->setWaitTime(0);
        DEBUG('z', "[RemoveFromQueue] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is remove from queue L[2]");
    }
    else if(!L3ReadyQueue->IsEmpty()){
        thread = L3ReadyQueue->RemoveFront();
        thread->setWaitTime(0);
        DEBUG('z', "[RemoveFromQueue] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is remove from queue L[3]");
    }
    else{
        thread = NULL;
    }
    return thread;
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread, bool finishing)
{
    Thread *oldThread = kernel->currentThread;
 
	// cout << "Current Thread" <<oldThread->getName() << "    Next Thread"<<nextThread->getName()<<endl;
   
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing) {	// mark that we need to delete current thread
         ASSERT(toBeDestroyed == NULL);
	     toBeDestroyed = oldThread;
    }
   
#ifdef USER_PROGRAM			// ignore until running user programs 
    if (oldThread->space != NULL) {	// if this thread is a user program,

        oldThread->SaveUserState(); 	// save the user's CPU registers
	    oldThread->space->SaveState();
    }
#endif
    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running
    
    // DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    // cout << "Switching from: " << oldThread->getID() << " to: " << nextThread->getID() << endl;
    SWITCH(oldThread, nextThread);

    // we're back, running oldThread
      
    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << kernel->currentThread->getID());

    CheckToBeDestroyed();		// check if thread we were running
					// before this one has finished
					// and needs to be cleaned up
    
#ifdef USER_PROGRAM
    if (oldThread->space != NULL) {	    // if there is an address space
        oldThread->RestoreUserState();     // to restore, do it.
	    oldThread->space->RestoreState();
    }
#endif
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL) {
        DEBUG(dbgThread, "toBeDestroyed->getID(): " << toBeDestroyed->getID());
        delete toBeDestroyed;
	    toBeDestroyed = NULL;
    }
}
 
//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::Print()
{
    cout << "Ready list contents:\n";
    // readyList->Apply(ThreadPrint);
    L1ReadyQueue->Apply(ThreadPrint);
    L2ReadyQueue->Apply(ThreadPrint);
    L3ReadyQueue->Apply(ThreadPrint);
}

// <TODO_Teresa>
// Function 3. Scheduler::UpdatePriority()
// Hint:
// 1. ListIterator can help.
// 2. Update WaitTime and priority in Aging situations
// 3. After aging, Thread may insert to different ReadyQueue
// <TODO_Teresa>
void Scheduler::UpdatePriority()
{
    Aging(L1ReadyQueue);
    Aging(L2ReadyQueue);
    Aging(L3ReadyQueue);
}

void Scheduler::Aging(List<Thread *> *list)
{
    ListIterator<Thread *> *iter = new ListIterator<Thread *>((List<Thread *> *)list);

    for(;iter->IsDone() != true; iter->Next()){
        Thread *iterThread = iter->Item();
        // waiting time update
        int oldWaitingTime = iterThread->getWaitTime();
        iterThread->setWaitTime(oldWaitingTime + 100);
        int oldPriority = iterThread->getPriority();
        // aging detection
        if((oldPriority >= 0 && oldPriority < 150) && oldWaitingTime >= 400) {
            iterThread->setWaitTime(0) ;
            iterThread->setPriority((oldPriority + 10 > 149) ? 149 : oldPriority + 10);
            DEBUG('z' ,"[UpdatePriority] Tick [" << kernel->stats->totalTicks << "]: Thread [" <<  iterThread->getID()  << "] changes its priority from[" << oldPriority << "] to [" << iterThread-> getPriority() <<"]");
            list->Remove(iterThread);
            ReadyToRun(iterThread);
            // // aging thread put into L1 ready queue
            // if(iterThread->getPriority() >= 100){
            //     L1ReadyQueue->Insert(iterThread);
            //     if(list != L1ReadyQueue){
            //         DEBUG('z',"[RemoveFromQueue] Tick [" << kernel->stats->totalTicks << "]: Thread [" << iterThread->getID()  << "] is removed from queue L[2]");
            //         DEBUG('z' ,"[InsertToQueue] Tick [" << kernel->stats->totalTicks << "]: Thread [" << iterThread->getID()  << "] is inserted into queue L[1]");
            //     }
            //     // preemption condition
            //     // 1. current running Thread in L2 or L3
            //     // 2. current running Thread in L1 & Remaining Burst Time is greater
            //     if(kernel->currentThread->getPriority() < 100){  
            //         kernel->alarm->preemptive = true;
            //     }else if(kernel->currentThread->getPriority() >= 100 && kernel->currentThread->getRemainingBurstTime() > iterThread->getRemainingBurstTime()){
            //         kernel->alarm->preemptive = true;
            //     }
            // }
            // // aging thread put into L2 ready queue
            // else if(iterThread->getPriority() >= 50){
            //     L2ReadyQueue->Insert(iterThread);
            //     if(list != L2ReadyQueue){
            //         DEBUG('z', "[RemoveFromQueue] Tick [" << kernel->stats->totalTicks << "]: Thread [" << iterThread->getID()  << "] is removed from queue L[3]");
            //         DEBUG('z', "[InsertToQueue] Tick [" << kernel->stats->totalTicks << "]: Thread [" << iterThread->getID()  << "] is inserted into queue L[2]");
            //     }
            //     // preemption condition
            //     // 1. currentThread in L3
            //     if(kernel->currentThread->getPriority() < 50){ 
            //         kernel->alarm->preemptive = true;
            //     }
            // // aging thread put back into L3 ready queue
            // }else{
            //     L3ReadyQueue->Append(iterThread);
            // }
        }   // end of aging operation
    }   // end of for-loop
}