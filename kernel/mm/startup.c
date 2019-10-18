#include <mm/mmu.h>

__attribute__((__aligned__(PAGE_DIR_SIZE)))
unsigned _startup_page_dir[PAGE_DIR_NUM] = {
	2 | (2 << 10) | (0 << 20), 
	2 | (2 << 10) | (1 << 20),
	2 | (2 << 10) | (2 << 20), 
	2 | (2 << 10) | (3 << 20),
	2 | (2 << 10) | (4 << 20), 
	2 | (2 << 10) | (5 << 20),
	2 | (2 << 10) | (6 << 20), 
	2 | (2 << 10) | (7 << 20),
	2 | (2 << 10) | (8 << 20), 
	2 | (2 << 10) | (9 << 20),
	2 | (2 << 10) | (10 << 20), 
	2 | (2 << 10) | (11 << 20),
	2 | (2 << 10) | (12 << 20), 
	2 | (2 << 10) | (13 << 20),
	2 | (2 << 10) | (14 << 20), 
	2 | (2 << 10) | (15 << 20),
	2 | (2 << 10) | (16 << 20), 
	2 | (2 << 10) | (17 << 20),
	2 | (2 << 10) | (18 << 20), 
	2 | (2 << 10) | (19 << 20),
	2 | (2 << 10) | (20 << 20), 
	2 | (2 << 10) | (21 << 20),
	2 | (2 << 10) | (22 << 20), 
	2 | (2 << 10) | (23 << 20),
	2 | (2 << 10) | (24 << 20), 
	2 | (2 << 10) | (25 << 20),
	2 | (2 << 10) | (26 << 20), 
	2 | (2 << 10) | (27 << 20),
	2 | (2 << 10) | (28 << 20), 
	2 | (2 << 10) | (29 << 20),
	2 | (2 << 10) | (30 << 20), 
	2 | (2 << 10) | (31 << 20),

	[KERNEL_BASE >> 20] = 
	2 | (2 << 10) | (0 << 20), 
	2 | (2 << 10) | (1 << 20),
	2 | (2 << 10) | (2 << 20), 
	2 | (2 << 10) | (3 << 20),
	2 | (2 << 10) | (4 << 20),
	2 | (2 << 10) | (5 << 20),
	2 | (2 << 10) | (6 << 20),
	2 | (2 << 10) | (7 << 20),
	2 | (2 << 10) | (8 << 20), 
	2 | (2 << 10) | (9 << 20),
	2 | (2 << 10) | (10 << 20),
	2 | (2 << 10) | (11 << 20),
	2 | (2 << 10) | (12 << 20),
	2 | (2 << 10) | (13 << 20),
	2 | (2 << 10) | (14 << 20),
	2 | (2 << 10) | (15 << 20),
	2 | (2 << 10) | (16 << 20),
	2 | (2 << 10) | (17 << 20),
	2 | (2 << 10) | (18 << 20),
	2 | (2 << 10) | (19 << 20),
	2 | (2 << 10) | (20 << 20),
	2 | (2 << 10) | (21 << 20),
	2 | (2 << 10) | (22 << 20),
	2 | (2 << 10) | (23 << 20),
	2 | (2 << 10) | (24 << 20),
	2 | (2 << 10) | (25 << 20),
	2 | (2 << 10) | (26 << 20),
	2 | (2 << 10) | (27 << 20),
	2 | (2 << 10) | (28 << 20),
	2 | (2 << 10) | (29 << 20),
	2 | (2 << 10) | (30 << 20),
	2 | (2 << 10) | (31 << 20),
};

