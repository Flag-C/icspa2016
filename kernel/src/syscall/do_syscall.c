#include "irq.h"

#include <sys/syscall.h>

void add_irq_handle(int, void (*)(void));
uint32_t mm_brk(uint32_t);
int fs_ioctl(int, uint32_t, void *);
void serial_printc(char);
int fs_open(const char *pathname, int flags);
int fs_close(int fd);
int fs_read(int fd, void *buf, int len);
int fs_write(int fd, void *buf, int len);
int fs_lseek(int fd, int offset, int whence);

static void sys_brk(TrapFrame *tf) {
	tf->eax = mm_brk(tf->ebx);
}

static void sys_ioctl(TrapFrame *tf) {
	tf->eax = fs_ioctl(tf->ebx, tf->ecx, (void *)tf->edx);
}

void do_syscall(TrapFrame *tf) {
	switch (tf->eax) {
	/* The `add_irq_handle' system call is artificial. We use it to
	 * let user program register its interrupt handlers. But this is
	 * very dangerous in a real operating system. Therefore such a
	 * system call never exists in GNU/Linux.
	 */
	case 0:
		cli();
		add_irq_handle(tf->ebx, (void*)tf->ecx);
		sti();
		break;

	case SYS_brk: sys_brk(tf); break;
	case SYS_ioctl: sys_ioctl(tf); break;

	/* TODO: Add more system calls. */
	case SYS_write: // write()
		//fd:ebx
#ifndef HAS_DEVICE
		if (tf->ebx == 1 || tf->ebx == 2) {
			char *buf = (void *) tf->ecx;
			int len = tf->edx;
			asm volatile (".byte 0xd6" : : "a"(2), "c"(buf), "d"(len));
			tf->eax = len;
			break;
		}
#else
		/*
		if (tf->ebx == 1 || tf->ebx == 2) {
			int count;
			for (count = 0; count < tf->edx; count++)
				serial_printc(((char*)tf->ecx)[count]);
			tf->eax = tf->edx;
			break;
		}
		*/
		tf->eax = fs_write(tf->ebx, (void *)tf->ecx, tf->edx);
		break;
	case SYS_read:
		tf->eax = fs_read(tf->ebx, (void)tf->ecx, tf->edx);
		break;
	case SYS_open:
		tf->eax = fs_open((const char *)tf->ebx, tf->ecx);
		break;
	case SYS_close:
		tf->eax = fs_close(tf->ebx);
		break;
	case SYS_lseek:
		tf->eax = fs_lseek(tf->ebx, tf->ecx, tf->edx);
		break;
#endif
		assert(0);
		break;

	default: panic("Unhandled system call: id = %d, eip = 0x%08x", tf->eax, tf->eip);
	}
}

