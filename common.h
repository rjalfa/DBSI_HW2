#ifndef __COMMON_H
#define __COMMON_H

#include <iostream>
#include <fstream>
#include <memory>
#include <random>
#include <string>
#include <cassert>
#include <queue>
#include <unordered_map>
#include <map>
using namespace std;

constexpr int RECORD_BLOCK_FACTOR = 300;
constexpr int BITMAP_BLOCK_FACTOR = 32000;
constexpr int ROWID_BLOCK_FACTOR = 1000;
constexpr unsigned int CACHE_SIZE = 2550;
constexpr unsigned int MAX_VALUE = 2500;
constexpr unsigned int BITSLICE_BITS = 12;
constexpr unsigned int CACHE_FLUSH_GUARD_VALUE = 4000;
extern long long count_accesses;
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
		Record get_record(const unsigned int& idx) { return records[idx]; }
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
		vector<bool> read_bitmap(){
			return this->bitmap;
		}
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
		vector<unsigned int> read_rowids(){
			return this->rowids;
		}
};

//Disk Handler Class
class Disk
{
	private:
		string prefix;
		unsigned int numblocks;
		queue<unsigned int> free_blocks;
		unordered_map<unsigned int, pair<Block*,bool> > index_vector;
		unsigned int cache_size;
	public:
		//Parametric Constructor
		Disk(const string config_file_name);
		
		//Checks if record is a valid 
		bool valid_block_index(int block_idx) { return  block_idx >= 0 && static_cast<unsigned int>(block_idx) < numblocks; }
		
		string get_block_file(int block_idx) const { return prefix + to_string(block_idx); }

		//Read and Write Blocks
		void write_block(Block* block_ptr, unsigned int block_idx);
		Block* read_block(unsigned int block_idx);
		void flush_cache();
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

unsigned int generate_bitmap(unsigned int num_records, vector<bool>& bitmap, Disk& diskInstance);
Record get_record(Disk& diskInstance, unsigned int i, unsigned int datablock_start_idx);

class Index
{
protected:
	map<unsigned int, int> secondary_index;
	Disk* disk_ref = nullptr;
public:
	int getSecondaryEntry(int a)
	{
		return this->secondary_index[a];
	}
	void setSecondaryEntry(int a, int x)
	{
		this->secondary_index[a] = x;
	}
	virtual void initialize_index(Disk& diskInstance, unsigned int num_bitmaps, unsigned int bitmap_size) = 0;
	virtual void addRecordToIndex(const Record& r) = 0;
	Disk* get_disk_ref(){ return this->disk_ref;}
	void set_disk_ref(Disk* ref){ this->disk_ref = ref;}
};

class Bitmap: public Index {
public:
	// virtual void initialize_index(Disk& diskInstance, unsigned int num_bitmaps, unsigned int bitmap_size) = 0;	
};

class RowId: public Bitmap {

public:
	void addRecordToIndex(const Record& r);
	void initialize_index(Disk& diskInstance, unsigned int num_bitmaps, unsigned int bitmap_size);
	void insertIntoBitmap(unsigned int bitmap_index, unsigned int data);
	long long sumQueryRecords(vector<bool> bfr);
	void constructIndex(unsigned int num_records, unsigned int datablock_start_idx);
	void initialize_existing_index(Disk& diskInstance, unsigned int num_records, unsigned int rowidblock_start_idx);
};

class Bitarray: public Bitmap {
	
public:
	void addRecordToIndex(const Record& r);
	void initialize_index(Disk& diskInstance, unsigned int num_bitmaps, unsigned int bitmap_size);
	void constructIndex(unsigned int num_records, unsigned int datablock_start_idx);
	void initialize_existing_index(Disk& diskInstance, unsigned int rowidblock_start_idx, unsigned int stride);
	long long sumQueryRecords(vector<bool> bfr);
};


class Bitslice: public Index {
	int index_size;
	bool position_set(int n, int position){
		return (1&(n>>(position)));
	}
	bool generateBitIndex(Disk& diskInstance, int bit_position);
public:
	Bitslice() { };
	void addRecordToIndex(const Record& r);
	void initialize_index(Disk& diskInstance, unsigned int num_bitmaps, unsigned int bitmap_size);
	void constructIndex(unsigned int num_records, unsigned int datablock_start_idx);
	void initialize_existing_index(Disk& diskInstance, unsigned int rowidblock_start_idx, unsigned int stride);
	long long sumQueryRecords(vector<bool> bfr);
};

#endif