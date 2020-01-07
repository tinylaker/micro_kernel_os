#include <kernel/hw_info.h>
#include <kernel/system.h>
#include <dev/framebuffer.h>
#include <kstring.h>
#include "mailbox.h"

static hw_info_t _hw_info;

inline void hw_info_init(void) {
	strcpy(_hw_info.machine, "raspi");
	_hw_info.phy_mem_size = 512*MB;
	_hw_info.phy_mmio_base = 0x20000000;
	_hw_info.mmio_size = 4*MB;
}

inline hw_info_t* get_hw_info(void) {
	return &_hw_info;
}

void arch_vm(page_dir_entry_t* vm) {
	map_pages(vm, (uint32_t)_framebuffer_base, V2P(_framebuffer_base), V2P(_framebuffer_end), AP_RW_D);
}
