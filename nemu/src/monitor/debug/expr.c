#include "nemu.h"
//#include "elf.c"
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ, NEQ, AND, OR, NUM, HEX, REG, POINTER, NEG, LE, GE, OBJ

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
	int priority;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	//add priority
	{" +",	NOTYPE, 0},				// spaces
	{"\\b[0-9]+\\b", NUM, 0},
	{"\\b0[xX][0-9a-fA-F]+\\b", HEX, 0},
	{"\\$[a-zA-Z]+", REG, 0},
	{"\\b[a-zA-Z_0-9]+", OBJ, 0},	//object
	{"\\+", '+', 4},					// plus
	{"-", '-', 4},
	{"\\*", '*', 5},
	{"/", '/', 5},
	{"==", EQ, 3},					// equal
	{"&&", AND, 2},
	{"\\|\\|", OR, 1},
	{"!=", NEQ, 3},
	{">=", GE, 3},
	{"<=", LE, 3},
	{">", '>', 3},
	{"<", '<', 3},
	{"!", '!', 6},
	{"\\(", '(', 7},
	{"\\)", ')', 7},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX]; //complie the regex

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
	int priority;
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0') {
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i ++) {
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				//Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch (rules[i].token_type) {
				case NOTYPE: break;
				case REG:
					tokens[nr_token].type = rules[i].token_type;
					tokens[nr_token].priority = rules[i].priority;
					strncpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
					tokens[nr_token].str[substr_len - 1] = '\0';
					nr_token++;
					break;
				default:
					tokens[nr_token].type = rules[i].token_type;
					tokens[nr_token].priority = rules[i].priority;
					strncpy(tokens[nr_token].str, substr_start, substr_len);
					tokens[nr_token].str[substr_len] = '\0';
					nr_token++;
				}

				break;
			}
		}

		if (i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}

		//check the unarys
		for (i = 0; i < nr_token; i ++) {
			if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != NUM && tokens[i - 1].type != HEX && tokens[i - 1].type != OBJ && tokens[i - 1].type != REG && tokens[i - 1].type != ')'))) {
				tokens[i].type = POINTER;
				tokens[i].priority = 6;
			}
			if (tokens[i].type == '-' && (i == 0 || (tokens[i - 1].type != NUM && tokens[i - 1].type != HEX && tokens[i - 1].type != OBJ && tokens[i - 1].type != REG && tokens[i - 1].type != ')'))) {
				tokens[i].type = NEG;
				tokens[i].priority = 6;
			}
		}
	}

	return true;
}

bool check_parentheses (int l, int r) { //just count the number
	int i;
	if (tokens[l].type == '(' && tokens[r].type == ')') {
		int lc = 0, rc = 0;
		for (i = l + 1; i < r; i ++) {
			if (tokens[i].type == '(')lc ++;
			if (tokens[i].type == ')')rc ++;
			if (rc > lc) return false; // like (1+2)+(3+4)
		}
		if (lc == rc)return true;
	}
	return false;
}

int dominant_operator (int l, int r)
{
	int i, j, min = 10, op = r;
	for (i = r; i >= l; i--)
	{
		if ((tokens[i].type == NUM) || (tokens[i].type == HEX) || (tokens[i].type == REG) || (tokens[i].type == OBJ))
			continue;

		int lcnt = 0;
		int rcnt = 0;
		for (j = i - 1; j >= l ; j --)
		{
			if (tokens[j].type == '(') lcnt ++;
			if (tokens[j].type == ')') rcnt ++;
		}
		if (lcnt != rcnt) continue;

		if (tokens[i].priority < min)
		{
			min = tokens[i].priority;
			op = i;
		}
	}
	return op;
}

uint32_t eval(int p, int q) {
	if (p > q) {
		Assert(p > q, "Bad expression\n");
	}
	else if (p == q) {
		/* Single token.
		 * For now this token should be a number.
		 * Return the value of the number.
		 */
		uint32_t num = 0;
		switch (tokens[p].type) {
		case NUM: sscanf(tokens[p].str, "%d", &num); break;
		case HEX: sscanf(tokens[p].str, "%x", &num); break;
		case REG:
		{
			if (strlen(tokens[p].str) == 3) {
				int i;
				for (i = R_EAX; i <= R_EDI; i++)
					if (strcmp(tokens[p].str, regsl[i]) == 0) {
						num = reg_l(i);
						break;
					}
				if ((i > R_EDI) && (strcmp(tokens[p].str, "eip") == 0))
					num = cpu.eip;
			}
			if (strlen(tokens[p].str) == 2) {
				if (tokens[q].str[1] == 'x' || tokens[q].str[1] == 'p' || tokens[q].str[1] == 'i') {
					int i;
					for (i = R_AX; i <= R_DI; i ++)
						if (strcmp (tokens[q].str, regsw[i]) == 0) break;
					num = reg_w(i);
				}
			}
			if (tokens[q].str[1] == 'q' || tokens[q].str[1] == 'h') {
				int i;
				for (i = R_AL; i <= R_BH; i ++)
					if (strcmp (tokens[q].str, regsb[i]) == 0)break;
				num = reg_b(i);
			}
			break;
		}
		case OBJ:
		{
			int i = 0;
			for (i = 0; i < nr_symtab_entry; i++)
				if ((symtab[i].st_info == STT_OBJECT))
				{
					char name[256];
					int len = symtab[i + 1].st_name - symtab[i].st_name - 1;
					strncpy(name, strtab + symtab[i].st_name, len);
					name[len] = '\0';
					if (strcmp(name, tokens[p].str) == 0)
						num = symtab[i].st_value;
				}
			break;
		}
		default: Assert(1, "something happened when read a number or reg");
		}
		return num;
	}
	else if (check_parentheses(p, q) == true) {
		/* The expression is surrounded by a matched pair of parentheses.
		 * If that is the case, just throw away the parentheses.
		 */
		return eval(p + 1, q - 1);
	}
	else {
		/* We should do more things here. */
		int op = dominant_operator(p, q);
		if (p == op || tokens[op].type == POINTER || tokens[op].type == NEG || tokens[op].type == '!') //calculate unary
		{
			uint32_t val = eval(p + 1, q);
			switch (tokens[p].type)
			{
			case POINTER: return swaddr_read(val, 4);
			case NEG: return -val;
			case '!': return !val;
			default: Assert(1, "strange expr");
			}
		}
		//time to binary
		uint32_t val1 = eval(p, op - 1);
		uint32_t val2 = eval(op + 1, q);
		switch (tokens[op].type)
		{
		case '+': return val1 + val2;
		case '-': return val1 - val2;
		case '*': return val1 * val2;
		case '/': return val1 / val2;
		case EQ : return val1 == val2;
		case NEQ: return val1 != val2;
		case AND: return val1 && val2;
		case OR : return val1 || val2;
		case '<': return val1 < val2;
		case '>': return val1 > val2;
		case GE : return val1 >= val2;
		case LE : return val1 <= val2;
		default: Assert(1, "unknown operation");
		}
	}
	Assert(1, "unknown expr");
	return -1;
}

uint32_t expr(char *e, bool *success) {
	if (!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	*success = true;
	return eval (0, nr_token - 1);
}

