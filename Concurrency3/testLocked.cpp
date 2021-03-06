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
  cout << "------ testLocked BEGIN ------" << endl;
  cout << ">> testing capabilities for algorithm to reject a fourth operator on resource" << endl;
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


  // initialize threads, 4 searchers, 1 helper printer
  pthread_t searcherThread[NUM_SEARCHERS + 1];
  pthread_t printerThread;

  cout << ">> Initializing three searchers & printer thread" << endl;
  // begin threads
  pthread_create(&printerThread, NULL, printer, &resource);
  for(int i = 0; i < NUM_SEARCHERS; i++) {
		pthread_create(&searcherThread[i], NULL, searcher, &resource);
	}

  std::cout << ">> Asserting that searcher creation is successful" << '\n';
  assert(state.counter == 3);
  std::cout << ">> SUCCESS!" << '\n\n';

  std::cout << ">> Attempting to start fourth searcher" << '\n';
  pthread_create(&searcherThread[i], NULL, searcher, &resource);

  std::cout << ">> Asserting that the fourth searcher was not allowed to access the resource" << '\n';
  assert(state.counter == 3);
  std::cout << ">> SUCCESS!" << '\n\n';


  std::cout << "------ testLocked SUCCESSFUL ------" << '\n\n\n';
  exit(0); //kills child threads as well
}
