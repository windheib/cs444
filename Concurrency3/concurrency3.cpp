#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>

#define NUM_SEARCHERS 3
#define SLEEPYTIME_SEARCH 2
#define SLEEPYTIME_INSERT 5
#define SLEEPYTIME_DELETE 4

using namespace std;

// global linked list

pthread_mutex_t searcherLock[NUM_SEARCHERS];
pthread_mutex_t inserterLock;
pthread_mutex_t deleterLock;
pthread_mutex_t outputLock;

int flag;

struct Node {
	int num; 
	struct Node *next;
	struct Node *prev;
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
		int lockNum;
		int numToSearch = rand() % 100;
		if(resource->counter == 0) {
			if(flag == 0){
				pthread_mutex_lock(&outputLock);
				cout << "... resource empty ..." << endl;
				pthread_mutex_unlock(&outputLock);
			}
			resource->isFull = false;
		}
		if(!resource->hasDeleter && !resource->isFull) {
			for(int i = 0; i < NUM_SEARCHERS; i++) {
				if(!pthread_mutex_trylock(&searcherLock[i])){	// try to get one of three locks. One should be available if we get to here
					lockNum = i;
					resource->counter++;
					if(resource->counter == 3){
						pthread_mutex_lock(&outputLock);
						cout << "*** !RESOURCE FULL! ***" << endl;
						pthread_mutex_unlock(&outputLock);				
						resource->isFull = true;
					}
					break;
				}
			}

			pthread_mutex_lock(&outputLock);
			cout << "Searching for: " << numToSearch << endl;
			pthread_mutex_unlock(&outputLock);

			struct Node* listNode = resource->listHead;
			while(true) {
				if(listNode == NULL) {	// end of list, no dice
					pthread_mutex_lock(&outputLock);
					cout << "    " << numToSearch << " not found" << endl;
					pthread_mutex_unlock(&outputLock);
					break;
				} else if(listNode->num == numToSearch) {	// found it!
					pthread_mutex_lock(&outputLock);
					cout << "    Found number: " << numToSearch << endl;
					pthread_mutex_unlock(&outputLock);
					break;
				}
				listNode = listNode->next;	// iterate
			}

			sleep(SLEEPYTIME_SEARCH);	// sleep
			pthread_mutex_unlock(&searcherLock[lockNum]);	// unlock specific mutex
			resource->counter--;	// count down. when counter hits 0, the isFull flag resets
		}
	}
}

void* inserter(void* ptr) {
	struct Resource* resource = (struct Resource*) ptr;

	while(true) {
		int numToInsert = rand() % 100;
		if(resource->counter == 0){
			if(flag == 0){
				pthread_mutex_lock(&outputLock);
				cout << "... resource empty ..." << endl;
				pthread_mutex_unlock(&outputLock);
			}
			resource->isFull = false;
		}

		if(!resource->hasDeleter && !resource->isFull && !resource->hasInserter) {
			pthread_mutex_lock(&inserterLock);
			pthread_mutex_lock(&outputLock);
			cout << "Inserting: " << numToInsert << endl;
			pthread_mutex_unlock(&outputLock);

			resource->hasInserter = true;
			resource->counter++;
			if(resource->counter == 3){
				pthread_mutex_lock(&outputLock);
				cout << "*** !RESOURCE FULL! ***" << endl;
				pthread_mutex_unlock(&outputLock);				
				resource->isFull = true;
			}

			struct Node* newNode = new struct Node;
			newNode->num = numToInsert;
			newNode->next = NULL;
			if(resource->listHead == NULL) {
				resource->listHead = newNode;
				resource->listTail = newNode;
			} else {
				resource->listTail->next = newNode;
				resource->listTail = newNode;
			}
			// while(true) {
			// 	if(listNode == NULL) {
			// 		// struct Node* newNode = (struct Node*) malloc(sizeof(struct Node));
			// 		// newNode->num = numToInsert;
			// 		// listNode->prev->next = newNode;
			// 		// newNode->next = NULL;
			// 		// newNode->prev = listNode->prev;
			// 		// //resource->listTail->prev = newNode;

			pthread_mutex_lock(&outputLock);
			cout << "    " << numToInsert << " inserted" << endl;
			pthread_mutex_unlock(&outputLock);
			resource->sizeOfList++;
			// 		// break;
			// 	}
			// 	listNode = listNode->next;
			// }

		}

		sleep(SLEEPYTIME_INSERT);
		pthread_mutex_unlock(&inserterLock);
		resource->counter--;
		resource->hasInserter = false;

	}
}

