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
