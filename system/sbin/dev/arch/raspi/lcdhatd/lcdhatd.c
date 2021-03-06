#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cmain.h>
#include <string.h>
#include <shm.h>
#include <vfs.h>
#include "../lib/gpio_arch.h"
#include "../lib/spi_arch.h"
#include <vdevice.h>
#include <syscall.h>
#include <sys/critical.h>
#include <dev/device.h>
#include <graph/graph.h>


#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

#define LCD_CS   8
#define LCD_RST  27
#define LCD_DC   25
#define LCD_BL   24

#define DEV_Delay_ms(x) usleep((x)*1000)
#define DEV_Digital_Write gpio_arch_write

#define LCD_CS_0		DEV_Digital_Write(LCD_CS,0)
#define LCD_CS_1		DEV_Digital_Write(LCD_CS,1)

#define LCD_RST_0		DEV_Digital_Write(LCD_RST,0)
#define LCD_RST_1		DEV_Digital_Write(LCD_RST,1)

#define LCD_DC_0		DEV_Digital_Write(LCD_DC,0)
#define LCD_DC_1		DEV_Digital_Write(LCD_DC,1)

#define LCD_BL_0		DEV_Digital_Write(LCD_BL,0)
#define LCD_BL_1		DEV_Digital_Write(LCD_BL,1)

#define LCD_HEIGHT 240
#define LCD_WIDTH 240

#define LCD_WIDTH_Byte 240

#define HORIZONTAL 0
#define VERTICAL   1

typedef struct{
	UWORD WIDTH;
	UWORD HEIGHT;
	UBYTE SCAN_DIR;
}LCD_ATTRIBUTES;
static LCD_ATTRIBUTES LCD;

static inline void DEV_SPI_Write(UBYTE* data, uint32_t sz) {
	spi_arch_activate(1);
	for(uint32_t i=0; i<sz; i++)
		spi_arch_transfer(data[i]);
	spi_arch_activate(0);
}

/******************************************************************************
function :	Hardware reset
parameter:
 ******************************************************************************/
static void LCD_Reset(void) {
	LCD_RST_1;
	DEV_Delay_ms(100);
	LCD_RST_0;
	DEV_Delay_ms(100);
	LCD_RST_1;
	DEV_Delay_ms(100);
}

/******************************************************************************
function :	send command
parameter:
Reg : Command register
 ******************************************************************************/
