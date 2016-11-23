/*
 * a very simple lz4 decoder
 * Copyright (c) 2016 Du Huanpeng<u74147@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>




char flash[500];
char sdram[1<<18];
char lz4[]={
#include "lz4data.c"
};



int unlz4(int start)
{
	int i = 0;
	int j = 0;

	unsigned int token;
	unsigned int len;
	int error = 0;
	
	#define limit len

	i = start;

	fprintf(stderr, "info: decoding...\n");

	do {
		token = (unsigned char)lz4[i++];
		len = 15U & (token>>4);
		if(len == 15U) {
			do {
				len += (unsigned int)lz4[i];
			} while(lz4[i++] == '\255');
		}
		limit = i + len;
		while(i<limit) {
			sdram[j] = lz4[i];
			j++, i++;
		}
		len = 15U & token;
		
#define offset token
		offset  = (unsigned char)lz4[i++];
		offset += (unsigned char)lz4[i++]*256U;

		if(len == 15U) {
			do {
				len += (unsigned int)lz4[i];
			} while(lz4[i++] == '\255');
		}
		len += 4;
		limit = j + len;
		if(limit>=sizeof(sdram)) {
			error |= 0x80000001;
			fprintf(stderr, "warning: out of memory[0x%08x]\n", limit);
			limit = sizeof(sdram);
		}
		if(j<offset){
			fprintf(stderr, "bug-bug!: invail offset[%08x]\n", offset);
			error |= 0x80000002;
			return error;
		}

		while(j<limit) {
			sdram[j] = sdram[j-offset];
			j++;
		}
		if(i+32U>=sizeof(lz4)) { /* TODO: finish */
			fprintf(stderr, "info: finish %s", error ? "with error\n" : "\n");
			error = j;
		}
	} while(error == 0);
	return error;
}

int unlz4f(int offset)
{
	int i;
	int ch;

	i = offset;


	if(lz4[i++] != '\x04') return -1;
	if(lz4[i++] != '\x22') return -1;
	if(lz4[i++] != '\x4D') return -1;
	if(lz4[i++] != '\x18') return -1;

	ch = lz4[i++];		/* FLG */

	if(ch & (1<<3)) {	/* BD */
		ch = lz4[i++];
		i += 8;		/* Content */
	} else {
		ch = lz4[i++];
	}

	ch = lz4[i++];

	ch = lz4[i++];
	ch = lz4[i++];
	ch = lz4[i++];
	ch = lz4[i++];

	offset = i;
	return 	unlz4(offset);

}


int main(int argc, char *argv[])
{
	return unlz4f(0);
}

