#include "common.h"

#include <bits/stdc++.h>

using namespace std;

class Index
{
protected:
	map<unsigned int, string> secondary_index;
public:
	bool isInIndex(int a){
		if(this->secondary_index.find(a) == this->secondary_index.end()){
			return false;
		}
		return true;
	}
	string getSecondaryEntry(int a)
	{
		return this->secondary_index[a];
	}
	void setSecondaryEntry(int a, string x)
	{
		this->secondary_index[a] = x;
	}
};

class Bitmap: public Index {

};

class RowId: public Bitmap {

public:

};

class Bitarray: public Bitmap {

};


class Bitslice: public Index {
	int index_size;
	public:
		Bitslice() : nbits(12) { };
		String bitsliceIndex(int a){
			ostringstream os;
			os << setbase(2) << a;
			string basic = os.str();
			int extra_zeros=this->index_size-basic.length();
			os.flush();
			while(extra_zeros--){
				os << 0;
			}
			string s = os.str() + basic;
			return s;
		}
		bool createIndex(RowIDBitmapBlock* starting_block){

		}

};
