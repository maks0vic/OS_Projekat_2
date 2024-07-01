#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
	static char buf[DIRSIZ+1];
	char *p;

	// Find first character after last slash.
	for(p=path+strlen(path); p >= path && *p != '/'; p--)
		;
	p++;

	// Return blank-padded name.
	if(strlen(p) >= DIRSIZ)
		return p;
	memmove(buf, p, strlen(p));
	memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
	return buf;
}



void
macka(char *buf)
{
	int n;
	int fd;
	char bufer[512];
	if((fd = open(buf, 0x010)) < 0){
		printf("symlinkinfo: cannot open %s\n", buf);
		exit();
	}

	if ((n = read(fd, bufer, sizeof(bufer))) < 0){
		printf("symlinkinfo: read error\n");
		close(fd);
		exit();
	}

	printf("%s -> %s\n", fmtname(buf), bufer);
	close(fd);
}

void
info(char *path)
{
	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;

	if((fd = open(path, 0x010)) < 0){
		fprintf(2, "symlinkinfo: cannot open %s\n", path);
		return;
	}

	if(fstat(fd, &st) < 0){
		fprintf(2, "symlinkinfo: cannot stat %s\n", path);
		close(fd);
		return;
	}

	switch(st.type){
	case T_FILE:
		break;
		printf("%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
		// printf("%s \n", fmtname(path));
		break;

	case T_DIR:
		if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
			printf("symlinkinfo: path too long\n");
			break;
		}
		// printf("path %s\n", path);
		strcpy(buf, path);
		p = buf+strlen(buf);
		*p++ = '/';
		while(read(fd, &de, sizeof(de)) == sizeof(de)){
			if(de.inum == 0)
				continue;
			memmove(p, de.name, DIRSIZ);
			p[DIRSIZ] = 0;
			// printf("buf:%s\n",buf);
			if(stat(buf, &st) < 0){
				printf("symlinkinfo: cannot stat %s\n", buf);
				continue;
			}
			if (st.type == 4)
			{
				// printf("%s %d %d %d \n", buf, st.type, st.ino, st.size);
				macka(buf);
			}
		}
		break;

	case T_SYMLINK:
		// printf("symlink\n");
		// printf("%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
		macka(path);
		break;
	}
	close(fd);
}

int
main(int argc, char *argv[])
{
	int i;

	if(argc < 2){
		info(".");
		exit();
	}
	for(i=1; i<argc; i++)
		info(argv[i]);
	exit();
}
