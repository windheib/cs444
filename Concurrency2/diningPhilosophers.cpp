/*********************************************************
 ** Concurrency 2 - Operating Systems II - Kyson Montague
 **	Written by Ben Windheim, Kyle Baldes, Burton Jaursch
 ** 29 October 2018
*********************************************************/

#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <iostream>

#define NUM_PHILOSOPHERS_FORKS 5
#define HUNGRY 0
#define THINKING 1
#define EATING 2
#define INIT 3

using namespace std;

string names[] = {"SOCRATES", "ARISTOTLE", "KANT", "DESCARTES", "KANYE"};

pthread_mutex_t outputLock;

struct Philosopher {
	int state;
	string name;
	pthread_t philosopherThread;
	pthread_mutex_t* leftFork; // num; these point to forks initialized in main
	pthread_mutex_t* rightFork; // (num + 1) % 5
};

/*********************************************************
 ** Function - philosopher
 **	Description - holds main logic for individual philosophers i.e. think -> sleep -> get forks and eat -> sleep -> put down forks
*********************************************************/
void* philosopher(void* ptr) {
	struct Philosopher* currentPhil = (struct Philosopher*) ptr;
	while(true) {
		// think
		if((currentPhil->state != HUNGRY && currentPhil->state == EATING) || currentPhil->state == INIT){ // if ya just ate, sleep and then think til u can eat again
			pthread_mutex_lock(&outputLock); // sync output access
			cout << currentPhil->name << " is thinking..." << endl;
			pthread_mutex_unlock(&outputLock);
			currentPhil->state = THINKING;
			sleep((rand() % 20) + 1);	// thinking random wait time, 1-20sec
		}
		// get lock on left fork
		pthread_mutex_lock(currentPhil->leftFork);
		// eat
		if(!pthread_mutex_lock(currentPhil->rightFork)) { // get lock on right fork
			pthread_mutex_lock(&outputLock);	// sync output access
			cout << currentPhil->name << " is eating!" << endl;
			pthread_mutex_unlock(&outputLock);
			currentPhil->state = EATING;
			sleep((rand() % 8) + 2);	// eating random wait time, 2-9sec
			pthread_mutex_unlock(currentPhil->leftFork);	// put down the forks
			pthread_mutex_unlock(currentPhil->rightFork);
		}	
	}
	return NULL;
}

/*********************************************************
 ** Function - main
 **	Description - initializes data structures, creates and joins threads 
*********************************************************/
int main(int argc, char* argv[]) {
	pthread_mutex_init(&outputLock, NULL);
	pthread_mutex_t forks[NUM_PHILOSOPHERS_FORKS]; // binary mutex locks for forks; lines up with left/right forks for philosophers
	struct Philosopher philosophers[NUM_PHILOSOPHERS_FORKS];

	srand(time(NULL));	// seed rand

	// initialize
	for(int i = 0; i < NUM_PHILOSOPHERS_FORKS; i++) {
		philosophers[i].name = names[i];
		philosophers[i].state = INIT;
		if(i <= NUM_PHILOSOPHERS_FORKS-2){
			pthread_mutex_init(&forks[i], NULL);
			pthread_mutex_init(&forks[i+1], NULL);
			philosophers[i].leftFork = &forks[i];
			philosophers[i].rightFork = &forks[i+1];
		} else {
			pthread_mutex_init(&forks[i], NULL);
			philosophers[i].leftFork = &forks[i];
			philosophers[i].rightFork = &forks[0];
		}
	}

	// create threads
	for(int i = 0; i < NUM_PHILOSOPHERS_FORKS; i++) {
		pthread_create(&philosophers[i].philosopherThread, NULL, philosopher, &philosophers[i]);		
	}

	// join threads
	for(int i = 0; i < NUM_PHILOSOPHERS_FORKS; i++) {
		pthread_join(philosophers[i].philosopherThread, NULL);
	}

	return 0;
}