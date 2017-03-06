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
	protected:
		BLOCK_TYPE type;
		int next_block_idx = -1;
		unsigned int blocking_factor;
	public:
		BLOCK_TYPE get_block_type() { return this->type; }
		int get_next_block_idx() { return this->next_block_idx; }
		void set_next_block_idx(decltype(next_block_idx) nbi) { next_block_idx = nbi; }
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
};

class BitmapBlock : public Block {
	vector<bool> bitmap;
	public:
		BitmapBlock() 
		{
			Block::type = BITMAP_BLOCK;
			Block::blocking_factor = BITMAP_BLOCK_FACTOR;
		}
		bool set_bitmap(const vector<bool>& bitarray);
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

//Implementations
bool RecordBlock::add_record(const Record& record)
{
	if(records.size() == blocking_factor) return false;
	records.push_back(record);
	return true;
}

bool BitmapBlock::set_bitmap(const vector<bool>& p_bitmap)
{
	if(p_bitmap.size() != blocking_factor) return false;
	bitmap.clear();
	copy(p_bitmap.begin(), p_bitmap.end(), bitmap.begin());
	return true;
}

void RecordBlock::serialize(const string& filename)
{
	ofstream out(filename, ios::out);
	if(!out.is_open()) { cerr << "[ERROR] Block Write Error" << endl;return; }
	out << type << " " << next_block_idx << " " << records.size() <<  endl;
	for(auto record : records) out << record.id << " " << record.amount << " " << record.name << endl;
	out.close();
}

void BitmapBlock::serialize(const string& filename)
{
	ofstream out(filename, ios::out);
	if(!out.is_open()) { cerr << "[ERROR] Block Write Error" << endl;return; }
	out << type << " " << next_block_idx << " " << bitmap.size() <<  endl;
	for(auto bit : bitmap) out << bit;
	out.close();
}

void RowIDBitmapBlock::serialize(const string& filename)
{
	ofstream out(filename, ios::out);
	if(!out.is_open()) { cerr << "[ERROR] Block Write Error" << endl;return; }
	out << type << " " << next_block_idx << " " << rowids.size() <<  endl;
	for(auto rowid : rowids) out << rowid << " ";
	out.close();
}

void RecordBlock::load(const string& filename)
{
	ifstream in(filename, ios::in);
	if(!in.is_open()) { cerr << "[ERROR] Block Read Error" << endl;return; }
	
	int temp, n;
	in >> temp >> next_block_idx >> n;
	assert(temp == type);
	for(int i = 0; i < n; i ++)
	{
		Record r;
		in >> r.id >> r.amount >> r.name;
		records.push_back(r);
	}
	in.close();
}

void BitmapBlock::load(const string& filename)
{
	ifstream in(filename, ios::in);
	if(!in.is_open()) { cerr << "[ERROR] Block Read Error" << endl;return; }
	
	int temp, n;
	string bitmap_str;
	in >> temp >> next_block_idx >> n;
	in >> bitmap_str;
	for(int i = 0; i < n; i ++) bitmap.push_back(bitmap_str[i] == '1');
	in.close();
}

void RowIDBitmapBlock::load(const string& filename)
{
	ifstream in(filename, ios::in);
	if(!in.is_open()) { cerr << "[ERROR] Block Read Error" << endl;return; }
	
	int temp, n;
	in >> temp >> next_block_idx >> n;
	for(int i = 0; i < n ; i ++)
	{
		decltype(rowids)::value_type x;
		in >> x;
		rowids.push_back(x);
	}
	in.close();
}


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
		
		string get_block_file(int block_idx) const { return prefix + to_string(block_idx); }

		//Read and Write Blocks
		void write_block(Block* block_ptr, unsigned int block_idx) {
			if(!valid_block_index(block_idx))
			block_ptr->serialize(get_block_file(block_idx));
		}
		Block* read_block(unsigned int block_idx);
		
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

Block* Disk::read_block(unsigned int block_idx)
{
	//If not a valid block, return null
	if(!valid_block_index(block_idx)) return nullptr;

	//Open file
	ifstream ifs(get_block_file(block_idx));
	if(!ifs.is_open()) return nullptr;

	int blk_type;
	ifs >> blk_type;
	ifs.close();

	//Pointer to Block object
	Block* ret = nullptr;
	switch(blk_type)
	{
		case RECORD_BLOCK:
			ret = new RecordBlock;break;
		case BITMAP_BLOCK:
			ret = new BitmapBlock;break;
		case ROWID_BITMAP_BLOCK:
			ret = new RowIDBitmapBlock;break;
	}
	ret->load(get_block_file(block_idx));
	return ret;
}

void generate_table(unsigned int num_records, Disk& diskInstance)
{
	unsigned int counter = 0;
	RecordBlock* blk = nullptr;
	while(num_records > 0)
	{
		//Request a free block
		int new_block_idx = diskInstance.get_free_block_idx();
		if(blk != nullptr) blk->set_next_block_idx(new_block_idx);
		while(true)
		{
			//Generate a random record
			Record r;
			r.id = counter ++;
			r.amount = generate_random_int(0,2500);
			r.name = generate_random_string(3);
			bool added = blk->add_record(r);
			if(added) {
				num_records --;
				if(num_records == 0) break;
			}
			else break;
		}
		//Write the block
		diskInstance.write_block(static_cast<Block*>(blk), new_block_idx);
	}
}

int main()
{
	//Start Disk
	Disk diskInstance("CONFIG");

	//Prompt based I/O
	return 0;
}