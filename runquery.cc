#include "common.h"

#define BF_SIZE 2000000

int main()
{
	vector<bool> query_vector(BF_SIZE, 0);
	int n, temp;
	cin>>n;
	for(int i=0;i<n;++i){
		cin>>temp;
		query_vector[temp] = true;
	}
	return 0;
}