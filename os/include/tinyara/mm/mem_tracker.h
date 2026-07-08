/****************************************************************************
 * apps/system/mem_leak_checker/mem_tracker.h
 * Memory capture tracker header
 ****************************************************************************/

#ifndef __MEM_TRACKER_H
#define __MEM_TRACKER_H

#include <tinyara/config.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <tinyara/mm/mm.h>

#ifdef CONFIG_MEM_CAPTURE

#define MEM_TRACKER_HASH_SIZE 2053
#define MEM_TRACKER_HASH(ptr) (((uintptr_t)(ptr) >> 4) % MEM_TRACKER_HASH_SIZE)

struct mem_track_entry {
	void *ptr;
	size_t size;
	mmaddress_t caller_addr;
	pid_t pid;
	bool is_freed;
	struct mem_track_entry *next;
};

struct mem_tracker {
	bool is_active;
	pid_t target_pid;
	struct mem_track_entry **hash_table;
	uint32_t hash_table_size;
	size_t alloc_count;
	size_t free_count;
};

#ifdef __cplusplus
extern "C" {
#endif

int mem_tracker_init(void);
int mem_tracker_start(pid_t target_pid);
int mem_tracker_stop(void);
void mem_tracker_add_allocation(void *ptr, size_t size, mmaddress_t caller_addr, pid_t pid);
void mem_tracker_mark_freed(void *ptr);
void mem_tracker_print_leaks(pid_t pid);
void mem_tracker_clear(void);
bool mem_tracker_is_active(void);
pid_t mem_tracker_get_target_pid(void);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_MEM_CAPTURE */
#endif /* __MEM_TRACKER_H */
