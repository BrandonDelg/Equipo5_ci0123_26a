// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//
// Copyright (c) -2025 Universidad de Costa Rica


#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synch.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SC_NachOS	12345
#define MAX_FILENAME 256
#define MAX_PROCESSES 128


#define MAX_SEMAPHORES 128
#define MAX_LOCKS 128
#define MAX_CONDITIONS 128

Semaphore* semTable[MAX_SEMAPHORES];
Lock* lockTable[MAX_LOCKS];
Condition* condTable[MAX_CONDITIONS];

struct ProcessEntry {
   bool used;
   int exitStatus;
   Semaphore *joinSem;
   Thread *thread;
};

struct ExecInfo {
   char *filename;
   int pid;
};

static ProcessEntry processTable[MAX_PROCESSES];

bool ReadStringFromUser(int userAddr, char *buffer, int maxSize) {
   int value;

   for (int i = 0; i < maxSize - 1; i++) {
      if (!machine->ReadMem(userAddr + i, 1, &value)) {
         return false;
      }

      buffer[i] = (char)value;

      if (buffer[i] == '\0') {
         return true;
      }
   }

   buffer[maxSize - 1] = '\0';
   return true;
}

int AllocProcess() {
   for (int i = 1; i < MAX_PROCESSES; i++) {
      if (!processTable[i].used) {
         processTable[i].used = true;
         processTable[i].exitStatus = 0;
         processTable[i].joinSem = new Semaphore("join semaphore", 0);
         processTable[i].thread = NULL;
         return i;
      }
   }

   return -1;
}

void updatePc() {
   int prev =  machine->ReadRegister(PCReg);
   int pc = machine->ReadRegister(NextPCReg);
   machine->WriteRegister(PrevPCReg, prev);
   machine->WriteRegister(PCReg, pc);
   machine->WriteRegister(NextPCReg,pc+4);
}

/*
 *  System call interface: Halt()
 */
void NachOS_Halt() {		// System call 0

	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();

}


/*
 *  System call interface: void Exit( int )
 */
// void NachOS_Exit()
// {
//    int status = machine->ReadRegister(4);
//    DEBUG('u', "Exit system call. Status %d\n", status);

//    delete currentThread->space;
//    currentThread->space = NULL;

//    currentThread->Finish();
// }
void NachOS_Exit()
{
   int status = machine->ReadRegister(4);

   DEBUG('u', "Exit system call. Status %d\n", status);

   for (int i = 1; i < MAX_PROCESSES; i++) {
      if (processTable[i].used && processTable[i].thread == currentThread) {
         processTable[i].exitStatus = status;
         processTable[i].joinSem->V();
         break;
      }
   }

   delete currentThread->space;
   currentThread->space = NULL;

   currentThread->Finish();
}
/*
 *  System call interface: SpaceId Exec( char * )
 */
void NachosExecThread(void *arg) {
   ExecInfo *info = (ExecInfo *)arg;

   OpenFile *executable = fileSystem->Open(info->filename);

   if (executable == NULL) {
      processTable[info->pid].exitStatus = -1;
      processTable[info->pid].joinSem->V();

      delete [] info->filename;
      delete info;

      currentThread->Finish();
      return;
   }

   processTable[info->pid].thread = currentThread;

   currentThread->space = new AddrSpace(executable);

   if (!currentThread->space->IsValid()) {
      processTable[info->pid].exitStatus = -1;
      processTable[info->pid].joinSem->V();

      delete currentThread->space;
      currentThread->space = NULL;

      delete executable;
      delete [] info->filename;
      delete info;

      currentThread->Finish();
      return;
   }

   delete executable;
   delete [] info->filename;
   delete info;

   currentThread->space->InitRegisters();
   currentThread->space->RestoreState();

   machine->Run();

   ASSERT(false);
}

