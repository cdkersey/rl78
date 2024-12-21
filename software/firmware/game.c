#define P0 (*(volatile unsigned char *)0xfff00)
#define PM0 (*(volatile unsigned char *)0xfff20)
#define P1 (*(volatile unsigned char *)0xfff01)
#define PM1 (*(volatile unsigned char *)0xfff21)
#define PM2 (*(volatile unsigned char *)0xfff22)
#define PM3 (*(volatile unsigned char *)0xfff23)
#define PM4 (*(volatile unsigned char *)0xfff24)
#define PM5 (*(volatile unsigned char *)0xfff25)
#define PM6 (*(volatile unsigned char *)0xfff26)
#define PM7 (*(volatile unsigned char *)0xfff27)

#define RPECTL (*(volatile unsigned char *)0xf00f5)

#define PER0 (*(volatile unsigned char *)0xf00f0)
#define ADM0 (*(volatile unsigned char *)0xf00f0) // TODO
#define ADM1 (*(volatile unsigned char *)0xf00f0) // TODO
#define ADCRH (*(volatile unsigned char *)0xfff1f)
#define ADCR (*(volatile unsigned char *)0xfff1e)
#define ADS (*(volatile unsigned char *)0xfff31)

#define NULL ((void*)0)

void entry();

__attribute__ ((section(".ivt"), used)) void (*ivt[])() = {
  entry, NULL, // 0: reset
  NULL, // 4
  NULL, // 6
  NULL, // 8
  NULL, // a
  NULL, // c
  NULL, // e
  NULL, // 10
  NULL, // 12
  NULL, // 14
  NULL, // 16
  NULL, // 18
  NULL, // 1a
  NULL, // 1c
  NULL, // 1e
  NULL, // 20
  NULL, // 22
  NULL, // 24
  NULL, // 26
  NULL, // 28
  NULL, // 2a
  NULL, // 2c
  NULL, // 2e
  NULL, // 30
  NULL, // 32
  NULL // 34
};

__attribute__ ((section(".opt"), used)) unsigned char option_bytes[] = {
  0x00, 0x91, 0xe8, 0x04
};

extern unsigned char imgdata[][1024];

void delay(unsigned long n) {
  static volatile unsigned long i;
  for (i = 0; i < (n << 12); ++i);
}

void delayu() {
  asm("nop;");
}

#define I2C_DATA 0x02
#define I2C_CLOCK 0x01


#define P1_NGATE 0x02
#define P1_SW 0x01

void power_init() {
  PM1 = P1_SW;
  P1 = P1_NGATE;
}

unsigned char sw() {
  return !(P1 & P1_SW);
}

void power_off() {
  P1 = 0;
  for(;;);
}

void i2c_init() {
  P0 = 0x00; // Open-drain!
  PM0 = I2C_DATA | I2C_CLOCK;
}

inline void i2c_data(unsigned char x) {
  if (!x)
    PM0 &= ~I2C_DATA;
  else
    PM0 |= I2C_DATA;
}

inline void i2c_clock(unsigned char x) {
  if (!x)
    PM0 &= ~I2C_CLOCK;
  else
    PM0 |= I2C_CLOCK;
}

void i2c_start() {
  i2c_clock(1);
  i2c_data(1);
  //delayu();
  i2c_data(0);
  //delayu();
  i2c_clock(0);
  //delayu();
}

void i2c_stop() {
  i2c_data(0);
  i2c_clock(0);
  //delayu();
  i2c_clock(1);
  //delayu();
  i2c_data(1);
  //delayu();
}

unsigned char send_i2c_bit(unsigned char b) {
  i2c_clock(0);
  delayu();
  i2c_data(b);
  delayu();
  i2c_clock(1);
  delayu();
  unsigned char r = P0 & I2C_DATA;
  i2c_clock(0);

  return r ? 1 : 0;
}

unsigned char send_i2c_byte(unsigned char b) {
  for (int i = 7; i >= 0; --i)
    send_i2c_bit(b & (1 << i));
  unsigned char ack = !send_i2c_bit(1);
  return ack;
}

void init_display() {
  for (unsigned i = 0; i < 10; ++i) {
    delay(10);
    i2c_start();
    send_i2c_byte(0x78);
    send_i2c_byte(0x80);
    send_i2c_byte(0xae);
    i2c_stop();
  }

  delay(10);

  i2c_start();
  send_i2c_byte(0x78);
  send_i2c_byte(0x80); send_i2c_byte(0xa8);
  send_i2c_byte(0x80); send_i2c_byte(0x3f);
  send_i2c_byte(0x80); send_i2c_byte(0xd3);
  send_i2c_byte(0x80); send_i2c_byte(0x00);
  send_i2c_byte(0x80); send_i2c_byte(0x40);
  send_i2c_byte(0x80); send_i2c_byte(0xa1);
  send_i2c_byte(0x80); send_i2c_byte(0xc8);
  send_i2c_byte(0x80); send_i2c_byte(0xda);
  send_i2c_byte(0x80); send_i2c_byte(0x12);
  send_i2c_byte(0x80); send_i2c_byte(0x81);
  send_i2c_byte(0x80); send_i2c_byte(0x7f);
  send_i2c_byte(0x80); send_i2c_byte(0xa4);
  send_i2c_byte(0x80); send_i2c_byte(0xa6);
  send_i2c_byte(0x80); send_i2c_byte(0xd5);
  send_i2c_byte(0x80); send_i2c_byte(0x80);
  send_i2c_byte(0x80); send_i2c_byte(0x8d);
  send_i2c_byte(0x80); send_i2c_byte(0x14);
  send_i2c_byte(0x80); send_i2c_byte(0x20);
  send_i2c_byte(0x80); send_i2c_byte(0x00);
  send_i2c_byte(0x80); send_i2c_byte(0x21);
  send_i2c_byte(0x80); send_i2c_byte(0x00);
  send_i2c_byte(0x80); send_i2c_byte(0x7f);
  send_i2c_byte(0x80); send_i2c_byte(0x22);
  send_i2c_byte(0x80); send_i2c_byte(0x00);
  send_i2c_byte(0x80); send_i2c_byte(0x07);
  send_i2c_byte(0x80); send_i2c_byte(0xaf);
  i2c_stop();
}

