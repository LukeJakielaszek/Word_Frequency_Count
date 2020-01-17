// Luke Jakielaszek, 915377673, HW1, CIS 4307, Spring 2020
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

using namespace std;

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

	// buffer for reading
	char buffer[100];

	// vector of read words
	vector<string> words;


	// attempt to read file
	if (input_stream.is_open()) {
		// number of words
		int count = 0;

		// read each word
		while (!input_stream.eof()) {
			// read a word
			input_stream >> buffer;

			// add word to vector
			words.push_back(buffer);

			// increment count
			count++;
		}

		// display number of words read
		printf("Read %d words\n", count);

		// close the file stream
		input_stream.close();
	} else {
		// error in opening file
		printf("ERROR: Failed to Open File [%s]\n", in_file.c_str());
		return -1;
	}

	// display contents of vector
	for (string word : words){
		cout << word;
		cout << " ";
	}

	cout << "\n";
	return 0;
}