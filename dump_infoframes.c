#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define HDMI_INFOFRAME_HEADER_SIZE 4


unsigned long parse_int (char *str);


int main (int argc, char *argv[]) {
	unsigned long addr;
	int type, len;
	int devmem;
	void *mapping;
size_t length;
	long page_size;
	off_t map_base, extra_bytes;

	char *buf;
	ssize_t ret;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s Infoframe type(in hex)\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	type = parse_int(argv[1]);
	//length = parse_int(argv[2]);

	devmem = open("/dev/mem", O_RDONLY);
	if (devmem == -1) {
		perror("Could not open /dev/mem");
		goto open_fail;
	}
	    fflush(stdout);
	switch (type) {
		case 0x81:
			addr = 0xfef01b24;
			break;
		case 0x82:
			addr = 0xfef01b48;
			break;
		case 0x83:
			addr = 0xfef01b6c;
			break;
		case 0x84:
			addr = 0xfef01b90;
			break;
		case 0x85:
			addr = 0xfef01bb4;
			break;
		case 0x86:
			addr = 0xfef01bd8;
			break;
		case 0x87:
			addr = 0xfef01bfc;
			break;
		default:
			printf ("Unknown infoframe type\n");
			break;
	}
		
	page_size = sysconf(_SC_PAGE_SIZE);

	map_base = addr & ~(page_size - 1);
	extra_bytes = addr - map_base;

	mapping = mmap(NULL, 4096UL, PROT_READ, MAP_SHARED, devmem, map_base);

	if (mapping == MAP_FAILED) {
		perror("Could not map memory");
		goto map_fail;
	}
    fflush(stdout);
	buf = malloc(50);
	if (buf == NULL) {
		fprintf(stderr, "Failed to allocate memory\n");
		goto alloc_fail;
	}
	
	/*
	 * Using a separate buffer for write stops the kernel from
	 * complaining quite as much as if we passed the mmap()ed
	 * buffer directly to write().
	 */

	memcpy(buf, (char *)mapping+extra_bytes, 50);
	int is_valid = 0;
	switch (buf[0]) {
		case 0x81:
			if (buf[1] == 0x01 || buf[1] == 0x02 || buf[1] == 0x03) is_valid = 1;
			break;
		case 0x82:
			if (buf[1] == 0x02 || buf[1] == 0x03) is_valid = 1;
			break;
		case 0x83:
			if (buf[1] == 0x01) is_valid = 1;
			break;
		case 0x84:
			if (buf[1] == 0x01) is_valid = 1;
			break;
		case 0x85:
			if (buf[1] == 0x01) is_valid = 1;
			break;
		case 0x86:
			if (buf[1] == 0x01) is_valid = 1;
			break;
		case 0x87:
			if (buf[1] == 0x01) is_valid = 1;
			break;
		default:
			printf ("invalid infoframe\n");
			is_valid = 0;
			break;
	}
	/*
	 * data byte 3 or buf[2] is size of infoframe
	 * add header size which includes type, version,
	 * size and checksum (4 bytes)
	 */
	len = buf[2] + HDMI_INFOFRAME_HEADER_SIZE;  
	
	if (is_valid) {
		printf ("%02x",buf[0]);
		for (int i=1; i < len; i++) {
			if (i==3 || i==11 || i==19 || i==27 || i==35) {  // weird hack to compensate for unknown addition addional 0x00 every 8th byte starting at 3 array index
				i++; 		
				len +=1;
			}
			printf (":%02x",buf[i]);
		}
		printf ("\n");
	}
	free(buf);

alloc_fail:
	munmap(mapping, 50 + extra_bytes);

map_fail:
	close(devmem);

open_fail:
	return EXIT_SUCCESS;
}

unsigned long parse_int (char *str) {
	long long result;
	char *endptr; 

	result = strtoll(str, &endptr, 0);
	if (*str == '\0' || *endptr != '\0') {
		fprintf(stderr, "\"%s\" is not a valid number\n", str);
		exit(EXIT_FAILURE);
	}

	return (unsigned long)result;
}
