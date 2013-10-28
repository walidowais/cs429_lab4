/*

	Name: Walid Owais
	EID: wo783
	Skip days: 1

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void process_obj(void);
void drop_the_bass(void);
char trim_before(char c, FILE *file);
int str_to_int(char *str);
void AND(int bits);
void TAD(int bits);
void ISZ(int bits);
void DCA(int bits);
void JMS(int bits);
void JMP(int bits);
void OPR(int bits);
void IOT(int bits);

int pc = -1;
int link_bit = 0;
int accumulator = 0;
int mem[4096];
int verbose = 0;
int halt = 0;
long long int time = 0;
FILE *input = NULL;

char *v_format = "Time %lld: PC=0x%03X instruction = 0x%03X (%s), rA = 0x%03X, rL = %d\n";

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
		exit(1);
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
	int instruction;
	int opcode;
	int rest;

	instruction = 0;
	opcode = 0;
	rest = 0;

	while(!halt){
		// fetch
		instruction = mem[pc];

		// decode
		opcode = ((instruction & 0xE00) >> 9) & 7;
		rest = instruction & 0x1FF;

		switch(opcode){
			case 0:
				AND(rest);
				break;
			case 1:
				TAD(rest);
				break;
			case 2:
				// isz2();
				ISZ(rest);
				break;
			case 3:
				DCA(rest);
				break;
			case 4:
				JMS(rest);
				break;
			case 5:
				JMP(rest);
				break;
			case 6:
				IOT(rest);
				break;
			case 7:
				OPR(rest);
				break;
			default:
				fprintf(stderr, "Invalid opcode: %d\n", opcode);
				exit(1);
		}
		
		pc = pc & 0xFFF;
	}
}

void AND(int bits){
	int address = 0;
	time += 2;

	address = (bits & 0x7F) + ((bits & 0x80) ? (pc & 0xF80) : 0);

	if(bits & 0x100){
		accumulator = accumulator & mem[mem[address]];
		time++;

		if(verbose){
			fprintf(stderr, v_format, time, pc, mem[pc], "AND I", accumulator, link_bit);
		}
	}
	else{
		accumulator = accumulator & mem[address];

		if(verbose){
			fprintf(stderr, v_format, time, pc, mem[pc], "AND", accumulator, link_bit);
		}
	}

	pc++;
}

void TAD(int bits){
	int address = 0;
	time += 2;

	address = (bits & 0x7F) + ((bits & 0x80) ? (pc & 0xF80) : 0);
	
	if(bits & 0x100){
		accumulator = accumulator + mem[mem[address]];
		if(accumulator & 0xF000){
			link_bit = (~link_bit) & 0x1;
			accumulator = accumulator & 0xFFF;
		}

		time++;

		if(verbose){
			fprintf(stderr, v_format, time, pc, mem[pc], "TAD I", accumulator, link_bit);
		}
	}
	else{
		accumulator = accumulator + mem[address];
		if(accumulator & 0xF000){
			link_bit = (~link_bit) & 0x1;
			accumulator = accumulator & 0xFFF;
		}

		if(verbose){
			fprintf(stderr, v_format, time, pc, mem[pc], "TAD", accumulator, link_bit);
		}
	}

	pc++;
}

void ISZ(int bits){
	int pc_orig;
	int address;

	pc_orig = pc;
	address = (bits & 0x7F) + ((bits & 0x080) ? (pc & 0xF80) : (0));

	if(bits & 0x100){
		mem[mem[address]] = (mem[mem[address]] + 1) & 0xFFF;
		if(mem[mem[address]] == 0){
			pc++;
		}

	}
	else{
		mem[address] = (mem[address] + 1) & 0xFFF;
		if(mem[address] == 0){
			pc++;
		}
	}

	pc++;
	time += 2;

	accumulator = accumulator & 0xFFF;
	if(verbose){
		fprintf(stderr, v_format, time, pc_orig, mem[pc_orig], "ISZ", accumulator, link_bit);
	}

}

void DCA(int bits){
	int address = 0;
	time += 2;

	address = (bits & 0x7F) + ((bits & 0x80) ? (pc & 0xF80) : 0);
	
	if(bits & 0x100){
		mem[mem[address]] = accumulator;
		accumulator = 0;
		time++;

		if(verbose){
			fprintf(stderr, v_format, time, pc, mem[pc], "DCA I", accumulator, link_bit);
		}
	}
	else{
		mem[address] = accumulator;
		accumulator = 0;

		if(verbose){
			fprintf(stderr, v_format, time, pc, mem[pc], "DCA", accumulator, link_bit);
		}
	}

	pc++;
}

void JMS(int bits){
	int address = 0;
	time += 2;


	address = (bits & 0x7F) + ((bits & 0x80) ? (pc & 0xF80) : 0);

	if(bits & 0x100){
		//indirect
		time++;
		if(verbose){
			fprintf(stderr, v_format, time, pc, mem[pc], "JMS I", accumulator, link_bit);
		}

		mem[mem[address]] = pc + 1;
		pc = mem[address] + 1;
	}
	else{
		//direct
		if(verbose){
			fprintf(stderr, v_format, time, pc, mem[pc], "JMS", accumulator, link_bit);
		}

		mem[address] = pc + 1;
		pc = address + 1;
	}
}

void JMP(int bits){
	int address = 0;
	time += 1;

	address = (bits & 0x7F) + ((bits & 0x80) ? (pc & 0xF80) : 0);

	if(bits & 0x100){
		time++;
		if(verbose){
			fprintf(stderr, v_format, time, pc, mem[pc], "JMP I", accumulator, link_bit);
		}

		pc = mem[address];
	}
	else{
		if(verbose){
			fprintf(stderr, v_format, time, pc, mem[pc], "JMP", accumulator, link_bit);
		}

		pc = address;
	}
}



void IOT(int bits){
	int device;

	device = (bits & 0x1F8) >> 3;
	time += 1;

	if(device == 3){
		accumulator = getchar();
		accumulator = accumulator & 0xFFF;
		if(verbose){
			fprintf(stderr, v_format, time, pc, mem[pc], "IOT 3", accumulator, link_bit);
		}
	}
	else if(device == 4){
			putchar((accumulator & 0xFF));

		if(verbose){
			fprintf(stderr, v_format, time, pc, mem[pc], "IOT 4", accumulator, link_bit);
		}
	}


	pc++;
}

char trim_before(char c, FILE *file){
	while((c == ' ') || (c == '\t') || (c == '\n')){
			c = getc(file);
	}

	return c;
}

int str_to_int(char *str){
	int i;
	char c;
	int result;

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
			fprintf(stderr, "Invalid Character.\n");
			exit(1);
		}

		i++;
		// fprintf(stderr, "%d\n", result);
		c = str[i];
	}

	return result;
}

void OPR(int bits){
	int rss, size, pc_orig;
	char *ops;

	pc_orig = pc;
	ops = malloc(40 * sizeof(char));
	size = 0;
	rss = 0;
	time++;

	if(!(bits & 0x100)){
		// group 1
		if(bits & 0x80){
			//CLA
			accumulator = 0;

			ops[size] = 'C';
			size++;
			ops[size] = 'L';
			size++;
			ops[size] = 'A';
			size++;
		}
		if(bits & 0x40){
			//CLL
			link_bit = 0;

			if(size){
				ops[size] = ' ';
				size++;
			}
			ops[size] = 'C';
			size++;
			ops[size] = 'L';
			size++;
			ops[size] = 'L';
			size++;
		}
		if(bits & 0x20){
			//CMA
			accumulator = (~accumulator) & 0xFFF;

			if(size){
				ops[size] = ' ';
				size++;
			}
			ops[size] = 'C';
			size++;
			ops[size] = 'M';
			size++;
			ops[size] = 'A';
			size++;
		}
		if(bits & 0x10){
			//CML
			link_bit = (~link_bit) & 0x1;

			if(size){
				ops[size] = ' ';
				size++;
			}
			ops[size] = 'C';
			size++;
			ops[size] = 'M';
			size++;
			ops[size] = 'L';
			size++;
		}
		if(bits & 0x01){
			//IAC
			accumulator++;
			if(accumulator & 0xF000){
				link_bit = (~link_bit) & 0x1;
				accumulator = accumulator & 0xFFF;
			}

			if(size){
				ops[size] = ' ';
				size++;
			}
			ops[size] = 'I';
			size++;
			ops[size] = 'A';
			size++;
			ops[size] = 'C';
			size++;
		}
		if((bits & 0x08) && !(bits & 0x02)){
			//RAR
			accumulator = accumulator + (link_bit << 12);
			link_bit = accumulator & 0x1;
			accumulator = (accumulator >> 1) & 0xFFF;

			if(size){
				ops[size] = ' ';
				size++;
			}
			ops[size] = 'R';
			size++;
			ops[size] = 'A';
			size++;
			ops[size] = 'R';
			size++;
		}
		if((bits & 0x04) && !(bits & 0x02)){
			//RAL
			accumulator = (accumulator << 1) + link_bit;
			link_bit = accumulator & 0x1000;
			accumulator = accumulator & 0xFFF;

			if(size){
				ops[size] = ' ';
				size++;
			}
			ops[size] = 'R';
			size++;
			ops[size] = 'A';
			size++;
			ops[size] = 'L';
			size++;
		}
		if((bits & 0x08) && (bits & 0x02)){
			//RTR
			accumulator += link_bit << 12;
			accumulator += (accumulator & 0x1) << 13;
			link_bit = accumulator & 0x2;
			accumulator = (accumulator >> 2) && 0xFFF;

			if(size){
				ops[size] = ' ';
				size++;
			}
			ops[size] = 'R';
			size++;
			ops[size] = 'T';
			size++;
			ops[size] = 'R';
			size++;
		}
		if((bits & 0x04) && (bits & 0x02)){
			//RTL
			accumulator = (accumulator << 2) + (link_bit << 1);
			accumulator += (accumulator & 0x2000) >> 13;
			link_bit = (accumulator & 0x1000) >> 12;
			accumulator = accumulator & 0xFFF;

			if(size){
				ops[size] = ' ';
				size++;
			}
			ops[size] = 'R';
			size++;
			ops[size] = 'T';
			size++;
			ops[size] = 'L';
			size++;
		}
	}
	else{
		//group 2
		if(bits & 0x08){
			//RSS
			rss = 1;
		}
		if(bits & 0x40){
			//SMA
			if(!rss && (accumulator & 0x800)){
				pc++;
			}
			else if(rss && !(accumulator & 0x800)){
				pc++;
			}
			if(size){
				ops[size] = ' ';
				size++;
			}
			ops[size] = 'S';
			size++;
			ops[size] = 'M';
			size++;
			ops[size] = 'A';
			size++;
		}
		if(bits & 0x20){
			//SZA
			if(!rss && !accumulator){
				pc++;
			}
			else if(rss && accumulator){
				pc++;
			}
			if(size){
				ops[size] = ' ';
				size++;
			}
			ops[size] = 'S';
			size++;
			ops[size] = 'Z';
			size++;
			ops[size] = 'A';
			size++;
		}
		if(bits & 0x10){
			//SNL
			if(!rss && link_bit){
				pc++;
			}
			else if(rss && !link_bit){
				pc++;
			}
			if(size){
				ops[size] = ' ';
				size++;
			}
			ops[size] = 'S';
			size++;
			ops[size] = 'N';
			size++;
			ops[size] = 'L';
			size++;
		}
		if(rss){
			if(size){
				ops[size] = ' ';
				size++;
			}
			ops[size] = 'R';
			size++;
			ops[size] = 'S';
			size++;
			ops[size] = 'S';
			size++;
		}
		if(bits & 0x80){
			//CLA
			accumulator = 0;
			if(size){
				ops[size] = ' ';
				size++;
			}
			ops[size] = 'C';
			size++;
			ops[size] = 'L';
			size++;
			ops[size] = 'A';
			size++;
		}
		if(bits & 0x04){
			//OSR
			//NOP
			if(size){
				ops[size] = ' ';
				size++;
			}
			ops[size] = 'O';
			size++;
			ops[size] = 'S';
			size++;
			ops[size] = 'R';
			size++;
		}
		if(bits & 0x02){
			//HLT
			halt = 1;
			if(size){
				ops[size] = ' ';
				size++;
			}
			ops[size] = 'H';
			size++;
			ops[size] = 'L';
			size++;
			ops[size] = 'T';
			size++;
		}
	}

	ops[size] = '\0';

	if(verbose){
		fprintf(stderr, v_format, time, pc_orig, mem[pc_orig], ops, accumulator, link_bit);
	}

	free(ops);
	
	pc++;
}