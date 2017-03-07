#include "common.h"

#define BF_SIZE 2000000

string prefix = "data/block";

long long no_index(unsigned int starting_block, vector<bool> query_vector){
	long long sum = 0;
	vector<Record> records;
	RecordBlock* block = new RecordBlock();
	do{
		block->load(prefix + to_string(starting_block));
		records = block->read_records();
		for(auto record : records){
			if(query_vector[record.id]){
				sum += record.amount;
			}
		}
		starting_block = block->get_next_block_idx();
		// delete records;
		cout << "Next up: " << block->get_next_block_idx() << endl;
	}
	while(block->get_next_block_idx()!=-1);
	delete block;
	return sum;
}

unsigned int get_start(const string config_file_name){
	ifstream config_file(config_file_name, ios :: in);
	if(!config_file.is_open()) {
		cerr << "[ERROR] CONFIG File cannot be opened" << endl;
		return static_cast<unsigned int>(-1);
	}
	while(!config_file.eof()) {
		string command, value;
		config_file >> command >> value;
		if(command == "datablock"){
			return static_cast<unsigned int>(stoi(value));
		}
	}
}

int main()
{
	vector<bool> query_vector(BF_SIZE, 0);
	int n, temp;
	cin>>n;
	for(int i=0;i<n;++i){
		cin>>temp;
		query_vector[temp] = true;
	}
	unsigned int starting_block = get_start("CONFIG");
	long long sum = no_index(starting_block, query_vector);
	cout << "Final sum was: "<< sum << endl;
	return 0;
}