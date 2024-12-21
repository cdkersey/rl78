#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>

#include "2221.h"

const unsigned EXPECT_ID(0x04d800dd);

using namespace std;
using namespace mcp2221;

void mcp2221drv::do_write(bool read) {
  if (hid_write(dev, buf, 65) == -1)
    usb_error("hid_write() failed.");
  
  if (read) {
    if (hid_read_timeout(dev, buf, 65, 10) == -1)
      usb_error("hid_read_timeout() failed.");

    #if 0
    cout << endl;
    for (unsigned i = 0; i < 64; ++i) {
      cout << ' ' << hex << setw(2) << setfill('0') << (unsigned)buf[i];
      if ((i & 15) == 15) cout << endl;
    }
    cout << endl;
    #endif
  }
}

void mcp2221drv::set_gpio_default_mask(unsigned char mask) {
  for (unsigned i = 0; i < 65; ++i)
    buf[i] = 0;

  buf[0] = 0x00;
  buf[1] = 0xb1;
  buf[2] = 0x01;

  for (unsigned i = 0; i < 4; ++i)
    buf[3 + i] = ((mask >> i) & 1) ? 0x00 : 0x08;

  do_write();

  for (unsigned i = 0; i < 65; ++i)
    buf[i] = 0;

  buf[0] = 0x00;
  buf[1] = 0xb1;
  buf[2] = 0x00;
  buf[7] = 0xd8;
  buf[8] = 0x04;
  buf[9] = 0xdd;
  buf[10] = 0x00;

  do_write();
}

void mcp2221drv::set_gpio_mask(unsigned char mask, unsigned char val) {
  for (unsigned i = 0; i < 65; ++i)
    buf[i] = 0;

  buf[0] = 0x00;
  buf[1] = 0x60;

  buf[8] = 0xff;
  for (unsigned i = 0; i < 4; ++i)
    buf[9 + i] = ((mask >> i) & 1) ? (((val >> i) & 1) ? 0x10 : 0x00) : 0x08;
 
  do_write();

  //buf[0] = 0x00;
  //buf[1] = 0x61;
  //do_write();
}

void mcp2221drv::gpio_out(char change_mask, char mask, char change_x, char x) {
  for (unsigned i = 0; i < 65; ++i)
    buf[i] = 0;

  buf[1] = 0x50;

  for (unsigned i = 0; i < 4; ++i) {
    buf[3 + 4*i] = (change_x >> i) & 1;
    buf[4 + 4*i] = (x >> i) & 1;
    buf[5 + 4*i] = (change_mask >> i) & 1;
    buf[6 + 4*i] = (mask >> i) & 1 ? 0 : 1;
  }

  do_write();
}

int mcp2221drv::gpio_in() {
  for (unsigned i = 0; i < 65; ++i)
    buf[i] = 0;

  int r = 0;

  buf[0] = 0x00;
  buf[1] = 0x51;

  do_write();

  for (unsigned i = 0; i < 4; ++i) {
    int x = buf[2 + 2*i];
    if (x == 0 || x == 1) {
      r |= (x << i);
    } else {
      cout << "GPIO " << i << " NOT INPUT" << endl;
    }
  }

  return r;
}

mcp2221drv::mcp2221drv() {
  dev = hid_open(EXPECT_ID >> 16, EXPECT_ID & 0xffff, NULL);
  if (!dev) usb_error("hid_open() failed.");


}

mcp2221drv::~mcp2221drv() {
}

void mcp2221drv::usb_error(string msg, int err) {
  cerr << "USB error: " << msg << endl;
  if (dev) cerr << "  " << hid_error(dev) << endl;
  abort();
}
