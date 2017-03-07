#include "common.h"

unsigned int generate_table(unsigned int num_records, Disk& diskInstance)
{
	unsigned int counter = 0;
	int first_block_idx = -1;
	int new_block_idx = -1;
	int prev_block_idx = -1;
	RecordBlock* blk = nullptr;
	while(num_records > 0)
	{
		//Request a free block
		new_block_idx = diskInstance.get_free_block_idx();
		if(first_block_idx == -1) first_block_idx = new_block_idx;
		if(blk != nullptr) {
			blk->set_next_block_idx(new_block_idx);
			//Write the block
			diskInstance.write_block(static_cast<Block*>(blk), prev_block_idx);
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
		prev_block_idx = new_block_idx;
	}
	if(blk != nullptr) {
		//Write the block
		diskInstance.write_block(static_cast<Block*>(blk), new_block_idx);	
	}
	return static_cast<unsigned int>(first_block_idx);
}

int main()
{
	//Start Disk
	Disk diskInstance("CONFIG");
	auto idx = generate_table(20'00'000, diskInstance);
	diskInstance.flush_cache();
	//Prompt based I/O
	ofstream of("CONFIG", ios::app);
	of << "datablock " << idx << endl;
	of << "numrecords " << 20'00'000 << endl;
	of.close();
	return 0;
}