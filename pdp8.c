#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void process(void);

short link_bit;
short accumulator;
short mem[4096];

int verbose;
FILE *input = NULL;

int main(int argc, char const *argv[])
{
	/* code */
	link_bit = 0;
	accumulator = 0;

	verbose = 0;

	if(argc < 2){
		fprintf(stderr, "Usage: pdp8 [-v] object.obj\n");
		exit(1);
	}
	else if(argc == 2){
		input = fopen(argv[1], "r");
	}
	else if(argc == 3){
		if(strcmp(argv[1], "-v")){
			verbose = 1;
			input = fopen(argv[2], "r");
		}
		else{
			fprintf(stderr, "Usage: pdp8 [-v] object.obj\n");
			exit(1);
		}
	}
	else{
		fprintf(stderr, "Usage: pdp8 [-v] object.obj\n");
		exit(1);
	}

	if(input == NULL){
		fprintf(stderr, "Error opening file.");
	}

	process();

	return 0;
}

void process(void){
	int c;

	c = getc(input);
	while(c != EOF){
		fprintf(stdout, "%c\n", c);

		c = getc(input);
	}
}