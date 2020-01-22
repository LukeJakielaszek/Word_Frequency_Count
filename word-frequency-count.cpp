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
map<string, int>* combine_dicts(vector<map<string, int>*> *frequency_maps);
void display_map(map<string, int> map_a);
void free_vect(vector<map<string, int>*> *maps);

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
		map<string, int> *freq_map = new map<string,int>();

		frequency_maps.push_back(freq_map);
		thread_params.freq_map = freq_map;

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

	map<string, int> *map_sum = combine_dicts(&frequency_maps);

	// free the allocated maps within the vector
	free_vect(&frequency_maps);

	display_map(*map_sum);

	return 0;
}

void free_vect(vector<map<string, int>*> *maps){
	for(vector<map<string,int>*>::iterator it = maps->begin(); it != maps->end(); ++it){
		map<string,int> * map_a = *it;

		delete map_a;
	}
}

void display_map(map<string, int> map_a){
	for(map<string, int>::iterator iter = map_a.begin(); iter != map_a.end(); ++iter){
		cout << "[" << iter->first << "] : [" << iter->second << "]" << endl;
	}

	cout << "Size " << map_a.size() << endl;
}

map<string, int>* combine_dicts(vector<map<string, int>*> *frequency_maps){
	cout << "Combining dictionaries" << endl;

	// allocate a map on the heap
	map<string, int> *ret_map = new map<string, int>;

	// iterator for looping
	vector<map<string, int>*>::iterator vect_it;

	// for every thread's dictionary
	for(vect_it = frequency_maps->begin(); vect_it != frequency_maps->end(); ++vect_it){
		cout << "looping through maps" << endl;

		// get the current map
		map<string, int> *freq_map = *vect_it;

		// iterator for looping through keys
		map<string, int>::iterator map_iter; 

		// for every key in the dictionary
		for(map_iter = freq_map->begin(); map_iter != freq_map->end(); ++map_iter){
			cout << "checking word " << map_iter->first << endl;

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

	// return map with summed counts
	return ret_map;
}

void* thread_count(void *params){
	// unpack parameters
	thread_wrapper thread_params = *reinterpret_cast<thread_wrapper*>(params);
	int start = thread_params.start;
	int end = thread_params.end;
	vector<string> words = *thread_params.words;
	map<string, int> *freq_map = thread_params.freq_map;

	// iterate through the thread's assigned section of vector of words
	for(vector<string>::iterator iter = words.begin() + start; iter != words.begin() + end; ++ iter){
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
		
		// check if word is in dictionary
		auto map_iter = freq_map->find(temp);

		if(map_iter != freq_map->end()){
			// increment our words count
			map_iter->second++;
		}else{
			// add the word to our dictionary, set count to zero
			freq_map->insert({temp, 1});
		}
	}

	return 0;
}