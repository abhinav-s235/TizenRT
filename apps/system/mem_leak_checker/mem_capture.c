/****************************************************************************
 * apps/system/mem_leak_checker/mem_capture.c
 * mem_capture_start and mem_capture_stop commands
 ****************************************************************************/

#include <tinyara/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef CONFIG_MEM_CAPTURE

#include <tinyara/mm/mem_tracker.h>

static int do_mem_capture_start(int argc, char **argv)
{
	pid_t pid;
	char *endptr;

	if (argc < 2) {
		printf("Usage: mem_capture_start <pid>\n");
		return -1;
	}

	pid = (pid_t)strtol(argv[1], &endptr, 10);
	if (*endptr != '\0' || pid <= 0) {
		printf("Invalid PID: %s\n", argv[1]);
		return -1;
	}

	return mem_tracker_start(pid);
}

static int do_mem_capture_stop(int argc, char **argv)
{
	pid_t pid;
	char *endptr;

	if (argc < 2) {
		printf("Usage: mem_capture_stop <pid>\n");
		return -1;
	}

	pid = (pid_t)strtol(argv[1], &endptr, 10);
	if (*endptr != '\0' || pid <= 0) {
		printf("Invalid PID: %s\n", argv[1]);
		return -1;
	}

	if (mem_tracker_get_target_pid() != pid) {
		printf("Tracker not active for PID %d\n", pid);
		return -1;
	}

	mem_tracker_stop();
	mem_tracker_print_leaks(pid);
	mem_tracker_clear();

	return 0;
}

int mem_capture_main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage:\n");
		printf("  mem_capture_start <pid>  - Start capturing allocations for PID\n");
		printf("  mem_capture_stop  <pid>  - Stop and print unfreed allocations\n");
		return -1;
	}

	/* Handle both "start" and "capture_start" formats */
	if (strcmp(argv[1], "start") == 0 || strcmp(argv[1], "capture_start") == 0) {
		return do_mem_capture_start(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "stop") == 0 || strcmp(argv[1], "capture_stop") == 0) {
		return do_mem_capture_stop(argc - 1, argv + 1);
	} else {
		printf("Unknown command: %s\n", argv[1]);
		return -1;
	}
}

#else

int mem_capture_main(int argc, char **argv)
{
	printf("MEM_CAPTURE not enabled\n");
	return -1;
}

#endif
