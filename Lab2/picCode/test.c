#include <stdio.h>

typedef struct {
	unsigned bits :4 ;
}nibbleField;

int main (void)
{
	nibbleField test;
	nibbleField test2;

	unsigned char large;

	large = 0b10111110;
	test.bits = large;
	test2.bits = large >> 4;
	printf("This is the value of large: %d, And this is test: %d \n", large, (int)test.bits);

	printf("This is the value of large: %d, And this is test: %d \n", large, (int)test2.bits);
	return 0;
}
