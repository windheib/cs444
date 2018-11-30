#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "concurrency3.h"

#define MAX_NUM 100
#define NUM_SEARCHERS 3
#define PRINT_INTERVAL 2

using namespace std;

int main(int argc, char const *argv[]) {
  cout << "------ testDeletion2 BEGIN ------" << endl;
  cout << ">> testing capabilities for algorithm to reject another operator on a resource when a deleter has it" << endl;
  srand(time(NULL));	// seed random

  // initialize resource
  struct Resource resource;
  resource.sizeOfList = 0;
  resource.listHead = NULL;
  resource.listTail = NULL;

  // init state, no need to lock since this is the only thread at this point
  state.hasDeleter = false;
  state.counter = 0;
  state.reset = false;

  // initialize locks
  pthread_mutex_init(&resourceLock, NULL);
  pthread_mutex_init(&stateLock, NULL);


  // initialize threads, 1 deleter, 1 inserter, 1 searcher, and 1 helper printer
  pthread_t deleterThread;
  pthread_t inserterThread;
  pthread_t searcherThread;
  pthread_t printerThread;

  cout << ">> Initializing 1 deleter & printer thread" << endl;
  // begin threads
  pthread_create(&printerThread, NULL, printer, &resource);
  pthread_create(&deleterThread, NULL, deleter, &resource);

  std::cout << ">> Asserting that deleter creation is successful" << '\n';
  assert(state.counter == 1);
  std::cout << ">> SUCCESS!" << '\n\n';

  std::cout << ">> Attempting to start searcher" << '\n';
  pthread_create(&searcherThread, NULL, searcher, &resource);

  std::cout << ">> Asserting that the searcher was not allowed to access the resource" << '\n';
  assert(state.counter == 1);
  std::cout << ">> SUCCESS!" << '\n\n';


  std::cout << "------ testDeletion2 SUCCESSFUL ------" << '\n\n\n';
  exit(0); //kills child threads as well
}
