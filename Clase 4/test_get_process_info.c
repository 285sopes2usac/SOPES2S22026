#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#define __NR_get_process_info 463
#define GPI_COMM_LEN 16

struct process_info {
	char comm[GPI_COMM_LEN];
	uint32_t pid;
	uint32_t ppid;
	int64_t current_time;
};

int main(void)
{
	struct process_info info;
	long ret;
	time_t raw_time;
	struct tm tm_info;
	char time_buffer[64];

	memset(&info, 0, sizeof(info));

	ret = syscall(__NR_get_process_info, &info);
	if (ret < 0) {
		perror("get_process_info");
		return EXIT_FAILURE;
	}

	raw_time = (time_t)info.current_time;
	if (localtime_r(&raw_time, &tm_info) == NULL) {
		perror("localtime_r");
		return EXIT_FAILURE;
	}

	if (strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &tm_info) == 0) {
		fprintf(stderr, "Error al convertir la hora del kernel\n");
		return EXIT_FAILURE;
	}

	printf("== Informacion obtenida via get_process_info() ==\n");
	printf("comm          : %s\n", info.comm);
	printf("pid           : %u\n", info.pid);
	printf("ppid          : %u\n", info.ppid);
	printf("current_time  : %s (%lld)\n", time_buffer, (long long)info.current_time);

	return EXIT_SUCCESS;
}