void NachOS_Exec()
{
   int nameAddr = machine->ReadRegister(4);
   char filename[MAX_FILENAME];

   if (!ReadStringFromUser(nameAddr, filename, MAX_FILENAME)) {
      machine->WriteRegister(2, -1);
      return;
   }

   int pid = AllocProcess();

   if (pid < 0) {
      machine->WriteRegister(2, -1);
      return;
   }

   ExecInfo *info = new ExecInfo;
   info->pid = pid;
   info->filename = new char[strlen(filename) + 1];
   strcpy(info->filename, filename);

   Thread *newT = new Thread("exec process");
   processTable[pid].thread = newT;

   newT->Fork(NachosExecThread, info);

   machine->WriteRegister(2, pid);
}

void NachOS_Join() {
   int pid = machine->ReadRegister(4);

   if (pid <= 0 || pid >= MAX_PROCESSES || !processTable[pid].used) {
      machine->WriteRegister(2, -1);
      return;
   }

   processTable[pid].joinSem->P();

   int status = processTable[pid].exitStatus;

   delete processTable[pid].joinSem;
   processTable[pid].joinSem = NULL;
   processTable[pid].thread = NULL;
   processTable[pid].used = false;

   machine->WriteRegister(2, status);
}

void NachOS_Create() {
   int nameAddr = machine->ReadRegister(4);
   char filename[MAX_FILENAME];

   if (!ReadStringFromUser(nameAddr, filename, MAX_FILENAME)) {
      machine->WriteRegister(2, -1);
      return;
   }

   int fd = creat(filename, 0644);

   if (fd < 0) {
      machine->WriteRegister(2, -1);
      return;
   }

   close(fd);
   machine->WriteRegister(2, 0);
}

void NachOS_Open() {
   int nameAddr = machine->ReadRegister(4);
   char filename[MAX_FILENAME];

   if (!ReadStringFromUser(nameAddr, filename, MAX_FILENAME)) {
      machine->WriteRegister(2, -1);
      return;
   }

   int unixFd = open(filename, O_RDWR);

   if (unixFd < 0) {
      machine->WriteRegister(2, -1);
      return;
   }

   int nachosFd = currentThread->space->openFilesTable->Open(unixFd);

   machine->WriteRegister(2, nachosFd);
}
/*
 *  System call interface: OpenFileId Write( char *, int, OpenFileId )
 */
// void NachOS_Write()
// {
//    int addr = machine->ReadRegister(4);
//    int size = machine->ReadRegister(5);
//    int file = machine->ReadRegister(6);

//    int car;

//    DEBUG('u', "Write system call\n");
//    for (int i = 0; i < size; i++) {
//       if (!machine->ReadMem(addr + i, 1, &car)) {
//          return;
//       }
//       printf("%c", car);
//    }
// }
void NachOS_Write()
{
   int addr = machine->ReadRegister(4);
   int size = machine->ReadRegister(5);
   int file = machine->ReadRegister(6);

   char buffer[512];
   int car;

   if (size > 512) size = 512;

   for (int i = 0; i < size; i++) {
      machine->ReadMem(addr + i, 1, &car);
      buffer[i] = (char) car;
   }

   int result;

   if (file == 1 || file == 2) {
      result = fwrite(buffer, 1, size, stdout);
      fflush(stdout);
   } else {
      int unixFd =
      currentThread->space->openFilesTable->getUnixHandle(file);

      if (unixFd < 0) {
         machine->WriteRegister(2, -1);
         return;
      }
      result = write(unixFd, buffer, size);   }

   machine->WriteRegister(2, result);
}
/*
 *  System call interface: OpenFileId Read( char *, int, OpenFileId )
 */
