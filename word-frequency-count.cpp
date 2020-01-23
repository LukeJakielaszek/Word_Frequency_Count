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
#include <chrono>

using namespace std;

// output file name
const string out_file_name = "hw1_out.txt";

// struct to pass multiple items to thread
struct thread_wrapper{
	// location where thread should start processing words
	int start;

	// location where thread should stop processing words
	int end;

	// id of thread for debugging
	int thread_id;

	// a frequency map for individual threads to compose
	map<string, int> *freq_map;

	// the words obtained from input file
	vector<string> *words;
};

void* thread_count(void *params);
map<string, int>* combine_dicts(vector<map<string, int>*> *frequency_maps);
void free_vect(vector<map<string, int>*> *maps);
int write_output(const string *out_file, int num_threads, const string *in_file, double exec_time, map<string, int> freq_map);

int main(int argc, char ** argv) {
	// get the start execution time
	auto start_time = chrono::high_resolution_clock::now();

	// ensure proper command line arguments	
	if (argc != 3) {
		printf("ERROR: Invalid Number of Command Line Arguments [%d]\n", argc);
		return -1;
	}

	// get file name
	string in_file(argv[1]);

	// get number of segments
	int num_segments = atoi(argv[2]);

	if(num_segments <= 0){
		printf("ERROR: Number of segments must be positive. User input [%d]\n", num_segments);
		return -1;
	}
	
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

			try{
				// add word to vector
				words.push_back(buffer);
			}catch(bad_alloc &){
				cout << "ERROR: Memory Allocation Failure" << endl;
				return -3;
			}
		}

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
		thread_wrapper *thread_params = reinterpret_cast<thread_wrapper*>(malloc(sizeof(thread_wrapper)));

		if(thread_params == NULL){
			cout << "ERROR: Malloc Failure" << endl;
			return -4;
		}

		// store a reference to the vector of words
		thread_params->words = &words;

		// create and store a reference to the current threads dictionary
		map<string, int> *freq_map;
		
		try{
			freq_map = new map<string,int>();

			frequency_maps.push_back(freq_map);
		}catch(bad_alloc &){
			cout << "ERROR: Memory Allocation Error" << endl;
			return -2;
		}
		
		thread_params->freq_map = freq_map;

		// store the start and end location for the thread to work within the word vector
		thread_params->start = start;
		thread_params->end = end;

		// store the thread_id for debugging
		thread_params->thread_id = i;

		// create our thread
		pthread_create(&threads[i], NULL, thread_count, (void*)thread_params);
		
		// update the start location to the current end location
		start = end;
	}

	// main waits for each thread to finish
	for(int i = 0; i < num_segments; i++){
		pthread_join(threads[i], NULL);
	}

	// combine each thread's dictionary into one final version
	map<string, int> *map_sum = combine_dicts(&frequency_maps);

	// free the allocated maps within the vector
	free_vect(&frequency_maps);

	// get the end execution time
	auto end_time = chrono::high_resolution_clock::now();

	// cast the total execution time to milliseconds
	double execution_time = chrono::duration_cast<chrono::milliseconds>(end_time-start_time).count();

	// write the frequency count and other stats to file
	write_output(&out_file_name, num_segments, &in_file, execution_time, *map_sum);

	int sum = 0;

	for(auto it = map_sum->begin(); it != map_sum->end(); ++it){
		sum += it->second;
	}

	// free the final map
	delete map_sum;

	return 0;
}

// writes stats and map to an output file
int write_output(const string *out_file, int num_threads, const string *in_file, double exec_time, map<string, int> freq_map){
	// set up file writing stream
	ofstream out_stream;
	out_stream.open(*out_file);

	// attempt to write to file
	if (out_stream.is_open()) {
		// write number of threads created
		out_stream << "Number of Threads : [" << num_threads << "]" << endl;

		// write the input file name
		out_stream << "Input File Name : [" << *in_file << "]" << endl;

		// write the total execution time
		out_stream << "Execution Time (milliseconds) : [" << exec_time << "]" << endl << endl;

		// write the word frequencies
		out_stream << "Word Frequency :" << endl;

		// loop through all keys in map
		for(auto map_it = freq_map.begin(); map_it != freq_map.end(); ++map_it){
			// display the key and count
			out_stream << "\t[" << map_it->first << "] : [" << map_it->second << "]" << endl;
		}

		// close the file stream
		out_stream.close();
		return 0;
	} else {
		// error in opening file
		printf("ERROR: Failed to Open File [%s]\n", out_file->c_str());
		return -1;
	}
}

// free a vector with malloc'd indices
void free_vect(vector<map<string, int>*> *maps){
	// loop through the vector
	for(auto it = maps->begin(); it != maps->end(); ++it){
		// free each indice
		delete *it;
	}
}

// combines all maps within a vector by sum key value combinations
map<string, int>* combine_dicts(vector<map<string, int>*> *frequency_maps){
	// returnable map on the heap
	map<string, int> *ret_map;
	
	try{
		// allocate the map
		ret_map = new map<string, int>;

		// iterator for looping
		vector<map<string, int>*>::iterator vect_it;

		// for every thread's dictionary
		for(vect_it = frequency_maps->begin(); vect_it != frequency_maps->end(); ++vect_it){
			// get the current map
			map<string, int> *freq_map = *vect_it;

			// iterator for looping through keys
			map<string, int>::iterator map_iter; 

			// for every key in the dictionary
			for(map_iter = freq_map->begin(); map_iter != freq_map->end(); ++map_iter){
				// check if the word is within the returnable map
				auto iter = ret_map->find(map_iter->first);

				// if not, set value to zer0
				if(iter == ret_map->end()){
					// add the word to our dictionary, set count to zero
					ret_map->insert({map_iter->first, 0});
				}

				// increment count for current key
				(*ret_map)[map_iter->first] += map_iter->second;
			}
		}
	}catch(bad_alloc &){
		cout << "ERROR: Memory Allocation Error" << endl;
		exit(-5);
	}

	// return map with summed counts
	return ret_map;
}

// thread function to count word frequency for a subsection of text and store within map
void* thread_count(void *params){
	// unpack parameters
	thread_wrapper thread_params = *reinterpret_cast<thread_wrapper*>(params);
	int start = thread_params.start;
	int end = thread_params.end;
	vector<string> words = *thread_params.words;
	map<string, int> *freq_map = thread_params.freq_map;

	// iterate through the thread's assigned section of vector of words
	for(vector<string>::iterator iter = words.begin() + start; iter != words.begin() + end; ++iter){
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

		if(temp.compare("") == 0){
			// skip empty elements
			continue;
		}

		// increment count of word in dictionary, if not there, sets to zero
		(*freq_map)[temp]++;
	}

	// free the allocated thread parameter struct
	free(params);

	return 0;
}