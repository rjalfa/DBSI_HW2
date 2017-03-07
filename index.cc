#include "index.h"

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
		block = static_cast<RowIDBitmapBlock*>(this->disk_ref->read_block(new_addr, ROWID_BITMAP_BLOCK));
		if(block->add_rowid(data)){
			this->disk_ref->write_block(static_cast<Block*>(block), add);
			delete block;
		}
		else{
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

int main(){
	return 0;
}