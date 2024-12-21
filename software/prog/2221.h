#ifndef _2221_H
#define _2221_H

#include <string>

extern "C" {
  #include <hidapi/hidapi.h>
  #include <unistd.h>
}

namespace mcp2221 {
  class mcp2221drv {
  public:
    mcp2221drv();
    ~mcp2221drv();

    void set_gpio_default_mask(unsigned char mask);
    void set_gpio_mask(unsigned char mask, unsigned char val);
    void gpio_out(char change_mask, char mask, char change_x, char x);
    int gpio_in();

  private:
    void do_write(bool read = true);

    void usb_error(std::string msg, int err = 0);

    unsigned char buf[65];
    hid_device *dev;
  };
}

#endif
