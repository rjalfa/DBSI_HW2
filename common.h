#ifndef __COMMON_H
#define __COMMON_H

#include <iostream>
#include <fstream>
#include <memory>
#include <random>
#include <string>
#include <cassert>
#include <queue>
using namespace std;

constexpr int RECORD_BLOCK_FACTOR = 300;
constexpr int BITMAP_BLOCK_FACTOR = 32000;
constexpr int ROWID_BLOCK_FACTOR = 1000;

enum BLOCK_TYPE { RECORD_BLOCK, BITMAP_BLOCK, ROWID_BITMAP_BLOCK };
/*
 * Random Generation utilities
 */

//Generates random String
string generate_random_string(unsigned int length);
//Generates random integer in range
int generate_random_int(int l, int r);


/*
 * Disk management Functions
 */
//Different kinds of blocks
class Block {
	protected:
		BLOCK_TYPE type;
		int next_block_idx;
		unsigned int blocking_factor;
	public:
		Block() : type(RECORD_BLOCK), next_block_idx(-1), blocking_factor(0) { };
		BLOCK_TYPE get_block_type() { return this->type; }
		int get_next_block_idx() { return this->next_block_idx; }
		void set_next_block_idx(int nbi) { next_block_idx = nbi; }
		virtual void serialize(const string& filename) = 0;
		virtual void load(const string& filename) = 0;
		virtual ~Block() {};
};

struct Record {
	unsigned int id;
	unsigned int amount;
	string name;
};

class RecordBlock : public Block {
	vector<Record> records;
	public:
		RecordBlock() 
		{
			Block::type = RECORD_BLOCK;
			Block::blocking_factor = RECORD_BLOCK_FACTOR;

		}
		bool add_record(const Record& record);
		void serialize(const string& filename);
		void load(const string& filename);
		vector<Record> read_records(){
			return this->records;
		}
};

class BitmapBlock : public Block {
	vector<bool> bitmap;
	public:
		BitmapBlock() 
		{
			Block::type = BITMAP_BLOCK;
			Block::blocking_factor = BITMAP_BLOCK_FACTOR;
		}
		bool add_bit(const bool& bit);
		void serialize(const string& filename);
		void load(const string& filename);
};

class RowIDBitmapBlock : public Block {
	vector<unsigned int> rowids;
	public:
		RowIDBitmapBlock()
		{
			Block::type = ROWID_BITMAP_BLOCK;
			Block::blocking_factor = ROWID_BLOCK_FACTOR;

		}
		bool add_rowid(const unsigned int& rowid);
		void serialize(const string& filename);
		void load(const string& filename);
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
			if(!config_file.is_open()) {
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
		
		string get_block_file(int block_idx) const { return prefix + to_string(block_idx); }

		//Read and Write Blocks
		void write_block(Block* block_ptr, unsigned int block_idx) {
			if(!valid_block_index(block_idx)) {
				cerr << "[ERROR] Block Index invalid" << endl;
				return;
			}
			block_ptr->serialize(get_block_file(block_idx));
		}
		Block* read_block(unsigned int block_idx);
		Block* read_block(unsigned int block_idx, BLOCK_TYPE type);
		
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

#endif