/****************************************************************************
 * os/mm/mm_heap/mem_tracker.c
 * Memory capture tracker implementation
 ****************************************************************************/

#include <tinyara/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tinyara/mm/mm.h>
#include <tinyara/sched.h>
#include "mem_tracker.h"

#ifdef CONFIG_MEM_CAPTURE

static struct mem_tracker g_tracker;
static bool g_initialized = false;
static bool g_in_tracker = false;  /* Reentrancy guard */

static struct mem_track_entry *mem_tracker_find(void *ptr)
{
	uint32_t hash = MEM_TRACKER_HASH(ptr);
	struct mem_track_entry *entry = g_tracker.hash_table[hash];
	
	while (entry) {
		if (entry->ptr == ptr)
			return entry;
		entry = entry->next;
	}
	return NULL;
}

int mem_tracker_init(void)
{
	size_t size = MEM_TRACKER_HASH_SIZE * sizeof(struct mem_track_entry *);
	g_tracker.hash_table = malloc(size);
	if (!g_tracker.hash_table)
		return -1;
	
	memset(g_tracker.hash_table, 0, size);
	g_tracker.hash_table_size = MEM_TRACKER_HASH_SIZE;
	g_initialized = true;
	return 0;
}

int mem_tracker_start(pid_t target_pid)
{
	if (!g_initialized && mem_tracker_init() < 0)
		return -1;
	
	if (g_tracker.is_active) {
		printf("Tracker already active\n");
		return -1;
	}
	
	mem_tracker_clear();
	g_tracker.is_active = true;
	g_tracker.target_pid = target_pid;
	printf("mem_capture started for PID %d\n", target_pid);
	return 0;
}

int mem_tracker_stop(void)
{
	if (!g_tracker.is_active) {
		printf("Tracker not active\n");
		return -1;
	}
	g_tracker.is_active = false;
	printf("mem_capture stopped\n");
	return 0;
}

void mem_tracker_add_allocation(void *ptr, size_t size, mmaddress_t caller_addr, pid_t pid)
{
	struct mem_track_entry *entry;
	uint32_t hash;
	
	if (!g_tracker.is_active)
		return;
	
	if (g_tracker.target_pid != 0 && g_tracker.target_pid != pid)
		return;
	
	/* Prevent recursion - don't track mallocs made while already in tracker */
	if (g_in_tracker)
		return;
	
	g_in_tracker = true;
	
	entry = malloc(sizeof(struct mem_track_entry));
	
	g_in_tracker = false;
	
	if (!entry)
		return;
	
	entry->ptr = ptr;
	entry->orig_size = size;
	entry->caller_addr = caller_addr;
	entry->pid = pid;
	entry->is_freed = false;
	
	hash = MEM_TRACKER_HASH(ptr);
	entry->next = g_tracker.hash_table[hash];
	g_tracker.hash_table[hash] = entry;
	g_tracker.alloc_count++;
}

void mem_tracker_mark_freed(void *ptr)
{
	struct mem_track_entry *entry;
	
	if (!g_tracker.is_active)
		return;
	
	/* Prevent recursion */
	if (g_in_tracker)
		return;
	
	g_in_tracker = true;
	
	entry = mem_tracker_find(ptr);
	if (entry) {
		entry->is_freed = true;
		g_tracker.free_count++;
	}
	
	g_in_tracker = false;
}

void mem_tracker_print_leaks(pid_t pid)
{
	uint32_t i;
	struct mem_track_entry *entry;
	int leak_count = 0;
	char task_name[CONFIG_TASK_NAME_SIZE + 1];
	
	printf("\n===== Memory Capture Report for PID %d =====\n", pid);
	
	if (prctl(PR_GET_NAME_BYPID, task_name, pid) < 0)
		strncpy(task_name, "UNKNOWN", CONFIG_TASK_NAME_SIZE);
	
	printf("Task: %s\n\n", task_name);
	printf("Owner Addr   | Size  | Ptr\n");
	printf("-------------|-------|------------\n");
	
	for (i = 0; i < g_tracker.hash_table_size; i++) {
		entry = g_tracker.hash_table[i];
		while (entry) {
			if (!entry->is_freed) {
				printf("0x%-10lx | %-5zu | %p\n",
					   (unsigned long)(uintptr_t)entry->caller_addr,
					   entry->orig_size, entry->ptr);
				leak_count++;
			}
			entry = entry->next;
		}
	}
	
	printf("\nTotal leaks: %d\n", leak_count);
	printf("========================================\n");
}

void mem_tracker_clear(void)
{
	uint32_t i;
	struct mem_track_entry *entry, *next;
	
	for (i = 0; i < g_tracker.hash_table_size; i++) {
		entry = g_tracker.hash_table[i];
		while (entry) {
			next = entry->next;
			free(entry);
			entry = next;
		}
		g_tracker.hash_table[i] = NULL;
	}
	
	g_tracker.alloc_count = 0;
	g_tracker.free_count = 0;
}

bool mem_tracker_is_active(void)
{
	return g_tracker.is_active;
}

pid_t mem_tracker_get_target_pid(void)
{
	return g_tracker.target_pid;
}

#endif /* CONFIG_MEM_CAPTURE */
