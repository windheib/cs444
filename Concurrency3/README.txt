For Extra Credit regarding concurrency 3, we've developed a quick test suite to showcase the combined algorithm that solves problems 1 & 2

To develop a single algorithm, we have to ensure that all cases listed in the assignment are met.
These cases as we have identified them are as follows:
- A process can access the resource when less than three process are using the resource
- After three processes are using the resource, the resource becomes locked and does not allow additional processes until all current processes release their use.
- There can never be more than one inserter using the resource
- There can never be more than one deleter using the resource
- When a deletion is taking place, there cannot be an active search or insertion

To test these cases, we have created unit tests that will simulate these behaviors and assert that our algorithm is working as expected. You can run these tests using our Makefile by using the command "make test".
