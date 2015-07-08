#include <iostream>
#include <fstream>
#include <malloc.h>
#include "SobelTrying.hpp"

using namespace std;


int main(int argc, char *argv[])
{


	if(argc > 1)
	{
		b1 = fillBuffer(argv[1]);
//		b2 = fillBuffer(argv[2]);
	}

	printf("%s",b1);
	input1 = readInput(b1);
//	input2 = readInput(b2);


	free(input1);





	return 0;
}
