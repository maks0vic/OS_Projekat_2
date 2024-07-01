#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
	int i;
	if (symlink(argv[1], argv[2]) != -1) {
		printf("%s -> %s\n", argv[2], argv[1]);
	}
	else {
		printf("Failed\n");
	}
	exit();
}
