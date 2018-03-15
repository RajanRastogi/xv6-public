#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_THREADS 10

struct tls {
  uint tid;
};

typedef struct balance {
   char name[32];
   char amount;
} balance_t;

balance_t per_thread_balance[MAX_THREADS];

volatile int total_balance = 0;

void *baseAdr;

int gettid(){
  int dummy;
  int stackAdr = (int)&dummy;
  int stackIndex = stackAdr/4096;
  int baseIndex = (int)baseAdr/4096;
  int selfIndex = stackIndex - baseIndex;
  return selfIndex;
}

int foo(){
  int tid = gettid();
  per_thread_balance[tid].amount = 10*tid;
  thread_exit();
  return 0; 
}

int main(int argc, char *argv[]){
  int i = 0, fakeNum = 0;
  for(i = 0; i < MAX_THREADS; i++){
    per_thread_balance[i].amount = 200;
    per_thread_balance[i].name[0] = (char)i;
  }

  baseAdr = sbrk(0);
   
  for(i = 0; i < MAX_THREADS; i++){
    void *stack = sbrk(4096);
    thread_create((void*)foo,(void*)&fakeNum, stack);
  }
  
  for(i = 0; i < MAX_THREADS; i++){
    thread_join();
  }

  for(i = 0; i < MAX_THREADS; i++){
   printf(1, "Thread %d value is: %d\n", i, per_thread_balance[i].amount);
  }

  exit();
}



