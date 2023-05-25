/* This is a small demo of the high-performance ThreadX kernel.  It includes examples of eight
   threads of different priorities, using a message queue, semaphore, mutex, event flags group, 
   byte pool, and block pool.  */

#include "tx_api.h"

#define DEMO_STACK_SIZE         400
#define DEMO_BYTE_POOL_SIZE     1024
#define DEMO_BLOCK_POOL_SIZE    10
#define DEMO_QUEUE_SIZE         10


/* Define the ThreadX object control blocks...  */

TX_THREAD               thread_1;
TX_THREAD               thread_2;
TX_MUTEX                mutex_0;
TX_BYTE_POOL            byte_pool_0;
UCHAR                   memory_area[DEMO_BYTE_POOL_SIZE];


/* Define the counters used in the demo application...  */

ULONG                   thread_1_counter;
ULONG                   thread_1_mutex_sent;
ULONG                   thread_1_mutex_received;
ULONG                   thread_2_counter;
ULONG                   thread_2_mutex_sent;
ULONG                   thread_2_mutex_received;


/* Define thread prototypes.  */

void    thread_1_and_2_entry(ULONG thread_input);


/* Define main entry point.  */

int main()
{

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


/* Define what the initial system looks like.  */

void    tx_application_define(void *first_unused_memory)
{

CHAR    *pointer = TX_NULL;
	UINT status;

    /* Create a byte memory pool from which to allocate the thread stacks.  */
	status = tx_byte_pool_create(&byte_pool_0, "byte pool 0", memory_area, DEMO_BYTE_POOL_SIZE);
    if(status != TX_SUCCESS) while(1);

    /* Put system definition stuff in here, e.g. thread creates and other assorted
       create information.  */

    /* Allocate the stack for thread 1.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    if(status != TX_SUCCESS) while(1);

    /* Create threads 1 and 2. These threads pass information through a ThreadX 
       message queue.  It is also interesting to note that these threads have a time
       slice.  */
    status = tx_thread_create(&thread_1, "thread 1", thread_1_and_2_entry, 1,
            pointer, DEMO_STACK_SIZE,
            8, 8, TX_NO_TIME_SLICE, TX_AUTO_START);
    if(status != TX_SUCCESS) while(1);

    /* Allocate the stack for thread 2.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    if(status != TX_SUCCESS) while(1);

    status = tx_thread_create(&thread_2, "thread 2", thread_1_and_2_entry, 2,
            pointer, DEMO_STACK_SIZE,
            8, 8, TX_NO_TIME_SLICE, TX_AUTO_START);
    if(status != TX_SUCCESS) while(1);

    /* Allocate the message queue.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_QUEUE_SIZE*sizeof(ULONG), TX_NO_WAIT);
    if(status != TX_SUCCESS) while(1);

    /* Create the mutex used by thread 1 and 2 without priority inheritance.  */
    status = tx_mutex_create(&mutex_0, "mutex 0", TX_NO_INHERIT);
    if(status != TX_SUCCESS) while(1);

    /* Release the block back to the pool.  */
    status = tx_byte_release(pointer);
    if(status != TX_SUCCESS) while(1);
}


void    thread_1_and_2_entry(ULONG thread_input)
{
	UINT    status;


    /* This function is executed from thread 1 and thread 2.  As the loop
       below shows, these function compete for ownership of mutex_0.  */
    while(1)  {

        /* Increment the thread counter.  */
        if (thread_input == 1)
            thread_1_counter++;
        else
            thread_2_counter++;

        /* Get the mutex with suspension.  */
        status =  tx_mutex_get(&mutex_0, TX_WAIT_FOREVER);
        /* Check status.  */
        if (status != TX_SUCCESS) break;

        /* Get the mutex again with suspension.  This shows
           that an owning thread may retrieve the mutex it
           owns multiple times.  */
        status =  tx_mutex_get(&mutex_0, TX_WAIT_FOREVER);
        /* Check status.  */
        if (status != TX_SUCCESS) break;

        /* Sleep for 2 ticks to hold the mutex.  */
        tx_thread_sleep(2);

        /* Release the mutex.  */
        status =  tx_mutex_put(&mutex_0);
        /* Check status.  */
        if (status != TX_SUCCESS) break;

        /* Release the mutex again.  This will actually
           release ownership since it was obtained twice.  */
        status =  tx_mutex_put(&mutex_0);
        /* Check status.  */
        if (status != TX_SUCCESS) break;
    }
}
