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

int int main(int argc, char const *argv[]) {
  cout << "------ testInserter BEGIN ------" << endl;
  cout << ">> testing capabilities for algorithm to introduce a second inserter on a resource" << endl;
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


  // initialize threads, 2 inserters, 1 helper printer
  pthread_t inserterThread[2];
  pthread_t printerThread;

  cout << ">> Initializing 1 inserter & printer thread" << endl;
  // begin threads
  pthread_create(&printerThread, NULL, printer, &resource);
  pthread_create(&inserterThread[0], NULL, inserter, &resource);

  std::cout << ">> Asserting that inserter creation is successful" << '\n';
  assert(state.counter == 1);
  std::cout << ">> SUCCESS!" << '\n\n';

  std::cout << ">> Sleeping to allow search threads to run" << '\n\n';
  randomSleep();

  std::cout << ">> Attempting to start second inserter" << '\n';
  pthread_create(&inserterThread[1], NULL, inserter, &resource);

  std::cout << ">> Asserting that the second inserter was not allowed to access the resource" << '\n';
  assert(state.counter == 1);
  std::cout << ">> SUCCESS!" << '\n\n';


  std::cout << "------ testInserter SUCCESSFUL ------" << '\n\n\n';
  exit(0); //kills child threads as well
}
