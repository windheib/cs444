// Dining Philosophers

#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <iostream>

#define NUM_PHILOSOPHERS_FORKS 5
#define HUNGRY 0
#define THINKING 1
#define EATING 2

using namespace std;

string names[] = {"SOCRATES", "ARISTOTLE", "KANT", "DESCARTES", "KANYE"};

struct Philosopher {
	int id;
	int state;
	string name;
	pthread_t philosopherThread;
	pthread_mutex_t* leftFork; // num; these point to forks initialized in main
	pthread_mutex_t* rightFork; // (num + 1) % 4
};

void* philosopher(void* ptr) {
	// cout << "Philosopher thread" << endl;
	struct Philosopher* currentPhil = (struct Philosopher*) ptr;
	while(true) {
		// think
		if(currentPhil->state != HUNGRY && currentPhil->state == EATING){ // if ya just ate, sleep and then think til u can eat again
			cout << currentPhil->name << " is thinking..." << endl;
			currentPhil->state = THINKING;
			sleep((rand() % 20) + 1);	
		}
		// get lock 
		pthread_mutex_lock(currentPhil->leftFork);
		// eat
		if(!pthread_mutex_lock(currentPhil->rightFork)) {
			cout << currentPhil->name << " is eating!" << endl;
			currentPhil->state = EATING;
			sleep((rand() % 8) + 2);
			pthread_mutex_unlock(currentPhil->leftFork);
			pthread_mutex_unlock(currentPhil->rightFork);
		}	
	}
	return NULL;
}


int main(int argc, char* argv[]) {
	pthread_mutex_t forks[NUM_PHILOSOPHERS_FORKS]; // binary mutex locks for forks; lines up with left/right forks for philosophers
	struct Philosopher philosophers[NUM_PHILOSOPHERS_FORKS];

	srand(time(NULL));

	// initialize
	for(int i = 0; i < NUM_PHILOSOPHERS_FORKS; i++) {
		philosophers[i].name = names[i];
		philosophers[i].id = i;
		philosophers[i].state = EATING;
		if(i <= NUM_PHILOSOPHERS_FORKS-2){
			pthread_mutex_init(&forks[i], NULL);
			pthread_mutex_init(&forks[i+1], NULL);
			philosophers[i].leftFork = &forks[i];
			cout << philosophers[i].name << " is getting left fork " << i << endl;
			philosophers[i].rightFork = &forks[i+1];
			cout << philosophers[i].name << " is getting right fork " << i+1 << endl;
		} else {
			pthread_mutex_init(&forks[i], NULL);
			philosophers[i].leftFork = &forks[i];
			cout << philosophers[i].name << " is getting left fork " << i << endl;	
			philosophers[i].rightFork = &forks[0];
			cout << philosophers[i].name << " is getting right fork " << 0 << endl;			
		}
	}
	for(int i = 0; i < NUM_PHILOSOPHERS_FORKS; i++) {
		pthread_create(&philosophers[i].philosopherThread, NULL, philosopher, &philosophers[i]);		
	}

	// join
	for(int i = 0; i < NUM_PHILOSOPHERS_FORKS; i++) {
		pthread_join(philosophers[i].philosopherThread, NULL);
	}

	return 0;
}