#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 33

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for (i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}
/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
	WP *n, *p;
	n = free_;
	if (free_->next != NULL)
		free_ = free_->next;
	else Assert(0, "watchpoint overflow");
	n->next = NULL;
	p = head;
	if (head == NULL) {
		head = n;
		p = head;
	}
	else {
		while (p->next != NULL)
			p = p->next;
		p->next = n;
	}
	return n;
}

void free_wp(WP *wp) {
	WP *f, *p;
	p = free_;
//increase free chain
	if (free_ == NULL) {
		free_ = wp;
		p = free_;
	}
	else {
		while (p->next != NULL) p = p->next;
		p->next = wp;
	}
	wp->next = NULL;
//decrease head chain
	f = head;
	if (head == NULL) Assert(0, "it seems nothing to free");
	if (head->NO == wp->NO) {
		head = head->next;
	}
	else {
		while (f->next != NULL && f->next->NO != wp->NO)
			f = f->next;
		if (f->next->NO == wp->NO)
			f->next = f->next->next;
		else Assert(1, "cannot match");

	}
}

void del_wp(int n) {
	WP *p;
	p = head;
    if (p==NULL) {
        printf("Nothing to delete\n");
        return;
    }
	while (p->NO != n && p->next != NULL)
		p = p->next;
	if (p->NO == n)
		free_wp(p);
	else if (p->next!=NULL && p->next->NO == n)
		free_wp(p->next);
	else printf("No watchpoint number %d\n", n);
}

bool check_wp() {
	WP *p;
	p = head;
	bool flag = true, suc;
	while (p != NULL) {
		uint32_t now = expr(p->expr, &suc);
		if (!suc) Assert(0, "expr can't calculate");
		if (now != p->val)
		{
			flag = false;
			printf("Watchpoint %d:\t%s\nOld value =\t%d\nNew value =\t%d\n", p->NO, p->expr, p->val, now);
			p->val = now;
		}
		p = p->next;
	}
	return flag;
}

void info_wp()
{
	WP *p;
	p = head;
	while (p != NULL) {
		printf("watchpoint %d:\t%s = %d\n", p->NO, p->expr, p->val);
		p = p->next;
	}
}