void NachOS_Read()
{
   int addr = machine->ReadRegister(4);
   int size = machine->ReadRegister(5);
   int file = machine->ReadRegister(6);

   char buffer[512];

   if (size > 512) size = 512;

   int result = 0;

   if (file == 0) {
      int i;
      for (i = 0; i < size - 1; i++) {
         int c = getchar();
         if (c == EOF) break;

         buffer[i] = (char)c;

         if (c == '\n') {
            i++;
            break;
         }
      }

      result = i;
   } else {
      int unixFd =
      currentThread->space->openFilesTable->getUnixHandle(file);

      if (unixFd < 0) {
         machine->WriteRegister(2, -1);
         return;
      }

      result = read(unixFd, buffer, size);   }

   if (result > 0) {
      for (int i = 0; i < result; i++) {
         machine->WriteMem(addr + i, 1, buffer[i]);
      }
   }

   machine->WriteRegister(2, result);
}

/*
 *  System call interface: void Close( OpenFileId )
 */
void NachOS_Close()
{
   int nachosFd = machine->ReadRegister(4);

   if (nachosFd <= 2) {
      machine->WriteRegister(2, -1);
      return;
   }

   int unixFd =
      currentThread->space->openFilesTable
         ->Close(nachosFd);

   if (unixFd < 0) {
      machine->WriteRegister(2, -1);
      return;
   }

   int result = close(unixFd);

   machine->WriteRegister(2, result);
}

void NachosForkThread(void *p)
{
   AddrSpace *space = currentThread->space;

   space->InitRegisters();
   space->RestoreState();

   machine->WriteRegister(RetAddrReg, 4);

   machine->WriteRegister(PCReg, (long)p);
   machine->WriteRegister(NextPCReg, (long)p + 4);

   machine->Run();

   ASSERT(false);
}
/*
 *  System call interface: void Fork( void (*func)() )
 */
void NachOS_Fork() {
   DEBUG('u', "Fork system call\n");

   int funcAddr = machine->ReadRegister(4);

   Thread *newT = new Thread("user fork thread");

   newT->space = new AddrSpace(currentThread->space);

   newT->Fork(NachosForkThread, (void *)(long)funcAddr);
   updatePc();
}

/*
 *  System call interface: void Yield()
 */
void NachOS_Yield() {
   currentThread->Yield();
}
/*
 *  System call interface: Sem_t SemCreate( int )
 */
void NachOS_SemCreate()
{
   int value = machine->ReadRegister(4);

   for (int i = 0; i < MAX_SEMAPHORES; i++) {

      if (semTable[i] == NULL) {

         semTable[i] = new Semaphore("user semaphore", value);

         machine->WriteRegister(2, i);
         return;
      }
   }

   machine->WriteRegister(2, -1);
}

/*
 *  System call interface: int SemDestroy( Sem_t )
 */
void NachOS_SemDestroy()
{
   int id = machine->ReadRegister(4);

   if (id < 0 || id >= MAX_SEMAPHORES ||
       semTable[id] == NULL) {

      machine->WriteRegister(2, -1);
      return;
   }

   semTable[id]->Destroy();

   delete semTable[id];
   semTable[id] = NULL;

   machine->WriteRegister(2, 0);
}


/*
 *  System call interface: int SemSignal( Sem_t )
 */
void NachOS_SemSignal()
{
   int id = machine->ReadRegister(4);

   if (id < 0 || id >= MAX_SEMAPHORES ||
       semTable[id] == NULL) {

      machine->WriteRegister(2, -1);
      return;
   }

   semTable[id]->V();

   machine->WriteRegister(2, 0);
}

/*
 *  System call interface: int SemWait( Sem_t )
 */
void NachOS_SemWait()
{
   int id = machine->ReadRegister(4);

   if (id < 0 || id >= MAX_SEMAPHORES ||
       semTable[id] == NULL) {

      machine->WriteRegister(2, -1);
      return;
   }

   semTable[id]->P();

   machine->WriteRegister(2, 0);
}

/*
 *  System call interface: Lock_t LockCreate( int )
 */
void NachOS_LockCreate()
{
   for (int i = 0; i < MAX_LOCKS; i++) {

      if (lockTable[i] == NULL) {

         lockTable[i] =
            new Lock("user lock");

         machine->WriteRegister(2, i);
         return;
      }
   }

   machine->WriteRegister(2, -1);
}

