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
  cout << "------ testOpen BEGIN ------" << endl;
  cout << ">> testing capabilities for algorithm to introduce third operator on resource" << endl;
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


  // initialize threads, 3 searchers, 1 helper printer
  pthread_t searcherThread[NUM_SEARCHERS];
  pthread_t printerThread;

  cout << ">> Initializing two searchers & printer thread" << endl;
  // begin threads
  pthread_create(&printerThread, NULL, printer, &resource);
  for(int i = 0; i < NUM_SEARCHERS - 1; i++) {
		pthread_create(&searcherThread[i], NULL, searcher, &resource);
	}

  std::cout << ">> Asserting that searcher creation is successful" << '\n';
  assert(state.counter == 2);
  std::cout << ">> SUCCESS!" << '\n\n';

  std::cout << ">> Sleeping to allow search threads to run" << '\n\n';
  randomSleep();

  std::cout << ">> Attempting to start third searcher" << '\n';
  pthread_create(&searcherThread[2], NULL, searcher, &resource);

  std::cout << ">> Asserting that third thread can access resource" << '\n';
  assert(state.counter == 3);
  std::cout << ">> SUCCESS!" << '\n\n';


  std::cout << "------ testOpen SUCCESSFUL ------" << '\n\n\n';
  exit(0);
}
