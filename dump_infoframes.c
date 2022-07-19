/*
 * Copyright (c) 2021 - 2022 Juan Francisco Loya
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * See the File README and COPYING for more detail about License
 *
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <ctype.h>


#define HDMI_INFOFRAME_HEADER_SIZE 4
static inline void *bytecopy(void *const dest, void const *const src, size_t bytes)
{
        while (bytes-->(size_t)0)
                ((unsigned int *)dest)[bytes] = ((unsigned int const *)src)[bytes];

        return dest;
}


unsigned long parse_int (char *str);


static const unsigned int hdmi_ram_pack_address[2][7] =
{
	{ 0xfef01b24, 0xfef01b48, 0xfef01b6c, 0xfef01b90, 0xfef01bb4, 0xfef01bd8, 0xfef01bfc }, 
	{ 0x3f902424, 0x3f902448, 0x3f90246c, 0x3f902490, 0x3f9024b4, 0x3f9024d8, 0x3f9024fc }, 
};

int main (int argc, char *argv[]) {
	uint64_t addr;
	int type, len, idx;
	int devmem;
	void *mapping, *virt_addr;
	unsigned offset;
	unsigned int page_size, map_size;
	off_t map_base;

	char *buf;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <Pi Model (RPI2 or RPI3 or RPI4)> <Infoframe type(in hex)>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
    if (!strcmp(argv[1], "RPI2")) {
		idx = 1;	
	} else if (!strcmp(argv[1], "RPI3")) {
		idx = 1;
	} else if (!strcmp(argv[1], "RPI4")) {
		idx = 0;
	}
	type = parse_int(argv[2]);

	devmem = open("/dev/mem", O_RDONLY);
	if (devmem == -1) {
		perror("Could not open /dev/mem");
		goto open_fail;
	}
	    fflush(stdout);
	switch (type) {
		case 0x81:
			addr = hdmi_ram_pack_address[idx][0];
			break;
		case 0x82:
			addr = hdmi_ram_pack_address[idx][1];
			break;
		case 0x83:
			addr = hdmi_ram_pack_address[idx][2];
			break;
		case 0x84:
			addr = hdmi_ram_pack_address[idx][3];
			break;
		case 0x85:
			addr = hdmi_ram_pack_address[idx][4];
			break;
		case 0x86:
			addr = hdmi_ram_pack_address[idx][5];
			break;
		case 0x87:
			addr = hdmi_ram_pack_address[idx][6];
			break;
		default:
			printf ("Unknown infoframe type\n");
			break;
	}

	page_size = sysconf(_SC_PAGE_SIZE);
	map_size = page_size;

	offset = (unsigned int)(addr & (page_size-1));
    if (offset + 4 > page_size ) {
        // Access straddles page boundary:  add another page:
        map_size += page_size;
    }
//	map_base = addr & ~(page_size - 1);
	map_base = addr & ~((typeof(addr))page_size-1);
//	extra_bytes = addr - map_base;

	mapping = mmap(NULL, map_size, PROT_READ, MAP_SHARED, devmem, map_base);

	if (mapping == MAP_FAILED) {
		perror("Could not map memory");
		goto map_fail;
	}
    fflush(stdout);
	buf = malloc(1000);
	if (buf == NULL) {
		fprintf(stderr, "Failed to allocate memory\n");
		goto alloc_fail;
	}
	
	/*
	 * Using a separate buffer for write stops the kernel from
	 * complaining quite as much as if we passed the mmap()ed
	 * buffer directly to write().
	 */
	virt_addr = mapping+offset;

	buf = bytecopy(buf,virt_addr,50);
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
	munmap(mapping, 50 + offset);

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
