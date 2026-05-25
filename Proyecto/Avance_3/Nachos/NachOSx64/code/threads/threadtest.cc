// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create several threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustrate the inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include <unistd.h>
#include "copyright.h"
#include "system.h"
#include "diningph.h"
#include <iostream>

// DiningPh * dp;

// void Philo( void * p ) {

//     int eats, thinks;
//     long who = (long) p;

//     currentThread->Yield();

//     for ( int i = 0; i < 3; i++ ) {

//         printf(" Philosopher %ld will try to pickup sticks\n", who + 1);

//         dp->pickup( who );
//         dp->print();
//         eats = Random() % 6;

//         currentThread->Yield();
//         sleep( eats );

//         dp->putdown( who );

//         thinks = Random() % 6;
//         currentThread->Yield();
//         sleep( thinks );
//     }

// }

Condition *HQueue;
Condition *OQueue;
Lock* lock;
int cO = 0;
int cH = 0;

void H(int) {
    lock->Acquire();
    cH++;
    while (!(cH >= 2 && cO >= 1)) {
        HQueue->Wait(lock);
    }
    if (cH >= 2 && cO >= 1) {
        cH -= 2;
        cO -= 1;
        HQueue->Signal(lock);
        HQueue->Signal(lock);
        OQueue->Signal(lock);
        printf("Se ha producido una molecula de agua\n");
    }
    lock->Release();
}

void O(int) {
    lock->Acquire();
    cO++;
    while (!(cH >= 2)) {
        OQueue->Wait(lock);
    }
    if (cH >= 2 && cO >= 1) {
        cH -= 2;
        cO -= 1;
        HQueue->Signal(lock);
        HQueue->Signal(lock);
        OQueue->Signal(lock);
        printf("Se ha producido una molecula de agua\n");
    }
    lock->Release();
}

void HThread(void* arg) {
    H((long)arg);
}

void OThread(void* arg) {
    O((long)arg);
}

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 10 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"name" points to a string with a thread name, just for
//      debugging purposes.
//----------------------------------------------------------------------

// void SimpleThread(void* name) {
//     // Reinterpret arg "name" as a string
//     char* threadName = (char*)name;
    
//     // If the lines dealing with interrupts are commented,
//     // the code will behave incorrectly, because
//     // printf execution may cause race conditions.
//     for (int num = 0; num < 10; num++) {
//         //IntStatus oldLevel = interrupt->SetLevel(IntOff);
// 	printf("*** thread %s looped %d times\n", threadName, num);
// 	//interrupt->SetLevel(oldLevel);
//         //currentThread->Yield();
//     }
//     //IntStatus oldLevel = interrupt->SetLevel(IntOff);
//     printf(">>> Thread %s has finished\n", threadName);
//     //interrupt->SetLevel(oldLevel);
// }



//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between several threads, by launching
//	ten threads which call SimpleThread, and finally calling 
//	SimpleThread ourselves.
//----------------------------------------------------------------------

void ThreadTest() {
    // Thread * Ph;

    // DEBUG('t', "Entering SimpleTest");


    // dp = new DiningPh();

    // for ( long k = 0; k < 5; k++ ) {
    //     Ph = new Thread( "dp" );
    //     Ph->Fork( Philo, (void *) k );
    // }

    // return;

    // for ( int k=1; k<5; k++) {
    //   char* threadname = new char[100];
    //   sprintf(threadname, "Hilo %d", k);
    //   Thread* newThread = new Thread (threadname);
    //   newThread->Fork (SimpleThread, (void*)threadname);
    // }
    
    // SimpleThread( (void*)"Hilo 0");
    lock = new Lock("lock");
    HQueue = new Condition("HQueue");
    OQueue = new Condition("OQueue");

    int limite = 10;
    for (int i = 0; i < limite; i++) {
        Thread* t = new Thread("atom");

        if (rand() % 2 == 0) {
            printf("Se crea Hidrogeno %d\n", i);
            t->Fork(HThread, (void*)(long)i);
        } else {
            printf("Se crea Oxigeno %d\n", i);
            t->Fork(OThread, (void*)(long)i);
        }
    }
}

