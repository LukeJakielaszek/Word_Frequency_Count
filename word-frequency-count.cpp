// Luke Jakielaszek, 915377673, HW1, CIS 4307, Spring 2020
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <pthread.h>
#include <iterator>
#include <algorithm>
#include <ctype.h>
#include <map>

using namespace std;

struct thread_wrapper{
	int start;
	int end;
	int thread_id;
	map<string, int> *freq_map;
	vector<string> *words;
};

void* thread_count(void *params);

int main(int argc, char ** argv) {
	// ensure proper command line arguments	
	if (argc != 3) {
		printf("ERROR: Invalid Number of Command Line Arguments [%d]\n", argc);
		return -1;
	}

	// get number of segments
	int num_segments = atoi(argv[2]);

	// get file name
	string in_file(argv[1]);

	// display both
	printf("Splitting [%s] into %d segments\n", in_file.c_str(), num_segments);
	
	// set up file reading stream
	ifstream input_stream;
	input_stream.open(in_file);

	// vector of read words
	vector<string> words;

	// attempt to read file
	if (input_stream.is_open()) {
		// buffer for reading
		char buffer[100];

		// read each word
		while (!input_stream.eof()) {
			// read a word
			input_stream >> buffer;

			// add word to vector
			words.push_back(buffer);
		}

		// display number of words read
		printf("Read %lu words\n", words.size());

		// close the file stream
		input_stream.close();
	} else {
		// error in opening file
		printf("ERROR: Failed to Open File [%s]\n", in_file.c_str());
		return -1;
	}

	// start location for each thread
	int start = 0;
	
	// end location for each thread
	int end = 0;

	// number of words each thread should process
	int delta = words.size() / num_segments;

	// our worker threads
	pthread_t threads[num_segments];

	// per thread dictionaries to count word frequency
	vector<map<string, int>*> frequency_maps;

	for(int i = 0; i < num_segments; i++){
		// update our end location
		if(i == num_segments - 1){
			// if final segment, go all the way to the end of our vector of words
			end = words.size()-1;
		}else{
			// increment end by the calculated delta to ensure evenness of lead on each thread
			end += delta;
		}

		// declare a wrapper struct to pass multiple items to thread
		thread_wrapper thread_params;

		// store a reference to the vector of words
		thread_params.words = &words;

		// create and store a reference to the current threads dictionary
		map<string, int> freq_map;
		frequency_maps.push_back(&freq_map);
		thread_params.freq_map = &freq_map;

		// store the start and end location for the thread to work within the word vector
		thread_params.start = start;
		thread_params.end = end;

		// store the thread_id for debugging
		thread_params.thread_id = i;

		// create our thread
		pthread_create(&threads[i], NULL, thread_count, (void*)&thread_params);
		
		// update the start location to the current end location
		start = end;
	}

	// main waits for each thread to finish
	for(int i = 0; i < num_segments; i++){
		pthread_join(threads[i], NULL);
	}

	cout << "DONE\n";

	return 0;
}

void* thread_count(void *params){
	// unpack parameters
	thread_wrapper thread_params = *reinterpret_cast<thread_wrapper*>(params);
	int start = thread_params.start;
	int end = thread_params.end;
	vector<string> words = *thread_params.words;
	map<string, int> freq_map = *thread_params.freq_map;

	// create an iterator at the start of the current thread's section
	vector<string>::iterator iter = words.begin() + start;

	// iterate through the thread's assigned section of vector of words
	for(int i = start; i < end; i++){
		// grab the current word
		string word = (string)*iter;

		// preprocessed word
		string temp = "";

		// remove non alphabetical characters and convert to uppercase
		for(char c : word){
			if(isalpha(c)){
				c = toupper(c);
				temp += c;
			}
		}
		
		// adjust iterator to next location
		iter = next(iter, 1);
	}

	printf("Start %d, end %d, delta %d\n", start, end, end-start);
	
	return 0;
}