/*
 *  System call interface: int LockDestroy( Lock_t )
 */
void NachOS_LockDestroy()
{
   int id = machine->ReadRegister(4);

   if (id < 0 || id >= MAX_LOCKS ||
       lockTable[id] == NULL) {

      machine->WriteRegister(2, -1);
      return;
   }

   delete lockTable[id];
   lockTable[id] = NULL;

   machine->WriteRegister(2, 0);
}

/*
 *  System call interface: int LockAcquire( Lock_t )
 */
void NachOS_LockAcquire()
{
   int id = machine->ReadRegister(4);

   if (id < 0 || id >= MAX_LOCKS ||
       lockTable[id] == NULL) {

      machine->WriteRegister(2, -1);
      return;
   }

   lockTable[id]->Acquire();

   machine->WriteRegister(2, 0);
}

/*
 *  System call interface: int LockRelease( Lock_t )
 */
void NachOS_LockRelease()
{
   int id = machine->ReadRegister(4);

   if (id < 0 || id >= MAX_LOCKS ||
       lockTable[id] == NULL) {

      machine->WriteRegister(2, -1);
      return;
   }

   lockTable[id]->Release();

   machine->WriteRegister(2, 0);
}

/*
 *  System call interface: Cond_t LockCreate( int )
 */
void NachOS_CondCreate()
{
   for (int i = 0; i < MAX_CONDITIONS; i++) {

      if (condTable[i] == NULL) {

         condTable[i] =
            new Condition("user condition");

         machine->WriteRegister(2, i);
         return;
      }
   }

   machine->WriteRegister(2, -1);
}

/*
 *  System call interface: int CondDestroy( Cond_t )
 */
void NachOS_CondDestroy()
{
   int id = machine->ReadRegister(4);

   if (id < 0 || id >= MAX_CONDITIONS ||
       condTable[id] == NULL) {

      machine->WriteRegister(2, -1);
      return;
   }

   delete condTable[id];
   condTable[id] = NULL;

   machine->WriteRegister(2, 0);
}

/*
 *  System call interface: int CondSignal( Cond_t )
 */
void NachOS_CondSignal()
{
   int condId = machine->ReadRegister(4);
   int lockId = machine->ReadRegister(5);

   condTable[condId]->Signal(
      lockTable[lockId]);

   machine->WriteRegister(2, 0);
}

/*
 *  System call interface: int CondWait( Cond_t )
 */
void NachOS_CondWait()
{
   int condId = machine->ReadRegister(4);
   int lockId = machine->ReadRegister(5);

   if (condId < 0 || condId >= MAX_CONDITIONS ||
       lockId < 0 || lockId >= MAX_LOCKS ||
       condTable[condId] == NULL ||
       lockTable[lockId] == NULL) {

      machine->WriteRegister(2, -1);
      return;
   }

   condTable[condId]->Wait(lockTable[lockId]);

   machine->WriteRegister(2, 0);
}


/*
 *  System call interface: int CondBroadcast( Cond_t )
 */
void NachOS_CondBroadcast()
{
   int condId = machine->ReadRegister(4);
   int lockId = machine->ReadRegister(5);

   condTable[condId]->Broadcast(
      lockTable[lockId]);

   machine->WriteRegister(2, 0);
}
/*
 *  System call interface: Socket_t Socket( int, int )
 */
void NachOS_Socket() {
   int family = machine->ReadRegister(4);
   int type = machine->ReadRegister(5);

   int sockFamily = AF_INET;
   int sockType = SOCK_STREAM;

   if (type == SOCK_DGRAM_NachOS) {
      sockType = SOCK_DGRAM;
   }

   int unixFd = socket(sockFamily, sockType, 0);

   if (unixFd < 0) {
      machine->WriteRegister(2, -1);
      return;
   }

   int nachosFd =
   currentThread->space->openFilesTable->Open(unixFd);

   machine->WriteRegister(2, nachosFd);
}

