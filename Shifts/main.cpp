#include<iostream>
using namespace std;

void main()
{
	setlocale(LC_ALL, "");
	unsigned int mask = UINT_MAX;
	mask <<= 7;
	cout << hex << mask << endl;
	for (int i = 0; i < 32; i++)
	{
		mask <<= 1;
		cout << hex << mask << endl;
	}
}