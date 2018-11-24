#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>

#define NUM_SEARCHERS 3
#define SLEEPYTIME_SEARCH 2
#define SLEEPYTIME_INSERT 4
#define SLEEPYTIME_DELETE 5

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
		if(pthread_mutex_trylock(&deleterLock)) {	// check if deleter is going
			continue;
		}
		pthread_mutex_unlock(&deleterLock);	// immediately give up lock

		int lockNum;
		int numToSearch = rand() % 100;

		if(resource->counter == 0) {		// reset isFull
			if(flag == 0){
				pthread_mutex_lock(&outputLock);
				cout << "... resource empty ..." << endl;
				pthread_mutex_unlock(&outputLock);
				flag = 1;
			}
			resource->isFull = false;
		}

		if(/*!resource->hasDeleter && */!resource->isFull) {	
			for(int i = 0; i < NUM_SEARCHERS; i++) {
				if(!pthread_mutex_trylock(&searcherLock[i])){	// try to get one of three locks. One should be available if we get to here
					lockNum = i;
					resource->counter++;
					break;
				}
			}

			pthread_mutex_lock(&outputLock);
			cout << "Searching for: " << numToSearch << endl;
			pthread_mutex_unlock(&outputLock);
			if(resource->counter == 3){
				pthread_mutex_lock(&outputLock);
				cout << "*** !RESOURCE FULL! ***" << endl;
				pthread_mutex_unlock(&outputLock);				
				resource->isFull = true;
			}

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
			// pthread_mutex_unlock(&deleterLock);
			pthread_mutex_unlock(&searcherLock[lockNum]);	// unlock specific mutex
			resource->counter--;	// count down. when counter hits 0, the isFull flag resets
			flag = 0;
		}
	}
}

void* inserter(void* ptr) {
	struct Resource* resource = (struct Resource*) ptr;

	while(true) {
		//sleep(1);
		if(pthread_mutex_trylock(&deleterLock)) {	// check for deleter
			// cout << "Deleter going" << endl;
			continue;
		}
		pthread_mutex_unlock(&deleterLock);	//immediately give it up if none

		int numToInsert = rand() % 100;
		if(resource->counter == 0){
			if(flag == 0){
				pthread_mutex_lock(&outputLock);
				cout << "... resource empty ..." << endl;
				pthread_mutex_unlock(&outputLock);
				flag = 1;
			}
			resource->isFull = false;
		}

		if(/*!resource->hasDeleter && */!resource->isFull/* && !resource->hasInserter*/) {
			pthread_mutex_lock(&inserterLock);

			pthread_mutex_lock(&outputLock);
			cout << "Inserting: " << numToInsert << endl;
			pthread_mutex_unlock(&outputLock);

			resource->hasInserter = true;

			resource->counter++;
			if(resource->counter == 3){			// set isFull
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

			pthread_mutex_lock(&outputLock);
			cout << "    " << numToInsert << " inserted" << endl;
			pthread_mutex_unlock(&outputLock);
			resource->sizeOfList++;
		}

		sleep(SLEEPYTIME_INSERT);
		//pthread_mutex_unlock(&deleterLock);
		resource->counter--;
		resource->hasInserter = false;
		pthread_mutex_unlock(&inserterLock);
		flag = 0;

	}
}

void* deleter(void* ptr) {
	struct Resource* resource = (struct Resource*) ptr;
	while(true) {
		if(pthread_mutex_trylock(&deleterLock)){
			continue;
		}
		int indexToDelete;
		if(resource->counter == 0){
			if(flag == 0){
				// pthread_mutex_lock(&outputLock);
				// cout << "... resource empty ..." << endl;
				// pthread_mutex_unlock(&outputLock);
			}
			resource->isFull = false;
		}
		if(resource->sizeOfList > 0){
			pthread_mutex_lock(&outputLock);
			cout << "           Size: " << resource->sizeOfList << endl;
			indexToDelete = rand() % resource->sizeOfList;
			cout << "           index: " << indexToDelete << endl;
			pthread_mutex_unlock(&outputLock);
			resource->hasDeleter = true; // add deleter so other threads stop
			pthread_mutex_lock(&outputLock);
			cout << "Deleting index: " << indexToDelete << endl;
			pthread_mutex_unlock(&outputLock);
			resource->counter++;
			//struct Node* cur = new struct Node;
			//struct Node* prev = new struct Node;
			//cur = resource->listHead;
			//for(int i = 1; i < indexToDelete; i++) {
			///	prev = cur;
			//	cur = cur->next;
			//}
			//prev->next = cur->next;

			struct Node* temp = resource->listHead;
			if(indexToDelete == 0) {
				resource->listHead = temp->next;
				free(temp);
			} else {
				for(int i = 0; temp != NULL && i < indexToDelete-1; i++) {
					temp = temp->next;
				}
				struct Node* nxt = temp->next->next;
				free(temp->next);
				temp->next = nxt;
			}
			resource->sizeOfList--;
		} else {
			if(flag == 0){
				pthread_mutex_lock(&outputLock);
				cout << "    Nothing to delete" << endl;
				pthread_mutex_unlock(&outputLock);
				flag = 1;
			}
			pthread_mutex_unlock(&deleterLock);
			continue;
		}

		sleep(SLEEPYTIME_DELETE);
		pthread_mutex_lock(&outputLock);
		cout << "Deleted index: " << indexToDelete << endl;
		pthread_mutex_unlock(&outputLock);

		struct Node* temp = new struct Node;
		temp = resource->listHead;
		pthread_mutex_lock(&outputLock);
		while(temp != NULL) {
			cout << temp->num << ", " ;
			temp = temp->next;
		}
		cout << endl;
		pthread_mutex_unlock(&outputLock);

		pthread_mutex_unlock(&deleterLock);
		flag = 0;
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
	resource.listTail = NULL;
	flag = 0;

	srand(time(NULL));

	pthread_t searcherThread[NUM_SEARCHERS];
	pthread_t deleterThread;
	pthread_t inserterThread[2];

	pthread_mutex_init(&inserterLock, NULL);
	pthread_mutex_init(&deleterLock, NULL);
	pthread_mutex_init(&outputLock, NULL);

	pthread_create(&inserterThread[0], NULL, inserter, &resource);
	pthread_create(&inserterThread[1], NULL, inserter, &resource);

	for(int i = 0; i < NUM_SEARCHERS; i++) {
		pthread_mutex_init(&searcherLock[i], NULL);
		pthread_create(&searcherThread[i], NULL, searcher, &resource);
	}

	pthread_create(&deleterThread, NULL, deleter, &resource);

	for(int i = 0; i < NUM_SEARCHERS; i++) {
		pthread_join(searcherThread[i], NULL);
	}
	pthread_join(inserterThread[0], NULL);
	pthread_join(inserterThread[1], NULL);
	pthread_join(deleterThread, NULL);

	return 0;
}
