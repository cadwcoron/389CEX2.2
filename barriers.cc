#include <pthread.h>
#include <iostream>
#include <cstdlib>
#include <math.h>
#include <atomic>
#include <unistd.h>
#include "barriers.hh"
#include "pthread.h"
#define NUM_THREADS 32
using namespace std; 
atomic<int> globalCounter= 2;
// you have to set the rounds by hand, because there were issues with the global variable not being constant
const int rounds = 1;
bool senseForDissemination = false;
bool senseForMCS = true;
bool flagsForDissemination[NUM_THREADS][2][rounds];
bool flagsForMCSUp[NUM_THREADS];
bool flagsForMCSDown[NUM_THREADS];


void *centralizedBarrier(void *threadid) {
   long tid;
   tid = (long)threadid;
   printf("Waiting for counter==P. Thread ID: %ld\n", tid);
   bool local_sense = ! senseForDissemination;

   // increment and wait til counter == P
   if (atomic_fetch_add(&globalCounter,1) == NUM_THREADS) {
    senseForDissemination = local_sense;
   } else {
      while (senseForDissemination != local_sense) { /* spin */ }
   }

   while(globalCounter<NUM_THREADS){}
   printf("Thread %ld, moving on !\n", tid);
}


void *disseminationBarrier(void *threadid) {
   long tid;
   tid = (long)threadid;
   bool sense = false;
   int parity = 0;

   // go through rounds
   for (int round = 0; round< rounds; round++) {

      // notify neighbor
      int partner = (tid + 2^round) % NUM_THREADS;
      printf("Thread: %ld ", tid);
      printf("in round %i, ", round);
      printf("notifiying partner %i.\n", partner);
      flagsForDissemination[partner][parity][round] = !sense;
      
      // wait for your neighbor next round
      while (flagsForDissemination[tid][parity][round] == sense)
         { /* spin */ }
   }
   if (parity == 1) {
      sense = !sense;
   }
   parity = 1- parity;
   printf("Thread: %ld ", tid);
   printf("moving on.\n");
}


void *MCSBarrier(void *threadid) {
   long tid;
   tid = (long)threadid;

   // calculate parent and children
   int c1 = 2*tid+1;
   int c2 = 2*(tid+1);
   int parent = floor((tid-1)/2);
   cout << "Thread: " << tid << " entered, child 1 = " << c1 << ", child 2 = " << c2 << ", parent " << parent << endl;

   // if you're a leaf, you win automatically
   if( c1 <NUM_THREADS || c2<NUM_THREADS){
      while (flagsForMCSUp[c1] != senseForMCS || (c2<NUM_THREADS && flagsForMCSUp[c2] != senseForMCS)) { /* spin */ }
   }  
   //set variable and wait for parent on way down
   printf("Thread %ld set, now going to sleep\n", tid);
   flagsForMCSUp[tid] = senseForMCS;
   if (tid!=0){
      //wait for children
      cout << "Waiting for parent on thread " << tid << endl;
      while (flagsForMCSDown[parent]!= senseForMCS){/* spin */};
   }
   printf("Exiting Barrier - Thread: %ld\n", tid);
   
   flagsForMCSDown[tid] = senseForMCS;
   if(tid ==0){
      senseForMCS = !senseForMCS;
   }
}


int main () {
   pthread_t threads[NUM_THREADS];

   int rc;
   int i;

   pthread_attr_t attr;
   void *status;

   //initialize the threads
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

   struct timespec start, end;

   clock_gettime(CLOCK_MONOTONIC, &start);

   for( i = 0; i < NUM_THREADS; i++ ) {
      cout << "main() : creating thread, " << i << endl;
      int randomTime = rand() %1000 + 1;
      // call the barrier on the thread
      usleep(randomTime);
      rc = pthread_create(&threads[i], NULL, disseminationBarrier, (void *)i);
      if (rc) {
         printf("Error:unable to create thread, %d\n", rc);
         exit(-1);
      }
   }

   // join threads
   pthread_attr_destroy(&attr);
   for( i = 0; i < NUM_THREADS; i++ ) {
      rc = pthread_join(threads[i], &status);
      if (rc) {
         cout << "Error:unable to join," << rc << endl;
         exit(-1);
      }
   }


   clock_gettime(CLOCK_MONOTONIC, &end);

   //report time
   double time = end.tv_nsec-start.tv_nsec;
   cout << "Time: " << time << endl;
   pthread_exit(NULL);
   return 1;
}