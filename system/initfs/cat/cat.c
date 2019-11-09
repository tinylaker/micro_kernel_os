#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cmain.h>
#include <string.h>
#include <fcntl.h>
#include <vfs.h>
#include <svc_call.h>

int main(int argc, char** argv) {
	if(argc != 2) {
		printf("  Usage: cat <file>\n");
		return -1;
	}

	int fd = open(argv[1], 0);
	if(fd < 0) {
		printf("'%s' not found!\n", argv[1]);
		return -1;
	}

	while(1) {
		char buf[128];
		int sz = read(fd, buf, 128);
		if(sz <= 0)
			break;
		write(1, buf, sz);
	}

	return 0;
}
