#include "common.h"

default_random_engine generator;

long long count_accesses = 0;

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
	if(!(temp == type)){
		cerr << filename << endl;
		assert(0);
	}
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
		return this->index_vector[block_idx].first;
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
		count_accesses ++;
		if(index_vector.size() == cache_size){
			if((*index_vector.begin()).second.second) (*index_vector.begin()).second.first->serialize(get_block_file((*index_vector.begin()).first));
			delete (*index_vector.begin()).second.first;
			index_vector.erase(index_vector.begin()); 
		}
		index_vector[block_idx] = make_pair(ret,false);
		return ret;
	}
}

void Disk::write_block(Block* block_ptr, unsigned int block_idx) {
	static unsigned int cnt = 0;

	if(!valid_block_index(block_idx)) {
		cerr << "[ERROR] Block Index invalid" << endl;
		return;
	}
	if(this->index_vector.find(block_idx) != this->index_vector.end()){
		this->index_vector[block_idx] = make_pair(block_ptr,true);
	}
	else {
		if(index_vector.size() == cache_size){
			if((*index_vector.begin()).second.second) (*index_vector.begin()).second.first->serialize(get_block_file((*index_vector.begin()).first));
			delete (*index_vector.begin()).second.first;
			index_vector.erase(index_vector.begin()); 
		}
		index_vector[block_idx] = make_pair(block_ptr, true);
	}
	if(cnt >= CACHE_FLUSH_GUARD_VALUE) {
		flush_cache();
		cnt = 0;
	}
	else cnt ++;
	//block_ptr->serialize(get_block_file(block_idx));
}

void Disk::flush_cache()
{
	for(auto it : index_vector) {
		if(it.second.second) (it.second.first)->serialize(get_block_file(it.first));
		delete it.second.first;
	}
	index_vector.clear();
}

Disk::Disk(const string config_file_name) {
	cache_size = CACHE_SIZE;
	ifstream config_file(config_file_name, ios :: in);
	if(!config_file.is_open()) {
		cerr << "[ERROR] CONFIG File cannot be opened" << endl;
		return;
	}
	int l = -1, r = -1;
	//Read key-value pairs from CONFIG and set accordingly
	while(!config_file.eof()) {
		string command, value;
		config_file >> command >> value;
		// cerr << "COMMAND: " << command << " " << value << endl;
		if(command == "prefix") prefix = value;
		else if(command == "numblocks") numblocks = static_cast<unsigned int>(stoi(value));
		else if(command == "datablock") l = static_cast<int>(stoi(value));
		else if(command == "datablock-end") r = static_cast<int>(stoi(value));
	}
	// cout << l << " " << r << endl;
	// if(l != -1 && r != -1) cout << "[INFO] Blocks in use during initialization : " << l << " to " << r << endl; 
	config_file.close();

	//Assuming empty blocks
	for(unsigned int i = 0; i < numblocks; i ++) if(!(l != -1 && r != -1 && l <= static_cast<int>(i) && r >= static_cast<int>(i))) free_blocks.push(i);
}

void RowId::initialize_index(Disk& diskInstance, unsigned int num_bitmaps, unsigned int bitmap_size){
	disk_ref = &diskInstance;
	for(unsigned int i=0;i<=num_bitmaps;i++){
		int index = diskInstance.get_free_block_idx();
		Block* temp = static_cast<Block*>(new RowIDBitmapBlock);
		diskInstance.write_block(temp, index);
		this->setSecondaryEntry(i, index);
		// delete temp;
	}
}

void Bitarray::initialize_index(Disk& diskInstance, unsigned int num_bitmaps, unsigned int bitmap_size){
	// for(unsigned int i=0;i<=num_bitmaps;i++){
	// 	unsigned int first_pointer = generate_bitmap(bitmap_size, diskInstance);
	// 	this->setSecondaryEntry(i, first_pointer);
	// }
}

void Bitslice::initialize_index(Disk& diskInstance, unsigned int num_bitmaps, unsigned int bitmap_size){
	// for(unsigned int i=0;i<=num_bitmaps;i++){
	// 	unsigned int first_pointer = generate_bitmap(bitmap_size, diskInstance);
	// 	this->setSecondaryEntry(i, first_pointer);
	// }
}

