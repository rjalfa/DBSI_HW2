#include "common.h"

#define BF_SIZE 2000000

string prefix = "";
unsigned int datablock_start_idx = -1;
unsigned int numrecords = 0;

Record get_record(Disk& diskInstance, unsigned int i)
{
	unsigned int addr = datablock_start_idx + i / RECORD_BLOCK_FACTOR;
	Block* blk = diskInstance.read_block(addr);
	return (static_cast<RecordBlock*>(blk))->get_record(i % RECORD_BLOCK_FACTOR);
}

/*
 * No Index Query
 */
long long no_index_query(vector<bool> query_vector, Disk& diskInstance){
	long long sum = 0;
	
	for(unsigned int i = 0; i < query_vector.size(); i ++)
	{
		if(query_vector[i]) {
			Record rc = get_record(diskInstance, i);
			sum += rc.amount;
		}
	}
	return sum;
}

void load_configuration(const string config_file_name){
	ifstream config_file(config_file_name, ios :: in);
	if(!config_file.is_open()) {
		cerr << "[ERROR] CONFIG File cannot be opened" << endl;
		return;
	}
	while(!config_file.eof()) {
		string command, value;
		config_file >> command >> value;
		if(command == "prefix") prefix = value;
		else if(command == "datablock") datablock_start_idx = static_cast<unsigned int>(stoi(value));
		else if(command == "numrecords") numrecords = static_cast<unsigned int>(stoi(value));
	}
	config_file.close();
}

int main()
{
	Disk diskInstance("CONFIG");
	load_configuration("CONFIG");
	
	vector<bool> query_vector(BF_SIZE, 0);
	int n, temp;
	cin>>n;
	for(int i=0;i<n;++i) {
		cin>>temp;
		query_vector[temp] = true;
	} 
	long long sum = no_index_query(query_vector, diskInstance);
	cout << "Final sum was: "<< sum << endl;
	diskInstance.flush_cache();
	return 0;
}