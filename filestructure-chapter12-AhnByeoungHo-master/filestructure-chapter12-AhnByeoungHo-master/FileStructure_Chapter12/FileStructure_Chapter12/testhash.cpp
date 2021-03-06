// tsthash.cpp

#include "direct.h"
#include "bucket.h"
#include "hash.h"
#include <iostream>

main()
{
	int result;
	Directory Dir(4);
	result = Dir.Open("tsthash");
	if (result == 0)
	{
		cout << "Please create tsthash.dir and tsthash.bkt" << endl;
		return 0;
	}
	char * keys[] = { "bill", "lee", "pauline", "alan", "julie",
		"mike", "elizabeth", "mark", "ashley", "peter",
		"joan", "john", "charles", "mary", "emily" };
	const int numkeys = 15;
	for (int i = 0; i<numkeys; i++)
	{
		cout << keys[i] << " " << (void*)Hash(keys[i])
			<< " " << (void*)MakeAddress(keys[i], 16) << endl;
		result = Dir.Remove(keys[i]);
		if (result == 0) cout << "remove for " << keys[i] << " failed" << endl;
		Dir.Print(cout);
	}
	return 1;
}