unsigned char framebuf[1024];

void clear_framebuf() {
  for (unsigned i = 0; i < 1024; ++i)
    framebuf[i] = 0;
}

void pset(unsigned char x, unsigned char y) {
  unsigned char b = 1<<(y & 0x07);
  unsigned idx = ((y&0xf8)<<4) | x;
  framebuf[idx] |= b;
}

void update_display() {
  i2c_start();
  send_i2c_byte(0x78);
  send_i2c_byte(0x40);
  for (unsigned i = 0; i < 1024; ++i) {
    if (!send_i2c_byte(framebuf[i])) break;
  }
  i2c_stop();
}

unsigned char get_img(unsigned char sel, unsigned i) {
  return *((unsigned char __far *)((unsigned)imgdata[sel] & 0xffff) + i);
}

#include "spritedata.h"

unsigned char get_sprite_data(unsigned char i) {
  return *((unsigned char __far *)((unsigned)sprite_data & 0xffff) + i);
}

unsigned char get_sprite_mask(unsigned char i) {
  return *((unsigned char __far *)((unsigned)sprite_mask & 0xffff) + i);
}

void draw_sprite(unsigned char pos) {
  int i;

  for (i = 0; i < 16; i++) {
    int x, x2;
    x = pos + i;
    x2 = x + 128;

    if (x >= 0 && x < 128) {
      framebuf[x] &= ~get_sprite_mask(i);
      framebuf[x2] &= ~get_sprite_mask(16 + i);
      framebuf[x] |= get_sprite_data(i);
      framebuf[x2] |= get_sprite_data(16 + i);
    }
  }
}

typedef enum {
  TITLE, MANS, PLAY, DEAD, WIN
} gamestate_t;

typedef enum {
  FRAME_LVL = 0,
  FRAME_MANS = 25,
  FRAME_DEAD = 28,
  FRAME_TITLE = 29,
  FRAME_WIN = 30
} frame_t;

int gamestate = TITLE;
unsigned ctr = 0, x;
unsigned char v, dir, sw_hist, framesel, flash, xpos, tally, mans = 3, lvl = 0,
  timeout;

void game() {
  // Init globals
  gamestate = TITLE;
  ctr = 0;
  lvl = 0;
  mans = 3;
  flash = 0;

  // Hold switch on
  power_init();
  i2c_init();
  init_display();
  PM2 = PM3 = PM4 = PM5 = PM6 = PM7 = 0;

  for (;;) {
    if (flash) flash--;

    if (gamestate == TITLE) {
      framesel = FRAME_TITLE;
      if (ctr++ == 80) {
	ctr = 0;
	mans = 3;
	lvl = 0;
	gamestate = MANS;
      }
    } else if (gamestate == MANS) {
      framesel = FRAME_MANS + 3 - mans;
      if (ctr++ == 80) {
	ctr = 0;
	gamestate = PLAY;
	x = 0;
	dir = 1;
	v = (lvl >> 2) + 6;
	sw_hist = 0;
	tally = 2;
	timeout = 20;
      }
    } else if (gamestate == PLAY) {
      xpos = (x >= 112 ? 224 - x : x);

      framesel = FRAME_LVL + lvl;

      x += v;
      if (x >= 224) {
	x -= 224;
	if (--timeout == 0)
	  gamestate = DEAD;
      }

      if (!sw_hist && sw()) {
	timeout = 20;
        if (xpos >= 40 && xpos <= 72) {
	  flash = 5;
	  tally++;
	  if (tally == 10) {
	    if (lvl < 24) {
              lvl++;
              gamestate = MANS;
	    } else {
	      gamestate = WIN;
	    }
	  }
        } else {
	  tally--;
	  if (tally == 0) {
            if (mans > 1) {
              mans--;
              gamestate = MANS;
            } else {
              gamestate = DEAD;
              ctr = 0;
            }
	  }
        }
        x = 0;
      }
    } else {
      framesel = (gamestate == DEAD ? FRAME_DEAD : FRAME_WIN);
      if (ctr++ == 80) power_off();
    }

    sw_hist = sw();
    for (unsigned i = 0; i < 1024; ++i) {
      framebuf[i] = get_img(framesel, i);
      if (flash && i >= 256)
        framebuf[i] = ~framebuf[i];
    }
    if (gamestate == PLAY) draw_sprite(xpos);
    update_display();
  }
}

void entry() {
  asm("movw sp, #0xfee0");

  // Initialize global data
  extern char _bdata, _edata, _binit;

  char __far *d = (char __far *)((unsigned)(&_binit) & 0xffff);
  char *q;
  for (q = &_bdata; q != &_edata; d++, q++)
    *q = *d;

  // Disable reset-on-parity-error. draw_sprite() does a read of uninitialized
  // memory that causes a reset sometimes.
  RPECTL = 0x80;

  // Enable interrupts
  // asm("ei");

  game();
}
