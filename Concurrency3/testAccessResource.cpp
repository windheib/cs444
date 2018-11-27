#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include "./concurrency3.cpp"

#define NUM_SEARCHERS 3

using namespace std;

//test that accessing a resource when there are two processes is possible
int testAccessOpenResource(struct Resource resource){
	//may need to create processes here
	assert(resource.isFull == FALSE); 
	assert(resource.counter == 2);
	//asserting that the resource has two processes currently are accessing the resource
	
	//create new process
	//
}

//issues using main() & concurrency3.cpp. Maybe use header file?
/*int main(){
		struct Resource resource;

		return 0;
}*/