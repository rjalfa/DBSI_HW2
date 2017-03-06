#include "common.h"

unsigned int generate_table(unsigned int num_records, Disk& diskInstance)
{
	unsigned int counter = 0;
	int first_block_idx = -1;
	RecordBlock* blk = nullptr;
	while(num_records > 0)
	{
		//Request a free block
		int new_block_idx = diskInstance.get_free_block_idx();
		if(first_block_idx == -1) first_block_idx = new_block_idx;
		if(blk != nullptr) {
			blk->set_next_block_idx(new_block_idx);
			delete blk;
		}
		blk = new RecordBlock;
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
			else {
				counter --;
				break;
			}
		}
		//Write the block
		diskInstance.write_block(static_cast<Block*>(blk), new_block_idx);
	}
	if(blk != nullptr) delete blk;
	return static_cast<unsigned int>(first_block_idx);
}

int main()
{
	//Start Disk
	Disk diskInstance("CONFIG");
	auto idx = generate_table(20'00'000, diskInstance);
	//Prompt based I/O
	ofstream of("CONFIG", ios::app);
	of << "datablock " << idx << endl;
	of.close();
	return 0;
}