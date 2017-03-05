#include <fstream>
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		cerr << "Usage: ./disk <<path_to_disk_files>> <<number_of_blocks>>" << endl;
		return 1;
	}

	string path(argv[1]);
	unsigned int num_blocks = atoi(argv[2]); 
	for(unsigned int i = 0; i < num_blocks; i ++)
	{
		ofstream out(path+to_string(i),ios::out);
		if(!out.is_open()) {
			cerr << "[ERROR]: Error creating block #" << i << endl;
			return 2;
		}
		out << "0";
		out.close();
	}

	ofstream out("CONFIG",ios::out);
	if(!out.is_open()) {
		cerr << "[ERROR]: Error creating Configuration File" << endl;
		return 3;
	}

	out << "prefix " << path << endl;
	out << "numblocks" << num_blocks << endl;

	return 0;
}