#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

typedef struct {
	uint32_t prev_ebp;
	uint32_t ret_addr;
	uint32_t args[4];
} PartOfStackFrame;

void cpu_exec(uint32_t);

extern void print_cache(swaddr_t addr);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_p(char *args);

static int cmd_x(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static int cmd_bt(char *args);

static int cmd_cache(char *args);


static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c},
	{ "q", "Exit NEMU", cmd_q},
	{ "si", "step through", cmd_si},
	{ "info", "print regInfo or watchPointInfo", cmd_info},
	{ "x", "Scan memory", cmd_x},
	{ "p", "expression evaluation", cmd_p},
	{ "w", "set watchpoint", cmd_w},
	{ "d", "delete watchpoint according to the number", cmd_d},
	{ "bt", "print the stack frame", cmd_bt},
	{ "cache", "print the cache of a address", cmd_cache}
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_cache(char *args) {
#ifndef CACHED
	printf("not cached\n");
	return 0;
#endif
	swaddr_t addr;
	sscanf(args, "%d", &addr);
	print_cache(addr);
	return 0;
}

static int cmd_bt(char *args) {
	PartOfStackFrame now;
	int count = 0, len;
	char name[32];
	uint32_t addr = reg_l(R_EBP);
	now.ret_addr = cpu.eip;
	now.prev_ebp = 0;
	while (addr > 0)
	{
		printf("%d 0x%08x in ", count++, now.ret_addr);
		int i;
		for (i = 0; i < nr_symtab_entry - 1; i++)
			if (symtab[i].st_value <= now.ret_addr
			        && symtab[i].st_value + symtab[i].st_size >= now.ret_addr
			        && (symtab[i].st_info & 0xf) == STT_FUNC)
			{
				len = symtab[i + 1].st_name - symtab[i].st_name - 1;
				strncpy (name, strtab + symtab[i].st_name, len);
				strcat(name, "\0");
				break;
			}
		i = nr_symtab_entry - 1;
		if (symtab[i].st_value <= now.ret_addr
		        && symtab[i].st_value + symtab[i].st_size >= now.ret_addr
		        && (symtab[i].st_info & 0xf) == STT_FUNC)
		{
			strcpy(name, strtab + symtab[i].st_name);
			strcat(name, "\0");
		}
		printf("%s", name);
		now.prev_ebp = swaddr_read(addr, 4);
		now.ret_addr = swaddr_read(addr + 4, 4);
		for (i = 0; i < 4; i ++)
			now.args[i] = swaddr_read(addr + 8 + 4 * i , 4);
		if (strcmp (name, "main") == 0)printf ("( )\n");
		else printf ("( %d , %d , %d , %d )\n", now.args[0], now.args[1], now.args[2], now.args[3]);
		addr = now.prev_ebp;
	}
	return 0;
}

static int cmd_d(char *args) {
	int n;
	sscanf(args, "%d", &n);
	del_wp(n);
	return 0;
}

static int cmd_w(char *args) {
	uint32_t num;
	bool succ;
	num = expr(args, &succ);
	if (succ)
	{
		WP *wp;
		wp = new_wp();
		Log("newed");
		strcpy(wp->expr, args);
		wp->val = num;
		printf("watchpoint %d: %s\n", wp->NO, wp->expr);
	}
	else Assert(1, "cannot calculate");
	return 0;
}

static int cmd_p(char *args) {
	uint32_t num;
	bool succ;
	num = expr(args, &succ);
	if (succ)
		printf("0x%x:\t%d\n", num, num);
	else Assert(1, "print error");
	return 0;
}

static int cmd_x(char * args) {
	char* arg = strtok(NULL, " ");
	int n;
	if (arg == NULL)
		printf("please input rightly:x N EXPR\n");
	else if (sscanf(arg, "%d", &n) == -1)
		printf("please input rightly:x N EXPR\n");
	else {
		char *arg1 = strtok(NULL, " ");
		if (arg == NULL) {
			printf("please input rightly:x N EXPR\n");
		}
		else {
			uint32_t addressStart;
			bool succ;
			addressStart = expr(arg1, &succ);
			int i;
			for (i = 1; i <= n; i++) {
				printf("%d.\t0x%x : 0x%08x\n", i, addressStart, swaddr_read(addressStart, 4) );
				addressStart += 4;
			}


		}
	}
	return 0;
}

static int cmd_info(char* args) {
	char* arg = strtok(NULL, " ");
	if (strcmp(arg, "r") == 0) {
		int i = R_EAX;
		for ( ; i < R_EDI; i++) {
			printf(" %s : 0x%x\n", regsl[i], cpu.gpr[i]._32);
		}
	}
	else if (strcmp(arg, "w") == 0) {
		info_wp();
	}
	else printf("please input right argment\n");
	return 0;
}

static int cmd_si(char* args) {
	char *arg = strtok(NULL, " ");
	int n;
	if (arg == NULL)
		cpu_exec(1);
	else {
		if (sscanf(arg, "%d", &n) == -1) {
			printf("Can't read number\n");
		}
		else {
			cpu_exec(n);
		}
	}
	return 0;
}

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if (arg == NULL) {
		/* no argument given */
		for (i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for (i = 0; i < NR_CMD; i ++) {
			if (strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while (1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if (cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if (args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for (i = 0; i < NR_CMD; i ++) {
			if (strcmp(cmd, cmd_table[i].name) == 0) {
				if (cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
