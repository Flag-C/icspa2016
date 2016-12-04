#include "common.h"
#include <sys/ioctl.h>
#include "string.h"

typedef struct {
	char *name;
	uint32_t size;
	uint32_t disk_offset;
} file_info;

enum {SEEK_SET, SEEK_CUR, SEEK_END};

/* This is the information about all files in disk. */
static const file_info file_table[] __attribute__((used)) = {
	{"1.rpg", 188864, 1048576}, {"2.rpg", 188864, 1237440},
	{"3.rpg", 188864, 1426304}, {"4.rpg", 188864, 1615168},
	{"5.rpg", 188864, 1804032}, {"abc.mkf", 1022564, 1992896},
	{"ball.mkf", 134704, 3015460}, {"data.mkf", 66418, 3150164},
	{"desc.dat", 16027, 3216582}, {"fbp.mkf", 1128064, 3232609},
	{"fire.mkf", 834728, 4360673}, {"f.mkf", 186966, 5195401},
	{"gop.mkf", 11530322, 5382367}, {"map.mkf", 1496578, 16912689},
	{"mgo.mkf", 1577442, 18409267}, {"m.msg", 188232, 19986709},
	{"mus.mkf", 331284, 20174941}, {"pat.mkf", 8488, 20506225},
	{"rgm.mkf", 453202, 20514713}, {"rng.mkf", 4546074, 20967915},
	{"sss.mkf", 557004, 25513989}, {"voc.mkf", 1997044, 26070993},
	{"wor16.asc", 5374, 28068037}, {"wor16.fon", 82306, 28073411},
	{"word.dat", 5650, 28155717},
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

int fs_ioctl(int fd, uint32_t request, void *p) {
	assert(request == TCGETS);
	return (fd >= 0 && fd <= 2 ? 0 : -1);
}

void ide_read(uint8_t *, uint32_t, uint32_t);
void ide_write(uint8_t *, uint32_t, uint32_t);

/* TODO: implement a simplified file system here. */

typedef struct {
	bool opened;
	uint32_t offset;
} Fstate;

#define FILE_START 3
#define FILE_END (NR_FILES+3)
#define FILE_TABLE(fd) file_table[(fd) - FILE_START]

static Fstate file_state[NR_FILES + 3];

int fs_open(const char *pathname, int flags)
{
	int i;
	for (i = FILE_START; i < FILE_END; i++)
		if (strcmp(FILE_TABLE(i).name, pathname) == 0)
		{
			file_state[i].opened = true;
			file_state[i].offset = 0;
			return i;
		}
	assert(0);
	return -1;
}

int fs_close(int fd)
{
	assert((fd >= FILE_START) && (fd < FILE_END));
	file_state[fd].opened = false;
	file_state[fd].offset = 0;
	return 0;
}

int fs_read(int fd, void *buf, int len)
{
	assert((fd >= FILE_START) && (fd < FILE_END));
	assert(len >= 0);
	assert(file_state[fd].opened);
	int n = (len < (FILE_TABLE(fd).size - file_state[fd].offset)) ?
	        len : (FILE_TABLE(fd).size - file_state[fd].offset);
	ide_read(buf, FILE_TABLE(fd).disk_offset + file_state[fd].offset, n);
	file_state[fd].offset += n;
	return n;
}

extern void serial_printc(char);

int fs_write(int fd, void *buf, int len)
{
	assert(fd < FILE_END);
	assert(len >= 0);
	if (fd == 1 || fd == 2) {
		int i;
		for (i = 0; i < len; i++)
			serial_printc(*(char*)(buf + i));
		return len;
	}
	else
	{
		int n = (len < (FILE_TABLE(fd).size - file_state[fd].offset)) ?
		        len : (FILE_TABLE(fd).size - file_state[fd].offset);
		ide_write(buf, FILE_TABLE(fd).disk_offset + file_state[fd].offset, n);
		file_state[fd].offset += n;
		return n;
	}
}

int fs_lseek(int fd, int offset, int whence)
{
	assert((fd >= FILE_START) && (fd < FILE_END));
	int set_offset = 0;
	switch (whence)
	{
	case SEEK_SET:
		set_offset = offset;
		break;
	case SEEK_CUR:
		set_offset = file_state[fd].offset + offset;
		break;
	case SEEK_END:
		//Log("hit SEEK");
		set_offset = FILE_TABLE(fd).size + offset;
		break;
	default: assert(0);
	}
	file_state[fd].offset = set_offset;
	return set_offset;
}