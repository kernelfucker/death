#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#define SECTOR_SIZE 512
#define INTERVAL 1

typedef struct{
	unsigned long long read_bytes;
	unsigned long long write_bytes;
} IOStat;

void get_io(const char *devname, IOStat *stat){
	FILE *fp = fopen("/proc/diskstats", "r");
	if(!fp){
		perror("cant read /proc/diskstats");
		exit(1);
	}

	char line[512];
	while(fgets(line, sizeof(line), fp)){
		char dev[32];
		unsigned long rd_sec, wr_sec;

		int matched = sscanf(line, "%*d %*d %31s %*u %*u %*u %*u %*u %*u %llu %llu", dev, &rd_sec, &wr_sec);
		if(matched == 3 && strcmp(dev, devname) == 0){
			stat->read_bytes = (unsigned long long)rd_sec * SECTOR_SIZE;
			stat->write_bytes = (unsigned long long)wr_sec * SECTOR_SIZE;
			fclose(fp);
			return;
		}
	}

	fclose(fp);
	stat->read_bytes = 0;
	stat->write_bytes = 0;
}

int main(int argc, char *argv[]){
	if(argc != 2){
		fprintf(stderr, "usage: %s /dev/sdX\n", argv[0]);
		exit(1);
	}

	const char *devpath = argv[1];
	const char *devname = basename((char *)devpath);

	IOStat prev = {0}, curr = {0};
	get_io(devname, &prev);

	while(1){
		sleep(INTERVAL);
		get_io(devname, &curr);

		unsigned long long r = curr.read_bytes - prev.read_bytes;
		unsigned long long w = curr.write_bytes - prev.write_bytes;

		printf("\033[2J\033[H");
		printf("device: %s\n", devname);
		printf("----------------\n");
		printf("read: %.2f kb/s\n", r / 1024.0);
		printf("write: %.2f kb/s\n", w / 1024.0);

		prev = curr;
	}
}
