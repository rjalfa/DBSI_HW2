#ifndef __INDEX_H
#define __INDEX_H

#include "common.h"

#include <map>

using namespace std;

unsigned int generate_bitmap(unsigned int num_records, Disk& diskInstance);

class Index
{
protected:
	map<unsigned int, int> secondary_index;
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
};

class Bitmap: public Index {
public:
	// virtual void initialize_index(Disk& diskInstance, unsigned int num_bitmaps, unsigned int bitmap_size) = 0;	
};

class RowId: public Bitmap {

public:
	void initialize_index(Disk& diskInstance, unsigned int num_bitmaps, unsigned int bitmap_size);
};

class Bitarray: public Bitmap {
	
public:
	void initialize_index(Disk& diskInstance, unsigned int num_bitmaps, unsigned int bitmap_size);
};


class Bitslice: public Index {
	int index_size;
	bool position_set(int n, int position){
		return (1&(n>>(position)));
	}
	bool generateBitIndex(Disk& diskInstance, int bit_position);
public:
	Bitslice() { };
	void initialize_index(Disk& diskInstance, unsigned int num_bitmaps, unsigned int bitmap_size);
};

#endif