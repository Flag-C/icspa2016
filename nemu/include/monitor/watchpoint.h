#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	char* expr;
	uint32_t val;
	/* TODO: Add more members if necessary */


} WP;

void init_wp_pool();
WP* new_wp();
void free_wp(WP *wp);
void del_wp(int n);
bool check_wp();
void info_wp();
#endif
