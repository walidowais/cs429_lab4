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
	short instruction;
	short opcode;
	short rest;
	int count;

	instruction = 0;
	opcode = 0;
	count = 0;
	rest = 0;

	while(pc >= 0){
		// fetch
		instruction = mem[pc];

		// decode
		opcode = ((instruction & 0xE00) >> 9) & 7;
		rest = instruction & 0x1FF;
		count++;

		switch(opcode){
			case 0:
				AND(rest);
				break;
			case 1:
				TAD(rest);
				break;
			case 2:
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

		if(count == 20){
			break;
		}
	}
}

void AND(short bits){
	short address = 0;
	time += 2;

	address = (bits & 0x7F) + ((bits & 0x80) ? (pc & 0xF80) : 0);

	if(bits & 0x100){
		accumulator = accumulator & mem[mem[address]];
		time++;
	}
	else{
		accumulator = accumulator & mem[address];
	}

	if(verbose){
		fprintf(stdout, v_format, time, pc, mem[pc], "AND", accumulator, link_bit);
	}

	pc++;
}

void TAD(short bits){
	short address = 0;
	time += 2;

	address = (bits & 0x7F) + ((bits & 0x80) ? (pc & 0xF80) : 0);
	
	if(bits & 0x100){
		accumulator = accumulator + mem[mem[address]];
		time++;
	}
	else{
		accumulator = accumulator + mem[address];
	}
	link_bit = ((accumulator & 0xF000) ? (!link_bit) : link_bit);

	if(verbose){
		fprintf(stdout, v_format, time, pc, mem[pc], "TAD", accumulator, link_bit);
	}

	pc++;
}

void ISZ(short bits){
	//TODO

	if(verbose){
		fprintf(stdout, v_format, time, pc, mem[pc], "ISZ", accumulator, link_bit);
	}

	pc++;
}

void DCA(short bits){
	short address = 0;
	time += 2;

	address = (bits & 0x7F) + ((bits & 0x80) ? (pc & 0xF80) : 0);
	
	if(bits & 0x100){
		mem[mem[address]] = accumulator;
		time++;
	}
	else{
		mem[address] = accumulator;
	}
	accumulator = 0;
	
	if(verbose){
		fprintf(stdout, v_format, time, pc, mem[pc], "DCA", accumulator, link_bit);
	}

	pc++;
}

void JMS(short bits){
	short address = 0;
	time += 2;

	if(verbose){
		fprintf(stdout, v_format, time, pc, mem[pc], "JMS", accumulator, link_bit);
	}

	address = (bits & 0x7F) + ((bits & 0x80) ? (pc & 0xF80) : 0);

	if(bits & 0x100){
		//indirect
		mem[mem[address]] = pc + 1;
		pc = mem[mem[address]] + 1;
		time++;
	}
	else{
		//direct
		mem[address] = pc + 1;
		pc = address + 1;
	}
}

void JMP(short bits){
	short address = 0;
	time += 1;

	if(verbose){
		fprintf(stdout, v_format, time, pc, mem[pc], "JMP", accumulator, link_bit);
	}

	address = (bits & 0x7F) + ((bits & 0x80) ? (pc & 0xF80) : 0);

	if(bits & 0x100){
		pc = mem[address];
		time++;
	}
	else{
		pc = address;
	}
}

void OPR(short bits){
	char *ops = "";
	time++;

	if(!(bits & 0x100)){
		// group 1
		if(bits & 0x80){
			//CLA
			accumulator = 0;
		}
		if(bits & 0x40){
			//CLL
			link_bit = 0;
		}
		if(bits & 0x20){
			//CMA
			accumulator = (~accumulator) & 0xFFF;
		}
		if(bits & 0x10){
			//CML
			link_bit = (~link_bit) & 0x1;
		}
		if(bits & 0x01){
			//IAC
			accumulator++;
			if(accumulator & 0xF000){
				link_bit = (~link_bit) & 0x1;
				accumulator = accumulator & 0xFFF;
			}
		}
		if((bits & 0x08) && !(bits & 0x02)){
			//RAR
			//TODO
		}
		if((bits & 0x04) && !(bits & 0x02)){
			//RAL
			//TODO
		}
		if((bits & 0x08) && (bits & 0x02)){
			//RTR
			//TODO
		}
		if((bits & 0x04) && (bits & 0x02)){
			//RTL
			//TODO
		}
	}
	else{
		//group 2
		if(bits & 0x40){
			//SMA
			//TODO
		}
		if(bits & 0x20){
			//SZA
			//TODO
		}
		if(bits & 0x10){
			//SNL
			//TODO
		}
		if(bits & 0x08){
			//RSS
			//TODO
		}
		if(bits & 0x80){
			//CLA
			//TODO
		}
		if(bits & 0x04){
			//OSR
			//TODO
		}
		if(bits & 0x02){
			//HLT
			//TODO
			pc = -999;
		}
	}

	if(verbose){
		fprintf(stdout, v_format, time, pc, mem[pc], ops, accumulator, link_bit);
	}
	
	pc++;
}

void IOT(short bits){
	//TODO
	time += 1;

	if(verbose){
		fprintf(stdout, v_format, time, pc, mem[pc], "IOT", accumulator, link_bit);
	}

	pc++;
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
			fprintf(stderr, "Invalid Character.\n");
			exit(1);
		}

		i++;
		// fprintf(stdout, "%d\n", result);
		c = str[i];
	}

	return result;
}