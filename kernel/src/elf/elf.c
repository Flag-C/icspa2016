#include "common.h"
#include "memory.h"
#include <string.h>
#include <elf.h>

#define ELF_OFFSET_IN_DISK 0

#ifdef HAS_DEVICE
void ide_read(uint8_t *, uint32_t, uint32_t);
#else
void ramdisk_read(uint8_t *, uint32_t, uint32_t);
#endif

#define STACK_SIZE (1 << 20)

void create_video_mapping();
uint32_t get_ucr3();

uint32_t loader() {
	Elf32_Ehdr *elf;
	Elf32_Phdr *ph = NULL;

	uint8_t buf[4096];

	uint8_t page_buf[4096];

#ifdef HAS_DEVICE
	ide_read(buf, ELF_OFFSET_IN_DISK, 4096);
#else
	ramdisk_read(buf, ELF_OFFSET_IN_DISK, 4096);
#endif

	elf = (void*)buf;

	/* TODO: fix the magic number with the correct one */
	const uint32_t elf_magic = 0x464C457F;
	uint32_t *p_magic = (void *)buf;
	nemu_assert(*p_magic == elf_magic);

	/* Load each program segment */
	//panic("please implement me");
	int index;
	for (index = 0; index < elf->e_phnum; index++ ) {
		/* Scan the program header table, load each segment into memory */
		ph = (void*)elf->e_phoff + index * elf->e_phentsize;
		if (ph->p_type == PT_LOAD) {

			/* TODO: read the content of the segment from the ELF file
			 * to the memory region [VirtAddr, VirtAddr + FileSiz)
			 */
#ifdef IA32_PAGE
			uint32_t virt_addr = ph->p_vaddr;
			int loaded = 0;
			while (virt_addr < (ph->p_vaddr + ph->p_memsz)) {
				//malloc a page
				uint32_t offset = virt_addr & 0xfff;
				virt_addr &= 0xfffff000;
				uint32_t addr = mm_malloc(virt_addr, 4096);
				memset(page_buf, 0, sizeof(page_buf)); //use buffer to save page temporary
				uint32_t load_num = 4096 - offset;
				if ((ph->p_filesz - loaded) < load_num)
					load_num = ph->p_filesz - loaded;
#ifndef HAS_DEVICE
				ramdisk_read((void *)(page_buf + offset), ELF_OFFSET_IN_DISK + ph->p_offset + loaded, load_num);
#else
				ide_read((void *)(page_buf + offset), ELF_OFFSET_IN_DISK + ph->p_offset + loaded, load_num);
#endif
				memcpy((void *)addr, page_buf, 4096);
				virt_addr += 4096;
				loaded += load_num;

			}
#endif
#ifndef IA32_PAGE

#ifndef HAS_DEVICE
			ramdisk_read((void*)ph->p_vaddr, ELF_OFFSET_IN_DISK + ph->p_offset, ph->p_filesz);
#else
			ide_read((void*)ph->p_vaddr, ELF_OFFSET_IN_DISK + ph->p_offset, ph->p_filesz);
#endif
			/* TODO: zero the memory region
			 * [VirtAddr + FileSiz, VirtAddr + MemSiz)
			 */
			memset((void*)ph->p_vaddr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
#endif

#ifdef IA32_PAGE
			/* Record the program break for future use. */
			extern uint32_t cur_brk, max_brk;
			uint32_t new_brk = ph->p_vaddr + ph->p_memsz - 1;
			if (cur_brk < new_brk) { max_brk = cur_brk = new_brk; }
#endif
		}
	}

	volatile uint32_t entry = elf->e_entry;

#ifdef IA32_PAGE
	mm_malloc(KOFFSET - STACK_SIZE, STACK_SIZE);

#ifdef HAS_DEVICE
	create_video_mapping();
#endif

	write_cr3(get_ucr3());
#endif

	return entry;
}
