# Word-Frequency-Count

Compile:

make -B

Run:

./word-frequency-count in_file.txt number_of_threads

Design:
The program first tracks the initial start time. Next, the program reads the entire input 
file word by word into a vector "words". Next, the program calculates frames of the words 
vector for each thread to work within. This is done by dividing the total number of words 
by the number of threads. The program allocates a per-thread map to store each thread's 
frequency counts. Each map is tracked within main by pushing a reference to the map on the 
vector "frequency_maps". The neccessary information for each thread is packed within a struct, 
including a pointer to the origional list of words, a pointer to the per thread map, the 
start and  end of the threads frame, and the thread's id for debugging. Throughout this program,
I chose to pass large data structures (maps and vectors) by reference rather than value
to save computation time. The threads are then launched with their corresponding references.
Each thread loops through its frame of the word vector and preprocesses each word before
incrementing the thread-level frequency count. To preprocess, all punctuation is removed
and the word is converted to upper case. The main thread waits for all threads to complete
processing their individual frames and maps. Upon completion, the main thread loops through
the vector "frequency_maps" and combines all maps together into "map_sum" ensuring to sum
each overlapping key's count. The allocated thread-level frequency maps are freed to
ensure no memory leak occurs. The end time is then taken and the execution time is calculated
in milliseconds. The final word frequency count and tracked statistics are then written to an
output file "hw1_out.txt". Finally, the allocated map_sum frequency map is freed to
ensure no memory leak occurs.