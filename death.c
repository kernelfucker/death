/* See LICENSE file for license details */
/* death - minimal disk input/output viewer */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#define sector_size 512
#define interval 1

#define version "0.2"

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

		int matched = sscanf(line, "%*d %*d %31s %*u %*u %*u %*u %*u %*u %lu %lu", dev, &rd_sec, &wr_sec);
		if(matched == 3 && strcmp(dev, devname) == 0){
			stat->read_bytes = (unsigned long long)rd_sec * sector_size;
			stat->write_bytes = (unsigned long long)wr_sec * sector_size;
			fclose(fp);
			return;
		}
	}

	fclose(fp);
	stat->read_bytes = 0;
	stat->write_bytes = 0;
}

void help(const char *death){
	printf("usage: %s [options]..\n", death);
	printf("options:\n");
	printf("  -v	show version information\n");
	printf("  -h	display this\n");
	exit(1);
}

void show_version(){
	printf("death-%s\n", version);
	exit(1);
}

int main(int argc, char *argv[]){
	if(argc == 2){
		if(strcmp(argv[1], "-h") == 0){
			help(argv[0]);
		}

		if(strcmp(argv[1], "-v") == 0){
			show_version();
		}
	}

	if(argc != 2){
		fprintf(stderr, "usage: %s /dev/sdX\n", argv[0]);
		fprintf(stderr, "try '%s -h' for more information\n", argv[0]);
		exit(1);
	}

	const char *devpath = argv[1];
	const char *devname = basename((char *)devpath);

	IOStat prev = {0}, curr = {0};
	get_io(devname, &prev);

	while(1){
		sleep(interval);
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
