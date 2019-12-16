#ifndef HW_INFO_H
#define HW_INFO_H

#include <types.h>
#include <mm/mmu.h>

typedef struct {
	uint32_t phy_mem_size;
	uint32_t phy_mmio_base;
	uint32_t mmio_size;
} hw_info_t;

extern hw_info_t _hw_info;

extern void hw_info_init(void);
extern void arch_vm(page_dir_entry_t* vm);

#endif
