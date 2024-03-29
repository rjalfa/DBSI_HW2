#include "common.h"

#define BF_SIZE 2000000

string prefix = "";
int datablock_start_idx = -1;
unsigned int numrecords = 0;
unsigned int unique_values = 2500;
int rowidblock_start_idx = -1;
int bitslice_start_idx = -1;
int bitslice_stride = 0;
int bitarray_start_idx = -1;
int bitarray_stride = 0;

//unsigned int datablock_start_idx = -1;

/*
 * No Index Query
 */
long long no_index_query(vector<bool> query_vector, Disk& diskInstance){
	long long sum = 0;
	for(unsigned int i = 0; i < query_vector.size(); i ++)
	{
		if(query_vector[i]) {
			Record rc = get_record(diskInstance, i, datablock_start_idx);
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
		else if(command == "rowidblock") rowidblock_start_idx = static_cast<unsigned int>(stoi(value));
		else if(command == "bitsliceblock") bitslice_start_idx = static_cast<unsigned int>(stoi(value));
		else if(command == "bitslicestride") bitslice_stride = static_cast<unsigned int>(stoi(value));
		else if(command == "bitarrayblock") bitarray_start_idx = static_cast<unsigned int>(stoi(value));
		else if(command == "bitarraystride") bitarray_stride = static_cast<unsigned int>(stoi(value));
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
	count_accesses = 0;
	long long sum = no_index_query(query_vector, diskInstance);
	cout << "No Index: Final sum was: "<< sum << " and count: "<< count_accesses << endl;

	count_accesses = 0;
	RowId row_bitmap;
	row_bitmap.initialize_existing_index(diskInstance, unique_values, rowidblock_start_idx);

	sum = row_bitmap.sumQueryRecords(query_vector);
	cout << "RowID Bitmap: Final sum was: "<< sum <<" and count: "<< count_accesses << endl;

	count_accesses = 0;
	Bitarray array_bitmap;
	array_bitmap.initialize_existing_index(diskInstance, bitarray_start_idx, bitarray_stride);

	sum = array_bitmap.sumQueryRecords(query_vector);
	cout << "Bitarray: Final sum was: "<< sum << " and count: "<< count_accesses << endl;

	count_accesses = 0;
	Bitslice bitslice;
	bitslice.initialize_existing_index(diskInstance, bitslice_start_idx, bitslice_stride);

	sum = bitslice.sumQueryRecords(query_vector);
	cout << "Bitslice: Final sum was: "<< sum << " and count: "<< count_accesses << endl;

	diskInstance.flush_cache();
	return 0;
}