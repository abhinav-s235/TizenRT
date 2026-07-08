/****************************************************************************
 * apps/system/mem_leak_checker/mem_leak_checker_main.c
 * Main entry for mem_leak command and mem_capture commands
 ****************************************************************************/
#include <tinyara/config.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/types.h>

#ifdef CONFIG_MEM_CAPTURE
extern int mem_capture_main(int argc, char **argv);
#endif

int mem_leak_checker_main(int argc, char **argv)
{
	/* Handle mem_capture_start and mem_capture_stop commands */
#ifdef CONFIG_MEM_CAPTURE
	if (argc >= 2) {
		if (strcmp(argv[1], "capture_start") == 0 ||
		    strcmp(argv[1], "capture_stop") == 0) {
			return mem_capture_main(argc, argv);
		}
	}
#endif

	/* Original mem_leak checker */
	int ret;
	ret = prctl(PR_MEM_LEAK_CHECKER, getpid());
	if (ret < 0) {
		printf("Fail to launch MEMORY LEAK CHECKER.\n");
		return -1;
	}

	return 0;
}
