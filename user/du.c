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

int
du(char *path)
{
	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;
	int sum = 0;

	if((fd = open(path, 0x010)) < 0){
		fprintf(2, "du: cannot open %s\n", path);
		return;
	}

	if(fstat(fd, &st) < 0){
		fprintf(2, "du: cannot stat %s\n", path);
		close(fd);
		return;
	}

	switch(st.type){
	case T_FILE:
		printf("%s %d\n", fmtname(path), st.blocks);
		sum += st.blocks;
		break;

	case T_DIR:
		if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
			printf("du: path too long\n");
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
				printf("du: cannot stat %s\n", buf);
				continue;
			}

			char *last;
			// Find first character after last slash.
			for(last=buf+strlen(buf); last >= buf && *last != '/'; last--)
				;
			last++;


			char *la;
			for(la=path+strlen(path); la >= path && *la != '/'; la--)
				;
			la++;

			if (strcmp(last, ".") == 0 && strcmp(path, la) == 0)
			{
				if (!(strcmp(last, ".") == 0 && strcmp(path, ".") == 0 && strcmp(la, ".") == 0))
				{
					printf("%s %d\n", fmtname(path), st.blocks);
					sum += st.blocks;
				}
			}
			if (strcmp(last, ".") != 0 && strcmp(last, "..") != 0)
			{
				printf("%s %d\n", fmtname(buf), st.blocks);
				sum += st.blocks;
				if (st.type == 1) {
					sum += du(buf);
				}
			}

		}
		break;

	case T_SYMLINK:
		printf("%s %d\n", fmtname(path), st.blocks);
		sum += st.blocks;
		break;
	}
	close(fd);
	return sum;
}

int
main(int argc, char *argv[])
{
	int i;
	int total = 0;
	if(argc < 2){
		total += du(".");
		printf("total = %d\n",total);
		exit();
	}

	for(i=1; i<argc; i++)
		total += du(argv[i]);
	printf("total = %d\n",total);
	exit();
}