void* deleter(void* ptr) {
	struct Resource* resource = (struct Resource*) ptr;
	while(true) {
		if(resource->counter == 0){
			if(flag == 0){
				pthread_mutex_lock(&outputLock);
				cout << "... resource FUCKempty ..." << endl;
				pthread_mutex_unlock(&outputLock);
			}
			resource->isFull = false;
		}
		if(resource->sizeOfList > 0){
			int indexToDelete = rand() % resource->sizeOfList + 1;
			resource->hasDeleter = true; // add deleter so other threads stop
			if(!resource->isFull || resource->hasInserter) {
				// cout << resource->isFull << resource->hasInserter << endl;
				flag = 1;
				continue;
			} 
			if(resource->counter == 0){
				pthread_mutex_lock(&deleterLock);
				pthread_mutex_lock(&outputLock);
				cout << "Deleting index: " << indexToDelete << endl;
				pthread_mutex_unlock(&outputLock);
				resource->counter++;
				struct Node* cur = new struct Node;
				struct Node* prev = new struct Node;
				cur = resource->listHead;
				for(int i = 1; i < indexToDelete; i++) {
					prev = cur;
					cur = cur->next;
				}
				prev->next = cur->next;

				pthread_mutex_lock(&outputLock);
				cout << "Deleted index: " << indexToDelete << endl;
				pthread_mutex_unlock(&outputLock);
				resource->sizeOfList--;
			}
		} else {
			// pthread_mutex_lock(&deleterLock);
			pthread_mutex_lock(&outputLock);
			cout << "    Nothing to delete" << endl;
			pthread_mutex_unlock(&outputLock);

			sleep(SLEEPYTIME_DELETE);
			continue;
		}

		sleep(SLEEPYTIME_DELETE);
		pthread_mutex_unlock(&deleterLock);
		resource->counter--;
		resource->hasDeleter = false;
	}
}


int main(int argc, char* argv[]) {
	struct Resource resource;
	resource.sizeOfList = 0;
	resource.isFull = false;
	resource.hasInserter = false;
	resource.hasDeleter = false;
	resource.counter = 0;
	resource.listHead = NULL;
	// resource.listHead->prev = NULL;
	// resource.listHead->next = resource.listTail;
	resource.listTail = NULL;
	// resource.listTail->next = NULL;
	// resource.listTail->prev = resource.listHead;
	flag = 0;

	srand(time(NULL));

	pthread_t searcherThread[NUM_SEARCHERS];
	pthread_t deleterThread;
	pthread_t inserterThread;

	pthread_mutex_init(&inserterLock, NULL);
	pthread_mutex_init(&deleterLock, NULL);
	pthread_mutex_init(&outputLock, NULL);

	pthread_create(&inserterThread, NULL, inserter, &resource);

	for(int i = 0; i < NUM_SEARCHERS; i++) {
		pthread_mutex_init(&searcherLock[i], NULL);
		pthread_create(&searcherThread[i], NULL, searcher, &resource);
	}

	pthread_create(&deleterThread, NULL, deleter, &resource);

	for(int i = 0; i < NUM_SEARCHERS; i++) {
		pthread_join(searcherThread[i], NULL);
	}
	pthread_join(inserterThread, NULL);
	pthread_join(deleterThread, NULL);

	return 0;
}