unsigned int generate_bitmap(unsigned int num_records, vector<bool>& bitmap, Disk& diskInstance)
{
	int first_block_idx = -1;
	int new_block_idx = -1;
	int prev_block_idx = -1;
	BitmapBlock* blk = nullptr;
	int i = 0;
	while(num_records > 0)
	{
		//Request a free block
		new_block_idx = diskInstance.get_free_block_idx();
		if(first_block_idx == -1) first_block_idx = new_block_idx;
		if(blk != nullptr) {
			blk->set_next_block_idx(new_block_idx);
			//Write the block
			diskInstance.write_block(static_cast<Block*>(blk), prev_block_idx);
			// delete blk;
		}
		blk = new BitmapBlock;
		while(true)
		{
			//Generate a random record
			bool bit = bitmap[i];
			bool added = blk->add_bit(bit);
			if(added) {
				i ++;
				num_records --;
				if(num_records == 0) break;
			}
			else {
				break;
			}
		}
		prev_block_idx = new_block_idx;
	}
	if(blk != nullptr) {
		//Write the block
		diskInstance.write_block(static_cast<Block*>(blk), new_block_idx);	
		// delete blk;
	}
	return static_cast<unsigned int>(first_block_idx);
}

Record get_record(Disk& diskInstance, unsigned int i, unsigned int datablock_start_idx)
{
	unsigned int addr = datablock_start_idx + i / RECORD_BLOCK_FACTOR;
	Block* blk = diskInstance.read_block(addr);
	if(!(blk->get_block_type() == RECORD_BLOCK)) {
		cerr << i << " " << datablock_start_idx << " " << addr << endl;
		assert(false);
	}
	return (static_cast<RecordBlock*>(blk))->get_record(i % RECORD_BLOCK_FACTOR);
}

void RowId::insertIntoBitmap(unsigned int bitmap_index, unsigned int data){
	unsigned int add = this->getSecondaryEntry(bitmap_index);
	RowIDBitmapBlock* block = nullptr;
	bool found = false;
	do
	{
		//if(block!=nullptr) delete block;
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
		// delete block;
	}
	else{
		unsigned int new_addr = this->disk_ref->get_free_block_idx();
		block->set_next_block_idx(new_addr);
		this->disk_ref->write_block(static_cast<Block*>(block), add);
		// delete block;
		block = new RowIDBitmapBlock;
		if(block->add_rowid(data)){
			this->disk_ref->write_block(static_cast<Block*>(block), add);
			// delete block;
		}
		else{
			//if(block != nullptr) delete block;
			cerr << "[ERROR] Fatal error!" << endl;
		}
	}
}

long long RowId::sumQueryRecords(vector<bool> bfr){
	long long sum = 0;
	for(unsigned int i=0;i<=MAX_VALUE;++i){
		unsigned int add = this->getSecondaryEntry(i);
		RowIDBitmapBlock* block = nullptr;
		do
		{
			if(block!=nullptr) delete block;
			block = static_cast<RowIDBitmapBlock*>(this->disk_ref->read_block(add));
			vector<unsigned int> indices = block->read_rowids();
			for(unsigned int j=0;j<indices.size();++j){
				if(bfr[indices[j]]){
					sum += i;
				}
			}
			//iterate through block data
			if(block->get_next_block_idx()==-1){
				break;
			}
			else{
				add = block->get_next_block_idx();
			}
		}while(true);
	}
	return sum;
}

long long Bitarray::sumQueryRecords(vector<bool> bfr){
	long long sum = 0;
	for(unsigned int i=0;i<=MAX_VALUE;++i){
		long long index = 0;
		unsigned int add = this->getSecondaryEntry(i);
		BitmapBlock* block = nullptr;
		do
		{
			// if(block!=nullptr) delete block;
			block = static_cast<BitmapBlock*>(this->disk_ref->read_block(add));
			vector<bool> bitmap = block->read_bitmap();
			for(unsigned int j=0;j<bitmap.size();++j){
				if(bfr[index+j] && bitmap[j]){
					sum += i;
				}
			}
			//iterate through block data
			if(block->get_next_block_idx()==-1){
				break;
			}
			else{
				index += bitmap.size();
				add = block->get_next_block_idx();
			}
		}while(true);
	}
	return sum;
}

