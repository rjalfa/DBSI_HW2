#include "common.h"

string prefix = "";
unsigned int datablock_start_idx = -1;
unsigned int numrecords = 0;
unsigned int unique_values = 2500;

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

int main(){
	
	Disk diskInstance("CONFIG");
	load_configuration("CONFIG");

	//RowId row_bitmap;
	//row_bitmap.set_disk_ref(&diskInstance);
	//row_bitmap.initialize_index(diskInstance, unique_values, -1);
	//row_bitmap.constructIndex(numrecords, datablock_start_idx);

	Bitslice bs;
	bs.set_disk_ref(&diskInstance);
	bs.constructIndex(numrecords, datablock_start_idx);

	diskInstance.flush_cache();
	return 0;
}
