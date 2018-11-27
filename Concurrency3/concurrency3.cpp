/*********************************************************
 ** Concurrency 3 - Operating Systems II - Kyson Montague
 **	Written by Ben Windheim, Kyle Baldes, Burton Jaursch
 ** 25 November 2018
*********************************************************/
#include "concurrency3.h"

using namespace std;

/*********************************************************
 ** Function - helper functions
 **	Description - development ease
*********************************************************/
void randomSleep() {
	usleep(rand() % 10000);	// microseconds, so up to 10 milliseconds
}
void lockState() {
	pthread_mutex_lock(&stateLock);
}
void unlockState() {
	pthread_mutex_unlock(&stateLock);
}
void lockResource() {
	pthread_mutex_lock(&resourceLock);
}
void unlockResource() {
	pthread_mutex_unlock(&resourceLock);
}

/*********************************************************
 ** Function - printList
 **	Description - prints the resource (linked list)
*********************************************************/
void printList(struct Resource* resource) {
	// print the list
	struct Node* temp = NULL;
	lockResource();
	temp = resource->listHead;
	cout << "          LIST SIZE: " << resource->sizeOfList << " --- ";
	while(temp != NULL) {
		cout << temp->num << ", " ;
		temp = temp->next;
	}
	cout << endl;
	unlockResource();
}

/*********************************************************
 ** Function - insert
 **	Description - inserts an item at the end of the list
*********************************************************/
void insert(struct Resource* resource, int numToInsert) {
	cout << "Inserting: " << numToInsert << "   -    " << state.counter << " running threads" << endl;

	struct Node* newNode = new struct Node;
	newNode->num = numToInsert;
	newNode->next = NULL;
	if(resource->listHead == NULL) {	// add at beginning, set head
		resource->listHead = newNode;
		resource->listTail = newNode;
	} else {							// add at end
		resource->listTail->next = newNode;
		resource->listTail = newNode;
	}
	resource->sizeOfList++;
	cout << "	Inserted: " << numToInsert << " at end of list" << endl;
}

/*********************************************************
 ** Function - search
 **	Description - looks for a value in the list
*********************************************************/
void search(struct Resource* resource, int numToSearch) {
	cout << "Searching for: " << numToSearch << "   -    " << state.counter << " running threads" << endl;

	struct Node* listNode = resource->listHead;
	while (listNode != NULL) {
		if(listNode->num == numToSearch) {	// found it!
			cout << "       found!: " << numToSearch << endl;
			return;
		}
		listNode = listNode->next;	// iterate
	}
	cout << "    not found: " << numToSearch << endl;
}

/*********************************************************
 ** Function - remove
 **	Description - removes the item in the list at indexToDelete
*********************************************************/
void remove(struct Resource* resource, int indexToDelete) {
	cout << "Deleting index: " << indexToDelete << "   -    " << state.counter << " running threads" << endl;

	if (resource->listHead == NULL || indexToDelete < 0 || resource->sizeOfList <= indexToDelete) {
		// don't go past the end of list
		cout << "WARNING - attempted to remove index past end of list: " << indexToDelete << endl;
		return;
	}
	int num = -1;
	struct Node* current = resource->listHead;
	int i = indexToDelete;
	// advance to node prior to one to delete
	while (--i > 0) {
		current = current->next;
	}
	// if we happen to be at the head, special-case
	if (current == resource->listHead) {
		resource->listHead = current->next;
		num = current->num;
		delete current;
	} else {
		// delete the next node
		struct Node* temp = current->next->next;
		num = current->next->num;
		delete current->next;
		current->next = temp;
		if (current->next == NULL) {
			resource->listTail = current;
		}
	}
	resource->sizeOfList = resource->sizeOfList - 1;

	cout << "	Deleted index: " << indexToDelete << " with num: " << num << endl;
}

/*********************************************************
 ** Function - printer
 **	Description - shows how many threads are running at a consistent interval
*********************************************************/
void* printer(void* ptr) {
	struct Resource* resource = (struct Resource*) ptr;
	while(true) {
		lockState();
		int counter = state.counter;
		unlockState();
		printList(resource);
		sleep(PRINT_INTERVAL);
	}
}

