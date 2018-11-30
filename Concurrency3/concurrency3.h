#ifndef CONCURRENCY3_H
#define CONCURRENCY3_H

#include <iostream>
#include <cstdlib>
#include <unistd.h>
//#include <pthread.h>
#include <time.h>

#define MAX_NUM 100
#define NUM_THREADS 3
#define PRINT_INTERVAL 2


struct Node {
	int num;
	struct Node *next;
};

struct State {
	int counter;	// keeps track of how many operators are using the resource
	bool hasDeleter;// keeps track if there's a deleter running
	bool hasInserter;
	bool reset;		// true if resource at capacity, false when emptied and < 3
};

struct Resource {
	struct Node* listHead;
	struct Node* listTail;
	int sizeOfList;
};

pthread_mutex_t resourceLock;
pthread_mutex_t stateLock;

struct State state;

void randomSleep();
void lockState();
void unlockState();
void lockResource();
void unlockResource();
void printList(struct Resource* resource);
void insert(struct Resource* resource, int numToInsert);
void search(struct Resource* resource, int numToSearch);
void remove(struct Resource* resource, int indexToDelete);
void* printer(void* ptr);
void* searcher(void* ptr);
void* inserter(void* ptr);
void* deleter(void* ptr);

#endif /* CONCURRENCY3_H */
