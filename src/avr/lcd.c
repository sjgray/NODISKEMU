/*-
 * Copyright (c) 2015 Nils Eilers. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "lcd.h"
#include "config.h"

#define LCD_DELAY_US_DATA   46
#define LCD_DELAY_MS_CLEAR  1000

#define LCD_DDRAM 128

uint8_t lcd_x, lcd_y;


static inline void lcd_set_data_mode(void) {
  LCD_PORT_RS |= _BV(LCD_PIN_RS);
}


static inline void lcd_set_command_mode(void) {
  LCD_PORT_RS &= ~_BV(LCD_PIN_RS);
}


static void lcd_pulse_e(void) {
  LCD_PORT_E |= _BV(LCD_PIN_E);                // E high
  _delay_us(20);
  LCD_PORT_E &= ~_BV(LCD_PIN_E);               // E low
}


static void lcd_write(uint8_t v) {
  LCD_PORT_DATA &= 0xF0;
  LCD_PORT_DATA |= ((v >> 4) & 0x0F);          // high nibble
  lcd_pulse_e();
  _delay_us(LCD_DELAY_US_DATA);
  LCD_PORT_DATA &= 0xF0;
  LCD_PORT_DATA |= ( v       & 0x0F);          // low  nibble
  lcd_pulse_e();
  _delay_us(LCD_DELAY_US_DATA);
}


void lcd_send_command(uint8_t cmd) {
  lcd_set_command_mode();
  lcd_write(cmd);
}


void lcd_locate(uint8_t x, uint8_t y) {
  if (x >= LCD_COLS) {
    x = 0;
    y++;
  }

  if (y >= LCD_LINES) y = 0;

  lcd_set_command_mode();
  uint8_t StartAddress = 0;
  switch (y)
  {
    case 0: StartAddress = LCD_ADDR_LINE1; break;
    case 1: StartAddress = LCD_ADDR_LINE2; break;
    case 2: StartAddress = LCD_ADDR_LINE3; break;
    case 3: StartAddress = LCD_ADDR_LINE4; break;
    default: StartAddress = 0;
  }
  lcd_write(LCD_DDRAM + StartAddress + x);
  lcd_x = x;
  lcd_y = y;
}


void lcd_clear(void) {
  lcd_send_command(0x01);
  lcd_x = lcd_y = 0;
  _delay_ms(LCD_DELAY_MS_CLEAR);
}


void lcd_home(void) {
  lcd_send_command(0x02);
  lcd_x = lcd_y = 0;
  _delay_us(LCD_DELAY_MS_CLEAR);
}


void lcd_init(void) {
  // LCD ports as output
  LCD_DDR_E  |= _BV(LCD_PIN_E);
  LCD_DDR_RS |= _BV(LCD_PIN_RS);
  LCD_DDR_DATA |= 0x0F;

  // Start with all outputs low
  LCD_PORT_E &= ~_BV(LCD_PIN_E);
  LCD_PORT_RS &= ~_BV(LCD_PIN_RS);
  LCD_PORT_DATA &= 0xF0;


  _delay_ms(15);               // allow LCD to init after power-on

  LCD_PORT_DATA &= 0xF0;
  LCD_PORT_DATA |= 0x03;       // send init value 0x30 three times
  lcd_pulse_e(); _delay_ms(5);
  lcd_pulse_e(); _delay_ms(1);
  lcd_pulse_e(); _delay_ms(1);

  LCD_PORT_DATA &= 0xF2;       // select bus width: 4-bit
  lcd_pulse_e(); _delay_ms(5);

  lcd_write(0x28);             // 4 bit, 2 line, 5x7 dots
  lcd_write(0x0C);             // Display on, cursor off
  lcd_write(0x06);             // Automatic increment, no shift
  lcd_clear();
}


void lcd_putc(char c) {
  if (c == '\n')
  {
    lcd_x = 0;
    lcd_y++;
  }

  lcd_locate(lcd_x, lcd_y);
  if (c >= ' ')
  {
    lcd_set_data_mode();
    lcd_write(c);
    lcd_x++;
  }
}


void lcd_puts(const char *s) {
  while (*s)
    lcd_putc(*s++);
}


void lcd_puts_P(const char *s) {
  char c;

  while ((c = pgm_read_byte(s++)))
    lcd_putc(c);
}