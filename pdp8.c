#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void process_obj(void);
void drop_the_bass(void);
char trim_before(char c, FILE *file);
short str_to_int(char *str);
void AND(short bits);
void TAD(short bits);
void ISZ(short bits);
void DCA(short bits);
void JMS(short bits);
void JMP(short bits);
void OPR(short bits);
void IOT(short bits);

short pc = -1;
short link_bit = 0;
short accumulator = 0;
short mem[4096];
int verbose = 0;
FILE *input = NULL;

int main(int argc, char const *argv[])
{
	/* code */

	if(argc < 2){
		fprintf(stderr, "Usage: pdp8 [-v] object.obj\n");
		exit(1);
	}
	else if(argc == 2){
		input = fopen(argv[1], "r");
	}
	else if(argc == 3){
		if((argv[1][0] == '-') && (argv[1][1] == 'v') && (argv[1][2] == '\0')){
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

	process_obj();
	drop_the_bass();

	return 0;
}

void process_obj(void){
	int c;
	int i;
	char addr[4];
	char inst[4];

	i = 0;

	c = getc(input);
	while(c != EOF){
		// fprintf(stdout, "%c", c);

		i = 0;
		c = trim_before(c, input);
		while((c != ':')){
			addr[i] = c;

			i++;
			c = getc(input);
		}
		addr[i] = '\0';

		i = 0;
		c = getc(input);
		c = trim_before(c, input);

		while((c != '\n')){
			inst[i] = c;

			i++;
			c = getc(input);
		}
		inst[i] = '\0';

		// fprintf(stdout, "%s:%s\n", addr, inst);

		if(addr[1] == 'P'){
			pc = str_to_int(inst);
		}
		else{
			mem[str_to_int(addr)] = str_to_int(inst);
		}

		c = getc(input);
	}
}

void drop_the_bass(void){
	short instruction;
	short opcode;
	short rest;
	int count;

	instruction = 0;
	opcode = 0;
	count = 0;
	rest = 0;

	while(pc != -1){
		// fetch
		instruction = mem[pc];

		// decode
		opcode = ((instruction & 0xE00) >> 9) & 7;
		rest = instruction & 0x1FF;
		count++;

		switch(opcode){
			case 0:
				AND(rest);
				pc++;
				break;
			case 1:
				TAD(rest);
				pc++;
				break;
			case 2:
				ISZ(rest);
				pc++;
				break;
			case 3:
				DCA(rest);
				pc++;
				break;
			case 4:
				JMS(rest);
				break;
			case 5:
				JMP(rest);
				break;
			case 6:
				IOT(rest);
				pc++;
				break;
			case 7:
				OPR(rest);
				pc++;
				break;
			default:
				fprintf(stderr, "Invalid opcode: %d\n", opcode);
				exit(1);
		}

		if(count == 20){
			break;
		}
	}
}

char trim_before(char c, FILE *file){
	while((c == ' ') || (c == '\t') || (c == '\n')){
			c = getc(file);
	}

	return c;
}

short str_to_int(char *str){
	int i;
	char c;
	short result;

	i = 0;
	c = str[i];
	result = 0;

	while(c != '\0'){
		result = result << 4;
		if((c >= '0') && (c <= '9')){
			result += ((int)(c - '0'));
		}
		else if((c >= 'A') && (c <= 'Z')){
			result += (((int)(c - 'A')) + 10);
		}
		else{
			fprintf(stderr, "INVALID CHARACTER\n");
		}

		i++;
		// fprintf(stdout, "%d\n", result);
		c = str[i];
	}

	return result;
}

void AND(short bits){
	short address = 0;
	address = (bits & 0x7F) + ((bits & 0x80) ? (pc & 0xF80) : 0);
	accumulator = accumulator & ((bits & 0x100) ? (mem[mem[address]]) : (mem[address]));

	fprintf(stdout, "AND--%d--%d--\n", accumulator, link_bit);
}

void TAD(short bits){
	short address = 0;
	address = (bits & 0x7F) + ((bits & 0x80) ? (pc & 0xF80) : 0);
	accumulator = (accumulator + 
		((bits & 0x100) ? (mem[mem[address]]) : (mem[address])));
	link_bit = ((accumulator & 0xF000) ? (!link_bit) : link_bit);

	fprintf(stdout, "TAD--%d--%d--\n", accumulator, link_bit);
}

void ISZ(short bits){
	// code
}

void DCA(short bits){
	short address = 0;
	address = (bits & 0x7F) + ((bits & 0x80) ? (pc & 0xF80) : 0);
	if(bits & 0x100){
		mem[mem[address]] = accumulator;
	}
	else{
		mem[address] = accumulator;
	}
	accumulator = 0;
	
	fprintf(stdout, "DCA--%d--%d--\n", accumulator, link_bit);
}

void JMS(short bits){
	// code
}

void JMP(short bits){
	fprintf(stdout, "JMP-from--%d--\n", pc);
	short address = 0;
	address = (bits & 0x7F) + ((bits & 0x80) ? (pc & 0xF80) : 0);

	pc = (bits & 0x100) ? (mem[address]) : (address);
	fprintf(stdout, "JMP--to---%d--\n", pc);
}

void OPR(short bits){
	// code
	if((bits & 0x100) && (bits & 0x002)){
		fprintf(stdout, "HALT\n");
		exit(0);
	}
}

void IOT(short bits){
	// code
}
