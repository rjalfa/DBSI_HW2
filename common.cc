#include "common.h"

default_random_engine generator;

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

//Implementations
bool RecordBlock::add_record(const Record& record)
{
	if(records.size() == blocking_factor) return false;
	records.push_back(record);
	return true;
}

bool BitmapBlock::add_bit(const bool& bit)
{
	if(bitmap.size() == blocking_factor) return false;
	bitmap.push_back(bit);
	return true;
}

bool RowIDBitmapBlock::add_rowid(const unsigned int& rowid)
{
	if(rowids.size() == blocking_factor) return false;
	rowids.push_back(rowid);
	return true;
}

// bool BitmapBlock::set_bitmap(const vector<bool>& p_bitmap)
// {
// 	if(p_bitmap.size() != blocking_factor) return false;
// 	bitmap.clear();
// 	copy(p_bitmap.begin(), p_bitmap.end(), bitmap.begin());
// 	return true;
// }

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

Block* Disk::read_block(unsigned int block_idx)
{
	if(this->index_vector.find(block_idx) != this->index_vector.end()){
		return this->index_vector[block_idx];
	}
	else{
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

		if(index_vector.size() == cache_size){
			(*index_vector.begin()).second->serialize(get_block_file((*index_vector.begin()).first));
			delete (*index_vector.begin()).second;
			index_vector.erase(index_vector.begin()); 
		}
		index_vector[block_idx] = ret;
		return ret;
	}
}

void Disk::write_block(Block* block_ptr, unsigned int block_idx) {
	if(!valid_block_index(block_idx)) {
		cerr << "[ERROR] Block Index invalid" << endl;
		return;
	}
	if(this->index_vector.find(block_idx) != this->index_vector.end()){
		this->index_vector[block_idx] = block_ptr;
	}
	else {
		if(index_vector.size() == cache_size){
			(*index_vector.begin()).second->serialize(get_block_file((*index_vector.begin()).first));
			delete (*index_vector.begin()).second;
			index_vector.erase(index_vector.begin()); 
		}
		index_vector[block_idx] = block_ptr;
	}
	//block_ptr->serialize(get_block_file(block_idx));
}

void Disk::flush_cache()
{
	for(auto it : index_vector) {
		(it.second)->serialize(get_block_file(it.first));
		delete it.second;
	}
	index_vector.clear();
}

void RowId::initialize_index(Disk& diskInstance, unsigned int num_bitmaps, unsigned int bitmap_size){
	for(unsigned int i=0;i<num_bitmaps;i++){
		int index = diskInstance.get_free_block_idx();
		Block* temp = static_cast<Block*>(new RowIDBitmapBlock);
		diskInstance.write_block(temp, index);
		this->setSecondaryEntry(i, index);
		delete temp;
	}
}

void Bitarray::initialize_index(Disk& diskInstance, unsigned int num_bitmaps, unsigned int bitmap_size){
	for(unsigned int i=0;i<num_bitmaps;i++){
		unsigned int first_pointer = generate_bitmap(bitmap_size, diskInstance);
		this->setSecondaryEntry(i, first_pointer);
	}
}

void Bitslice::initialize_index(Disk& diskInstance, unsigned int num_bitmaps, unsigned int bitmap_size){
	for(unsigned int i=0;i<num_bitmaps;i++){
		unsigned int first_pointer = generate_bitmap(bitmap_size, diskInstance);
		this->setSecondaryEntry(i, first_pointer);
	}
}

unsigned int generate_bitmap(unsigned int num_records, Disk& diskInstance)
{
	unsigned int counter = 0;
	int first_block_idx = -1;
	int new_block_idx = -1;
	int prev_block_idx = -1;
	BitmapBlock* blk = nullptr;
	while(num_records > 0)
	{
		//Request a free block
		new_block_idx = diskInstance.get_free_block_idx();
		if(first_block_idx == -1) first_block_idx = new_block_idx;
		if(blk != nullptr) {
			blk->set_next_block_idx(new_block_idx);
			//Write the block
			diskInstance.write_block(static_cast<Block*>(blk), prev_block_idx);
			delete blk;
		}
		blk = new BitmapBlock;
		while(true)
		{
			//Generate a random record
			bool bit = false;
			bool added = blk->add_bit(bit);
			if(added) {
				num_records --;
				if(num_records == 0) break;
			}
			else {
				counter --;
				break;
			}
		}
		prev_block_idx = new_block_idx;
	}
	if(blk != nullptr) {
		//Write the block
		diskInstance.write_block(static_cast<Block*>(blk), new_block_idx);	
		delete blk;
	}
	return static_cast<unsigned int>(first_block_idx);
}

void RowId::insertIntoBitmap(unsigned int bitmap_index, unsigned int data){
	unsigned int add = this->getSecondaryEntry(bitmap_index);
	RowIDBitmapBlock* block = nullptr;
	bool found = false;
	do
	{
		if(block!=nullptr) delete block;
		block = static_cast<RowIDBitmapBlock*>(this->disk_ref->read_block(add));
		if(block->get_next_block_idx()==-1){
			found = true;
		}
		else{
			add = block->get_next_block_idx();
		}
	}while(!found);
	if(block->add_rowid(data)){
		this->disk_ref->write_block(static_cast<Block*>(block), add);
		delete block;
	}
	else{
		unsigned int new_addr = this->disk_ref->get_free_block_idx();
		block->set_next_block_idx(new_addr);
		this->disk_ref->write_block(static_cast<Block*>(block), add);
		delete block;
		block = new RowIDBitmapBlock;
		if(block->add_rowid(data)){
			this->disk_ref->write_block(static_cast<Block*>(block), add);
			delete block;
		}
		else{
			if(block != nullptr) delete block;
			cerr << "[ERROR] Fatal error!" << endl;
		}
	}
}

void RowId::addRecordToIndex(const Record& r){
	this->insertIntoBitmap(r.amount, r.id);
}

void Bitarray::addRecordToIndex(const Record& r){
		
}

void Bitslice::addRecordToIndex(const Record& r){
	
}
