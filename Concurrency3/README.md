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
