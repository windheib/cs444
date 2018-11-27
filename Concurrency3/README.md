Ben Windheim, Kyle Baldes, Burton Jaursch 
Concurrency 3 ReadMe
25 November 2018

 - Compile program with "make", clean with "make clean"

 - Program runs with ./concurrency3. We suggest outputting to a file, like 
	./concurrency3 > output.txt
 - The program runs very fast and will output a lot of information. Let it run for ~20 seconds and use ^c to quit. 
 - Open output.txt. 
 - The output will show data from all three operations on the resource. It will show the beginning of the operation, how many operations are currently running, and then the results of that operation. 
 - Every 5 seconds it prints the size and contents of the list. Search for "LIST SIZE" and see that the list grows and shrinks throughout the duration
 - When there are 3 operations running at the same time the program will print "Resource at capacity", followed by 3 running threads. When the operations go back to 0, the program will print "Resource emptied" to show it is good to go again. Search for these strings to verify
 - Search for "1 running thread", "2 running threads", "3 running tthreads" to verify there are concurrent operations
 - Code has many comments and logic is pretty clear. Output combined with code should show everything you need to prove it's working as specified

For Extra Credit regarding concurrency 3, we've developed a quick test suite to showcase the combined algorithm that solves problems 1 & 2

 To develop a single algorithm, we have to ensure that all cases listed in the assignment are met.
 These cases as we have identified them are as follows (TestFileName):
 - (testOpen) A process can access the resource when less than three process are using the resource
 - (testLocked) After three processes are using the resource, the resource becomes locked and does not allow additional processes until all current processes release their use.
 - (testInserter) There can never be more than one inserter using the resource
 - (testDeleter) There can never be more than one deleter using the resource
 - (testDeletion2) When a deletion is taking place, there cannot be an active search or insertion

 To test these cases, we have created unit tests that will simulate these behaviors and assert that our algorithm is working as expected. You can run these tests using our Makefile by using the command "make test". 