/*
 *  System call interface: Socket_t Connect( char *, int )
 */
void NachOS_Connect()
{
   int sockId = machine->ReadRegister(4);
   int ipAddr = machine->ReadRegister(5);
   int port = machine->ReadRegister(6);

   char ip[64];
   int car;
   int i = 0;

   while (i < 63) {
      machine->ReadMem(ipAddr + i, 1, &car);
      ip[i] = (char) car;

      if (ip[i] == '\0') break;
      i++;
   }

   ip[63] = '\0';

   struct sockaddr_in server;
   bzero((char *) &server, sizeof(server));

   server.sin_family = AF_INET;
   server.sin_port = htons(port);
   server.sin_addr.s_addr = inet_addr(ip);

   int unixFd = currentThread->space->openFilesTable->getUnixHandle(sockId);

   if (unixFd < 0) {
      machine->WriteRegister(2, -1);
      return;
   }

   int result = connect(unixFd,(struct sockaddr *) &server,sizeof(server));
   machine->WriteRegister(2, result);
}

/*
 *  System call interface: int Bind( Socket_t, int )
 */
void NachOS_Bind()
{
   int sockId = machine->ReadRegister(4);
   int port = machine->ReadRegister(5);

   int unixFd =
      currentThread->space->openFilesTable
         ->getUnixHandle(sockId);

   if (unixFd < 0) {
      machine->WriteRegister(2, -1);
      return;
   }

   sockaddr_in addr;

   bzero((char *)&addr, sizeof(addr));

   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = INADDR_ANY;
   addr.sin_port = htons(port);

   int result =
      bind(unixFd,
           (struct sockaddr *)&addr,
           sizeof(addr));

   machine->WriteRegister(2, result);
}
/*
 *  System call interface: int Listen( Socket_t, int )
 */
void NachOS_Listen()
{
   int sockId = machine->ReadRegister(4);
   int backlog = machine->ReadRegister(5);

   int unixFd =
      currentThread->space->openFilesTable
         ->getUnixHandle(sockId);

   if (unixFd < 0) {
      machine->WriteRegister(2, -1);
      return;
   }

   int result = listen(unixFd, backlog);

   machine->WriteRegister(2, result);
}

/*
 *  System call interface: int Accept( Socket_t )
 */
void NachOS_Accept()
{
   int sockId = machine->ReadRegister(4);

   int unixFd =
      currentThread->space->openFilesTable
         ->getUnixHandle(sockId);

   if (unixFd < 0) {
      machine->WriteRegister(2, -1);
      return;
   }

   sockaddr_in client;
   socklen_t len = sizeof(client);

   int newUnixFd =
      accept(unixFd,
             (struct sockaddr *)&client,
             &len);

   if (newUnixFd < 0) {
      machine->WriteRegister(2, -1);
      return;
   }

   int nachosFd =
      currentThread->space->openFilesTable
         ->Open(newUnixFd);

   machine->WriteRegister(2, nachosFd);
}

/*
 *  System call interface: int Shutdown( Socket_t, int )
 */
