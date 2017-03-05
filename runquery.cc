#include <iostream>
#include <fstream>
#include <memory>
#include <random>
#include <string>
#include <queue>
using namespace std;

const int data_blocking_factor = 300;
const string disk_prefix = "data/block";
/*
 * Random Generation utilities
 */
default_random_engine generator;

//Generates random String
string generate_random_string(unsigned int length) {
	//Return empty string if length = 0
	if(length == 0) return "";

	//ret = Return value
	string ret = "";

	//create distribution, only have a-z latin chars
	uniform_int_distribution<int> distribution('a','z');

	//append characters to return value
	for(unsigned int i = 0; i < length; i ++) ret += static_cast<char>(distribution(generator));

	//Return the value
	return ret;
}

//Generates random integer in range
int generate_random_int(int l, int r) {
	if(l <= r)
	{
		uniform_int_distribution<int> distribution(l,r);
		return distribution(generator);
	}
	else return 0;
}


/*
 * Disk management Functions
 */
//Different kinds of blocks
class Block {
	public:
		int block_type;
		virtual void serialize(const string& filename) = 0;
		virtual void load(const string& filename) = 0;
		virtual ~Block() = 0;
};

class RecordBlock : public Block {

};

class BitmapBlock : public Block {

};

class RowIDBitmapBlock : public Block {

};

//Disk Handler Class
class Disk
{
	private:
		string prefix;
		unsigned int numblocks;
		queue<unsigned int> free_blocks; 

	public:
		//Parametric Constructor
		Disk(const string config_file_name) {
			ifstream config_file(config_file_name, ios :: in);
			if(config_file.is_open()) {
				cerr << "[ERROR] CONFIG File cannot be opened" << endl;
				return;
			}

			//Read key-value pairs from CONFIG and set accordingly
			while(!config_file.eof()) {
				string command, value;
				config_file >> command >> value;
				if(command == "prefix") prefix = value;
				else if(command == "numblocks") numblocks = static_cast<unsigned int>(stoi(value));
			}

			config_file.close();

			//Assuming empty blocks
			for(unsigned int i = 0; i < numblocks; i ++) free_blocks.push(i);
		}
		
		//Checks if record is a valid 
		bool valid_block_index(int block_idx) { return  block_idx >= 0 && static_cast<unsigned int>(block_idx) < numblocks; }
		
		//Read and Write Blocks
		void write_block(unique_ptr<Block> block_ptr, unsigned int block_idx) {
			if(!valid_block_index(block_idx))
			block_ptr->serialize(prefix + to_string(block_idx));
		}
		unique_ptr<Block> read_block(unsigned int block_idx);
		
		//Get free block index, block is marked as used now
		int get_free_block_idx() {
			if(free_blocks.empty()) return -1;
			unsigned int block_idx = free_blocks.front();
			free_blocks.pop();
			return block_idx;
		}
		
		//Free the block with addr block_idx. Requires one write
		bool free_block(unsigned int block_idx)	{
			if(valid_block_index(block_idx)) {
				free_blocks.push(block_idx);
				return true;
			}
			else return false;
		}
};

int main()
{
	//Prompt based I/O
	return 0;
}