long long Bitslice::sumQueryRecords(vector<bool> bfr){
	long long sum = 0;
	for(unsigned int i=0;i<BITSLICE_BITS;++i){
		long long index = 0;
		unsigned int add = this->getSecondaryEntry(i);
		BitmapBlock* block = nullptr;
		do
		{
			// if(block!=nullptr) delete block;
			block = static_cast<BitmapBlock*>(this->disk_ref->read_block(add));
			vector<bool> bitmap = block->read_bitmap();
			for(unsigned int j=0;j<bitmap.size();++j){
				if(bfr[index+j] && bitmap[j]){
					sum += (1ll << i);
				}
			}
			//iterate through block data
			if(block->get_next_block_idx()==-1){
				break;
			}
			else{
				index += bitmap.size();
				add = block->get_next_block_idx();
			}
		}while(true);
	}
	return sum;
}

void RowId::constructIndex(unsigned int num_records, unsigned int datablock_start_idx){
	for(unsigned int i = 0; i < num_records; i ++)
	{
		if(i % 10000 == 0) cerr << i << " records done!" << endl;
		Record rc = get_record(*(this->get_disk_ref()), i, datablock_start_idx);
		this->addRecordToIndex(rc);
	}
}

void Bitarray::constructIndex(unsigned int num_records, unsigned int datablock_start_idx){
	unsigned int index;
	for(unsigned int j = 0; j <= MAX_VALUE; j ++)
	{
		vector<bool> bitmap;
		for(unsigned int i = 0; i < num_records; i ++)
		{	
			// if(i % 10000 == 0) cerr << i << " records done!" << endl;
			Record rc = get_record(*(this->get_disk_ref()), i, datablock_start_idx);
			bitmap.push_back(rc.amount == j);
		}
		index = generate_bitmap(num_records, bitmap, *(this->get_disk_ref()));
		cerr << j << " bitmap saved start at " << index << endl; 
		this->setSecondaryEntry(j, index);
	}
}

void Bitslice::constructIndex(unsigned int num_records, unsigned int datablock_start_idx){
	unsigned int index;
	for(unsigned int j = 0; j < BITSLICE_BITS; j ++)
	{

		vector<bool> bitmap;
		for(unsigned int i = 0; i < num_records; i ++)
		{	
			// if(i % 10000 == 0) cerr << i << " records done!" << endl;
			Record rc = get_record(*(this->get_disk_ref()), i, datablock_start_idx);
			bitmap.push_back(this->position_set(rc.amount,j));
		}
		index = generate_bitmap(num_records, bitmap, *(this->get_disk_ref()));
		cerr << j << " bitmap saved start at " << index << endl; 
		this->setSecondaryEntry(j, index);
	}
}

void RowId::initialize_existing_index(Disk& diskInstance, unsigned int num_records, unsigned int rowidblock_start_idx) {
	disk_ref = &diskInstance;
	for(unsigned int i = 0; i <= num_records; i ++ ) setSecondaryEntry(i, rowidblock_start_idx + i);
}

void Bitarray::initialize_existing_index(Disk& diskInstance, unsigned int rowidblock_start_idx, unsigned int stride) {
	disk_ref = &diskInstance;
	for(unsigned int i = 0; i <= MAX_VALUE; i ++ ) setSecondaryEntry(i, rowidblock_start_idx + i*stride);
}

void Bitslice::initialize_existing_index(Disk& diskInstance, unsigned int rowidblock_start_idx, unsigned int stride) {
	disk_ref = &diskInstance;
	for(unsigned int i = 0; i <= BITSLICE_BITS; i ++ ) setSecondaryEntry(i, rowidblock_start_idx + i*stride);
}

void RowId::addRecordToIndex(const Record& r){
	this->insertIntoBitmap(r.amount, r.id);
}

void Bitarray::addRecordToIndex(const Record& r){
		
}

void Bitslice::addRecordToIndex(const Record& r){
	
}
