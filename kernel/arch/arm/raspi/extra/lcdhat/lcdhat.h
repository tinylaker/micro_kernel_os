#ifndef LCDHAT_H
#define LCDHAT_H

#define LCD_HEIGHT 240
#define LCD_WIDTH 240

void  lcd_flush(const void* buf, uint32_t size);
void  lcd_init(void);

#endif