static void LCD_SendCommand(UBYTE Reg) {
	LCD_DC_0;
	// LCD_CS_0;
	DEV_SPI_Write(&Reg, 1);
	// LCD_CS_1;
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
 ******************************************************************************/
static void LCD_SendData_8Bit(UBYTE Data) {
	LCD_DC_1;
	// LCD_CS_0;
	DEV_SPI_Write(&Data, 1);
	// LCD_CS_1;
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
 ******************************************************************************/
/*static void LCD_SendData_16Bit(UWORD Data) {
	LCD_DC_1;
	// LCD_CS_0;
	DEV_SPI_WriteByte((Data >> 8) & 0xFF);
	DEV_SPI_WriteByte(Data & 0xFF);
	// LCD_CS_1;
}
*/

/********************************************************************************
function:	Set the resolution and scanning method of the screen
parameter:
		Scan_dir:   Scan direction
 ********************************************************************************/
static void LCD_SetAttributes(UBYTE Scan_dir) {
	//Get the screen scan direction
	LCD.SCAN_DIR = Scan_dir;
	UBYTE MemoryAccessReg = 0x00;

	//Get GRAM and LCD width and height
	if(Scan_dir == HORIZONTAL) {
		LCD.HEIGHT	= LCD_HEIGHT;
		LCD.WIDTH   = LCD_WIDTH;
		MemoryAccessReg = 0X70;
	} else {
		LCD.HEIGHT	= LCD_WIDTH;
		LCD.WIDTH   = LCD_HEIGHT;
		MemoryAccessReg = 0X00;
	}

	// Set the read / write scan direction of the frame memory
	LCD_SendCommand(0x36); //MX, MY, RGB mode
	LCD_SendData_8Bit(MemoryAccessReg);	//0x08 set RGB
}

/******************************************************************************
function :	Initialize the lcd register
parameter:
 ******************************************************************************/
static void LCD_InitReg(void) {
	LCD_SendCommand(0x11); 
	DEV_Delay_ms(120);
	// LCD_SendCommand(0x36);
	// LCD_SendData_8Bit(0x00);

	LCD_SendCommand(0x3A); 
	LCD_SendData_8Bit(0x05);

	LCD_SendCommand(0xB2);
	LCD_SendData_8Bit(0x0C);
	LCD_SendData_8Bit(0x0C);
	LCD_SendData_8Bit(0x00);
	LCD_SendData_8Bit(0x33);
	LCD_SendData_8Bit(0x33); 

	LCD_SendCommand(0xB7); 
	LCD_SendData_8Bit(0x35);  

	LCD_SendCommand(0xBB);
	LCD_SendData_8Bit(0x37);

	LCD_SendCommand(0xC0);
	LCD_SendData_8Bit(0x2C);

	LCD_SendCommand(0xC2);
	LCD_SendData_8Bit(0x01);

	LCD_SendCommand(0xC3);
	LCD_SendData_8Bit(0x12);   

	LCD_SendCommand(0xC4);
	LCD_SendData_8Bit(0x20);  

	LCD_SendCommand(0xC6); 
	LCD_SendData_8Bit(0x0F);    

	LCD_SendCommand(0xD0); 
	LCD_SendData_8Bit(0xA4);
	LCD_SendData_8Bit(0xA1);

	LCD_SendCommand(0xE0);
	LCD_SendData_8Bit(0xD0);
	LCD_SendData_8Bit(0x04);
	LCD_SendData_8Bit(0x0D);
	LCD_SendData_8Bit(0x11);
	LCD_SendData_8Bit(0x13);
	LCD_SendData_8Bit(0x2B);
	LCD_SendData_8Bit(0x3F);
	LCD_SendData_8Bit(0x54);
	LCD_SendData_8Bit(0x4C);
	LCD_SendData_8Bit(0x18);
	LCD_SendData_8Bit(0x0D);
	LCD_SendData_8Bit(0x0B);
	LCD_SendData_8Bit(0x1F);
	LCD_SendData_8Bit(0x23);

	LCD_SendCommand(0xE1);
	LCD_SendData_8Bit(0xD0);
	LCD_SendData_8Bit(0x04);
	LCD_SendData_8Bit(0x0C);
	LCD_SendData_8Bit(0x11);
	LCD_SendData_8Bit(0x13);
	LCD_SendData_8Bit(0x2C);
	LCD_SendData_8Bit(0x3F);
	LCD_SendData_8Bit(0x44);
	LCD_SendData_8Bit(0x51);
	LCD_SendData_8Bit(0x2F);
	LCD_SendData_8Bit(0x1F);
	LCD_SendData_8Bit(0x1F);
	LCD_SendData_8Bit(0x20);
	LCD_SendData_8Bit(0x23);

	LCD_SendCommand(0x21); 

	LCD_SendCommand(0x29);
}

/********************************************************************************
function :	Initialize the lcd
parameter:
 ********************************************************************************/
static void LCD_1in3_Init(UBYTE Scan_dir) {
	//Turn on the backlight
	LCD_BL_1;

	//Hardware reset
	LCD_Reset();
	//Set the resolution and scanning method of the screen
	LCD_SetAttributes(Scan_dir);

	//Set the initialization register
	LCD_InitReg();
}

/********************************************************************************
function:	Sets the start position and size of the display area
parameter:
		Xstart 	:   X direction Start coordinates
		Ystart  :   Y direction Start coordinates
		Xend    :   X direction end coordinates
Yend    :   Y direction end coordinates
 ********************************************************************************/
static void LCD_1in3_SetWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend) {
	//set the X coordinates
	LCD_SendCommand(0x2A);
	LCD_SendData_8Bit((Xstart >> 8) & 0xFF);
	LCD_SendData_8Bit(Xstart & 0xFF);
	LCD_SendData_8Bit(((Xend  - 1) >> 8) & 0xFF);
	LCD_SendData_8Bit((Xend  - 1) & 0xFF);

	//set the Y coordinates
	LCD_SendCommand(0x2B);
	LCD_SendData_8Bit((Ystart >> 8) & 0xFF);
	LCD_SendData_8Bit(Ystart & 0xFF);
	LCD_SendData_8Bit(((Yend  - 1) >> 8) & 0xFF);
	LCD_SendData_8Bit((Yend  - 1) & 0xFF);

	LCD_SendCommand(0X2C);
}

/******************************************************************************
function :	Clear screen
parameter:
 ******************************************************************************/
static void LCD_1in3_Clear(UWORD Color) {
	UWORD j;
	UWORD Image[LCD_WIDTH*LCD_HEIGHT];

	Color = ((Color<<8)&0xff00)|(Color>>8);

	for (j = 0; j < LCD_HEIGHT*LCD_WIDTH; j++) {
		Image[j] = Color;
	}

	//LCD_1in3_SetWindows(0, 0, LCD_WIDTH, LCD_HEIGHT);
	LCD_DC_1;
	DEV_SPI_Write((uint8_t *)Image, LCD_WIDTH*LCD_HEIGHT*2);
}

static void lcd_init(void) {
	gpio_arch_init();

	gpio_arch_config(LCD_CS, 1);
	gpio_arch_config(LCD_RST, 1);
	gpio_arch_config(LCD_DC, 1);
	gpio_arch_config(LCD_BL, 1);

	spi_arch_init(4);
	spi_arch_select(1);

	LCD_1in3_Init(HORIZONTAL);
	LCD_1in3_SetWindows(0, 0, LCD_WIDTH, LCD_HEIGHT);
	LCD_1in3_Clear(0x0);
}

typedef struct {
	void* data;
	uint32_t size;
	int32_t shm_id;
} fb_dma_t;

static int _gpio_fd = -1;

static void  do_flush(const void* buf, uint32_t size) {
	if(size < LCD_WIDTH * LCD_HEIGHT* 4)
		return;

	critical_enter();

	LCD_DC_1;
	spi_arch_activate(1);

	uint32_t *src = (uint32_t*)buf;
	uint32_t sz = LCD_HEIGHT*LCD_WIDTH;
	UWORD i;

	for (i = 0; i < sz; i++) {
		register uint32_t s = src[i];
		register uint8_t b = (s >> 16) & 0xff;
		register uint8_t g = (s >> 8)  & 0xff;
		register uint8_t r = s & 0xff;
		UWORD color = ((r >> 3) <<11) | ((g >> 3) << 6) | (b >> 3);
		//color = ((color<<8)&0xff00)|(color>>8);
		uint8_t* p = (uint8_t*)&color;
		spi_arch_transfer(p[1]);
		spi_arch_transfer(p[0]);
	}

	spi_arch_activate(0);
	critical_quit();
}

static int lcd_flush(int fd, int from_pid, fsinfo_t* info, void* p) {
	(void)fd;
	(void)from_pid;
	(void)info;
	fb_dma_t* dma = (fb_dma_t*)p;

	do_flush(dma->data, dma->size);
	return 0;
}

static int lcd_dma(int fd, int from_pid, fsinfo_t* info, int* size, void* p) {
	(void)fd;
	(void)from_pid;
	(void)info;
	fb_dma_t* dma = (fb_dma_t*)p;
	*size = dma->size;
	return dma->shm_id;
}

static int lcd_write(int fd, int from_pid, fsinfo_t* info, 
		const void* buf, int size, int offset, void* p) {
	(void)fd;
	(void)from_pid;
	(void)info;
	(void)offset;
	(void)p;
	
	do_flush(buf, size);
	return size;
}

static int lcd_fcntl(int fd, int from_pid, fsinfo_t* info, 
		int cmd, proto_t* in, proto_t* out, void* p) {
	(void)fd;
	(void)from_pid;
	(void)info;
	(void)in;
	(void)p;

	if(cmd == CNTL_INFO) {
		proto_add_int(out, LCD_WIDTH);
		proto_add_int(out, LCD_HEIGHT);
	}
	return 0;
}

int main(int argc, char** argv) {
	lcd_init();

	const char* mnt_point = argc > 1 ? argv[1]: "/dev/lcd";

	uint32_t sz = LCD_HEIGHT*LCD_WIDTH*4;
	fb_dma_t dma;
	dma.shm_id = shm_alloc(sz, 1);
	if(dma.shm_id <= 0)
		return -1;
	dma.size = sz;
	dma.data = shm_map(dma.shm_id);
	if(dma.data == NULL)
		return -1;

	vdevice_t dev;
	memset(&dev, 0, sizeof(vdevice_t));
	strcpy(dev.name, "lcd");
	dev.write = lcd_write;
	dev.flush = lcd_flush;
	dev.dma   = lcd_dma;
	dev.fcntl = lcd_fcntl;

	device_run(&dev, mnt_point, FS_TYPE_DEV, &dma, 1);

	close(_gpio_fd);
	shm_unmap(dma.shm_id);
	return 0;
}