void NachOS_Shutdown() {
   int sockId = machine->ReadRegister(4);
   int how = machine->ReadRegister(5);

   int unixFd =
      currentThread->space->openFilesTable
         ->getUnixHandle(sockId);

   if (unixFd < 0) {
      machine->WriteRegister(2, -1);
      return;
   }

   int result = shutdown(unixFd, how);

   machine->WriteRegister(2, result);
}

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2) - SC_Base;
    switch ( which ) {

       case SyscallException:
          switch ( type ) {
             case SC_Halt:		// System call # 0
                NachOS_Halt();
                updatePc();
                break;
             case SC_Exit:		// System call # 1
                NachOS_Exit();
                break;
             case SC_Exec:		// System call # 2
                NachOS_Exec();
                updatePc();
                break;
             case SC_Join:		// System call # 3
                NachOS_Join();
                updatePc();
                break;

             case SC_Create:		// System call # 4
                NachOS_Create();
                updatePc();
                break;
             case SC_Open:		// System call # 5
                NachOS_Open();
                updatePc();
                break;
            case SC_Write:		// System call # 7
               NachOS_Write();
               updatePc();
               break;
             case SC_Read:		// System call # 6
                NachOS_Read();
                updatePc();
                break;
             case SC_Close:		// System call # 8
                NachOS_Close();
                updatePc();
                break;

             case SC_Fork:		// System call # 9
                NachOS_Fork();
                break;
             case SC_Yield:		// System call # 10
                NachOS_Yield();
                updatePc();
                break;

             case SC_SemCreate:         // System call # 11
                NachOS_SemCreate();
                updatePc();
                break;
             case SC_SemDestroy:        // System call # 12
                NachOS_SemDestroy();
                updatePc();
                break;
             case SC_SemSignal:         // System call # 13
                NachOS_SemSignal();
                updatePc();
                break;
             case SC_SemWait:           // System call # 14
                NachOS_SemWait();
                updatePc();
                break;

             case SC_LckCreate:         // System call # 15
                NachOS_LockCreate();
                updatePc();
                break;
             case SC_LckDestroy:        // System call # 16
                NachOS_LockDestroy();
                updatePc();
                break;
             case SC_LckAcquire:         // System call # 17
                NachOS_LockAcquire();
                updatePc();
                break;
             case SC_LckRelease:           // System call # 18
                NachOS_LockRelease();
                updatePc();
                break;

             case SC_CondCreate:         // System call # 19
                NachOS_CondCreate();
                updatePc();
                break;
             case SC_CondDestroy:        // System call # 20
                NachOS_CondDestroy();
                updatePc();
                break;
             case SC_CondSignal:         // System call # 21
                NachOS_CondSignal();
                updatePc();
                break;
             case SC_CondWait:           // System call # 22
                NachOS_CondWait();
                updatePc();
                break;
             case SC_CondBroadcast:           // System call # 23
                NachOS_CondBroadcast();
                updatePc();
                break;

            case SC_Socket:	// System call # 30
               NachOS_Socket();
               updatePc();
               break;
            case SC_Connect:	// System call # 31
               NachOS_Connect();
               updatePc();
               break;
            case SC_Bind:	// System call # 32
               NachOS_Bind();
               updatePc();
               break;
            case SC_Listen:	// System call # 33
               NachOS_Listen();
               updatePc();
               break;
            case SC_Accept:	// System call # 32
               NachOS_Accept();
               updatePc();
               break;
            case SC_Shutdown:	// System call # 33
               NachOS_Shutdown();
               updatePc();
               break;

             default:
                printf("NachOS version: %d-%d\n", (SC_Base + SC_NachOS)/10, (SC_Base + SC_NachOS)%10 );
                printf("Unexpected syscall exception %d\n", type );
                ASSERT( false );
                break;
          }
          break;

       case PageFaultException:
         printf("PageFaultException en PC=%d, BadVAddr=%d\n",
         machine->ReadRegister(PCReg),
         machine->ReadRegister(BadVAddrReg));
         ASSERT(false);
         break;

       case ReadOnlyException:
          printf( "Read Only exception (%d)\n", which );
          ASSERT( false );
          break;

       case BusErrorException:
          printf( "Bus error exception (%d)\n", which );
          ASSERT( false );
          break;

       case AddressErrorException:
          printf( "Address error exception (%d)\n", which );
          ASSERT( false );
          break;

       case OverflowException:
          printf( "Overflow exception (%d)\n", which );
          ASSERT( false );
          break;

       case IllegalInstrException:
          printf( "Ilegal instruction exception (%d)\n", which );
          ASSERT( false );
          break;

       default:
          printf( "Unexpected exception %d\n", which );
          ASSERT( false );
          break;
    }

}