/*********************************************************
 ** Function - searcher
 **	Description - consistently looping function to search for a value in the resource. Up to three can be running concurrently
*********************************************************/
void* searcher(void* ptr) {
	struct Resource* resource = (struct Resource*) ptr;

	while (true) {
		bool searchNow = false;
		lockState();
		if (state.counter < 3 && !state.hasDeleter) { 	// checks if there's room for a searcher
			if(state.counter == 0) {
				if(state.reset)
					cout << "			Resource emptied" << endl;
				state.reset = false;
			}
			if(!state.reset){
				state.counter++;
				if(state.counter == 3) {
					cout << "			Resource at capacity" << endl;
					state.reset = true;
				}
				searchNow = true;
			}
		}
		unlockState();
		if (searchNow) {
			int numToSearch = rand() % MAX_NUM;	// random number to look for
			
			// lockResource();
			search(resource, numToSearch);	// lock state and search
			// unlockResource();
			
			lockState();
			state.counter--;
			unlockState();
		}

		randomSleep();	// chill for a minute
	}
}

/*********************************************************
 ** Function - inserter
 **	Description - insertion thread. Only one, 
*********************************************************/
void* inserter(void* ptr) {
	struct Resource* resource = (struct Resource*) ptr;

	while (true) {
		bool insertNow = false;
		lockState();
		if (state.counter < 3 && !state.hasDeleter && !state.hasInserter) { 	// check if there's room
			if(state.counter == 0) {
				state.reset = false;
			}
			if(!state.reset){
				state.counter++;
				if(state.counter == 3) {
					state.reset = true;
					cout << "Resource at capacity" << endl;
				}
				state.hasInserter = true;
				insertNow = true;
			}
		}
		unlockState();
		if (insertNow) {
			int numToInsert = rand() % MAX_NUM;	// random number to insert

			// lockResource();
			insert(resource, numToInsert);	// insert at end of list
			// unlockResource();	
			
			lockState();
			state.counter--;
			state.hasInserter = false;
			unlockState();
		}

		randomSleep();
	}
}

/*********************************************************
 ** Function - deleter
 **	Description - deletion thread. mutually exclusive with all other threads, only one can run. 
*********************************************************/
void* deleter(void* ptr) {
	struct Resource* resource = (struct Resource*) ptr;

	while (true) {
		bool deleteNow = false;
		lockState();
		if (state.counter == 0 && !state.hasDeleter) {	// waits for resource to be empty
			state.reset = false;
			state.counter++;
			state.hasDeleter = true;	// set flag
			deleteNow = true;
		}
		unlockState();
		if (deleteNow) {
			lockResource();
			if (resource->sizeOfList > 0) {
				int indexToDelete = rand() % resource->sizeOfList;	// get a random index
				remove(resource, indexToDelete);	// delete it
			}
			unlockResource();
			
			lockState();
			state.counter--;
			state.hasDeleter = false;
			unlockState();
		}

		randomSleep();
	}
}

/*********************************************************
 ** Function - main
 **	Description - initializes threads and data structures for running the program
*********************************************************/
int main(int argc, char* argv[]) {
	srand(time(NULL));	// seed random

	// initialize resource
	struct Resource resource;
	resource.sizeOfList = 0;
	resource.listHead = NULL;
	resource.listTail = NULL;

	// init state, no need to lock since this is the only thread at this point
	state.hasDeleter = false;
	state.hasInserter = false;
	state.counter = 0;
	state.reset = false;

	// initialize locks
	pthread_mutex_init(&resourceLock, NULL);
	pthread_mutex_init(&stateLock, NULL);

	// initialize threads, 3 searchers, 3 deleters, 3 inserters, 1 helper printer
	pthread_t printerThread;
	pthread_t searcherThread[NUM_THREADS];
	pthread_t deleterThread[NUM_THREADS];
	pthread_t inserterThread[NUM_THREADS];

	// begin threads
	pthread_create(&printerThread, NULL, printer, &resource);
	for(int i = 0; i < NUM_THREADS; i++) {
		pthread_create(&inserterThread[i], NULL, inserter, &resource);
		pthread_create(&searcherThread[i], NULL, searcher, &resource);
		pthread_create(&deleterThread[i], NULL, deleter, &resource);
	}

	// join  threads
	for(int i = 0; i < NUM_THREADS; i++) {
		pthread_join(searcherThread[i], NULL);
		pthread_join(inserterThread[i], NULL);
		pthread_join(deleterThread[i], NULL);
	}
	pthread_join(printerThread, NULL);

	return 0;
}
