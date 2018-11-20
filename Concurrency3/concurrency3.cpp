#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>

#define NUM_SEARCHERS 3

using namespace std;

// global linked list

pthread_mutex_t searcherLock;
pthread_mutex_t inserterLock;
pthread_mutex_t deleterLock;

struct Node {
	int num; 
	struct Node *next;
};

struct Resource {
	struct Node* listHead;
	struct Node* listTail;
	int sizeOfList;
	bool isFull;
	int counter; // 0-3
	bool hasDeleter;
	bool hasInserter;
};


void* searcher(void* ptr) {
	struct Resource* resource = (struct Resource*) ptr;
	while(true){
		int numToSearch = rand() % 100;
		// obtain access to Resource
		// search for numToSearch
		// include logic for counter
		// give up access
		break;
	}
}

void* inserter(void* ptr) {
	struct Resource* resource = (struct Resource*) ptr;
	while(true) {
		int numToInsert = rand() % 100;
		// obtain access to Resource
		// insert random num at end of list
		// unlock 
		break;
	}
}

void* deleter(void* ptr) {
	struct Resource* resource = (struct Resource*) ptr;
	while(true) {
		int indexToDelete = rand() % resource->sizeOfList;
		// obtain access
		// delete random index
		// unlock
		break;
	}
}


int main(int argc, char* argv[]) {
	// 
	struct Resource resource;
	resource.sizeOfList = 0;
	resource.isFull = false;
	resource.hasInserter = false;
	resource.hasDeleter = false;
	resource.counter = 0;

	srand(time(NULL));

	pthread_t searcherThread[NUM_SEARCHERS];
	pthread_t deleterThread;
	pthread_t inserterThread;

	pthread_mutex_init(&searcherLock, NULL);
	pthread_mutex_init(&inserterLock, NULL);
	pthread_mutex_init(&deleterLock, NULL);

	for(int i = 0; i < NUM_SEARCHERS; i++) {
		pthread_create(&searcherThread[i], NULL, searcher, &resource);
	}
	pthread_create(&deleterThread, NULL, deleter, &resource);
	pthread_create(&inserterThread, NULL, inserter, &resource);



	return 0;
}