#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <fstream>

#include "2221.h"

extern "C" {
  #include <libserialport.h>
  #include <unistd.h>
}

using namespace std;
using namespace mcp2221;

const unsigned FLASH_SIZE = 0x30000;

int check(enum sp_return x) {
  if (x < 0) {
    cout << "Serial port error: " << sp_last_error_message() << endl;
    abort();
  }

  return x;
}

void print_hex(char *buf, int n) {
  for (unsigned i = 0; i < n; ++i) {
    cout << ' ' << setw(2) << setfill('0') << hex
         << (int)(unsigned char)buf[i];
    if ((i & 7) == 7) cout << endl;
  }
  cout << endl;
}

const unsigned SERIAL_TIMEOUT = 1000;

struct board {
  board(const char *serial_port);
  ~board();

  void push_soh();
  void push_stx();
  void push_etx();
  void push_etb();
  void push_byte(int b);
  void push_checksum();

  void send();
  void send_packet();
  void send_mode();

  bool blank_check(int start, int end, int tar);

  void write_or_verify_flash(bool write, const char *data);
  void write_flash(const char *data);
  void verify_flash(const char *data);
  void erase_block(unsigned addr);
  void erase_flash();

  void rec(int n);
  int rec_packet(int &type);

  mcp2221drv m;
  struct sp_port *sp;
  char buf[512];
  int pos, sum;
};

void board::erase_block(unsigned addr) {
  push_soh();
  push_byte(0x04);
  push_byte(0x22);

  // Starting address: 0x00000
  push_byte(addr & 0xff);
  push_byte((addr >> 8) & 0xff);
  push_byte((addr >> 16) & 0xff);

  push_checksum();
  push_etx();

  send_packet();

  int l, type;
  l = rec_packet(type);
  if (type != 2 || l != 3 || buf[0] != 6) {
    cout << "Erase failed." << endl;
    abort();
  }
}

void board::erase_flash() {
  for (unsigned i = 0; i < FLASH_SIZE; i += 0x400)
    erase_block(i);
}

void board::write_or_verify_flash(bool write, const char *data) {
  push_soh();
  push_byte(0x07);
  push_byte(write ? 0x40 : 0x13);

  // Starting address: 0x00000
  push_byte(0x00);
  push_byte(0x00);
  push_byte(0x00);

  // End address: 0x2ffff
  push_byte(0xff);
  push_byte(0xff);
  push_byte(0x02);

  push_checksum();
  push_etx();

  send_packet();

  int type;
  int l = rec_packet(type);
  if (type != 2 || buf[0] != 6) {
    cout << "Did not receive ack in response to write flash command." << endl;
    abort();
  }

  for (unsigned blk = 0; blk < FLASH_SIZE; blk += 0x100) {
    bool last = (blk == 0x2ff00);
    push_stx();
    push_byte(0x00);
    for (unsigned i = 0; i < 256; ++i)
      push_byte(data[blk + i]);
    push_checksum();
    if (last) push_etx();
    else push_etb();

    send_packet();

    l = rec_packet(type);
    if (type != 2 || buf[0] != 6 || buf[1] != 6) {
      cout << "Did not receive ack in response to write/verify command."
	   << " Block starting 0x" << hex << blk << endl;
      abort();
    }
  }
}

void board::write_flash(const char *data) {
  write_or_verify_flash(true, data);
}

void board::verify_flash(const char *data) {
  write_or_verify_flash(false, data);
}

void board::push_stx() {
  push_byte(0x02);
  sum = 0;
}

void board::push_etx() {
  push_byte(0x03);
}

void board::push_etb() {
  push_byte(0x17);
}

bool board::blank_check(int start, int end, int tar) {
  push_soh();
  push_byte(0x08);
  push_byte(0x32);
  push_byte(start & 0xff);
  push_byte((start >> 8) & 0xff);
  push_byte((start >> 16) & 0xff);
  push_byte(end & 0xff);
  push_byte((end >> 8) & 0xff);
  push_byte((end >> 16) & 0xff);
  push_byte(tar);
  push_checksum();
  push_etx();

  send_packet();

  int type;
  int l = rec_packet(type);

  return (l == 3 && buf[0] == 6);
}

void board::push_soh() {
  push_byte(0x01);
  sum = 0;
}

void board::push_byte(int b) {
  if (pos == 512) {
    cout << "Transmit buffer overflowed." << endl;
    abort();
  }
  buf[pos++] = b;
  sum += b;
}

void board::push_checksum() {
  push_byte(-sum & 0xff);
}

void board::send_mode() {
  push_byte(0x3a);
  send();
  usleep(100);
}

void board::send() {
  if (pos == 0) {
    cout << "Attempt to send empty buffer." << endl;
    abort();
  }
  check(sp_blocking_write(sp, buf, pos, SERIAL_TIMEOUT));
  pos = 0;
}

void board::send_packet() {
  int type = buf[0];
  vector<char> data;
  for (unsigned i = 1; i < pos; ++i)
    data.push_back(buf[i]);

  send();
  int rec_type;
  int l = rec_packet(rec_type);
  if (l != data.size() - 1) {
    cout << "Tried to transmit " << dec << data.size()
	 << " bytes but only saw " << l << " bytes." << endl;
    abort();
  }
  bool data_mismatch = false;
  for (unsigned i = 0; i < l; ++i)
    if (data.size() <= i || buf[i] != data[i + 1]) data_mismatch = true;
  if (data_mismatch) {
    cout << "Sent bytes do not match observed data." << endl;
    abort();
  }
}

void board::rec(int n) {
  int n_rec = check(sp_blocking_read(sp, buf, n, SERIAL_TIMEOUT));
  if (n_rec != n) {
    cout << "Timeout on receive." << endl;
    abort();
  }
}

int board::rec_packet(int &type) {
  rec(2);

  type = buf[0];
  int len = buf[1];
  if (len == 0) len = 256;
  rec(len + 2);

  cout << "Received type " << dec << type << ", " << len + 2 << " bytes."
       << endl;

  int sum = len;
  for (unsigned i = 0; i < len + 1; ++i)
    sum += buf[i];

  if (sum & 0xff) {
    cout << "Checksum fail on received packet:" << endl;
    print_hex(buf, len + 2);
    cout << "Residual checksum value 0x" << hex << (sum & 0xff) << endl;
    abort();
  }
  
  print_hex(buf, len + 2);

  return len + 2;
}

board::board(const char *serial_port): pos(0), sum(0) {
  // Assert reset
  m.set_gpio_mask(0x03, 0x00);
  usleep(200000);

  check(sp_get_port_by_name(serial_port, &sp));
  cout << "Serial port description: " << sp_get_port_description(sp) << endl;

  check(sp_open(sp, SP_MODE_READ_WRITE));
  check(sp_set_baudrate(sp, 115200));
  check(sp_set_bits(sp, 8));
  check(sp_set_parity(sp, SP_PARITY_NONE));
  check(sp_set_stopbits(sp, 2));
  check(sp_set_flowcontrol(sp, SP_FLOWCONTROL_NONE));

  // De-assert reset
  m.gpio_out(0x00, 0x00, 0x01, 0x01);
  
  usleep(2000);
  
  // De-assert tool0
  m.gpio_out(0x02, 0x01, 0x00, 0x00);

  send_mode();

  do { rec(1); } while (buf[0] != 0x3a);

  push_soh();
  push_byte(0x03);
  push_byte(0x9a);
  push_byte(0x00);
  push_byte(0x32);
  push_checksum();
  push_etx();

  send_packet();

  int type;
  int l = rec_packet(type);
  if (l != 5 || type != 2) {
    cout << "Invalid response format on setting baud rate." << endl;
    abort();
  }

  if (buf[0] != 0x06) {
    cout << "Failed to receive ack on setting baud rate." << endl;
    abort();
  }

  cout << "CPU speed: " << dec << (int)buf[1] << " MHz" << endl
       << (buf[2] ? "wide-voltage mode" : "full-speed mode") << endl;
}

board::~board() {
  // Re-assert and de-assert reset
  m.gpio_out(0x00, 0x00, 0x01, 0x00);
  usleep(10000);
  m.gpio_out(0x00, 0x00, 0x01, 0x01);
  sleep(1);

  check(sp_close(sp));
  sp_free_port(sp);
}

void load_bin(vector<char> &v, const char *filename) {
  ifstream in(filename);

  v.clear();
  while (!!in)
    v.push_back(in.get());

  while (v.size() < 0x30000)
    v.push_back(0x00);
}

//#define ERASE
#define WRITE
//#define VERIFY

int main(int argc, char **argv) {
  if (argc != 3) {
    cout << "Usage:" << endl << "  " << argv[0] << " <serial port> <e/w/v>" << endl;
    return 1;
  }

  board b(argv[1]);

  if (argv[2][0] == 'e') {
    if (!b.blank_check(0x00000, 0x2ffff, 0)) {
      cout << "Blank check failed!" << endl;
      b.erase_flash();
    }
  } else {
    vector<char> prog_data;
    load_bin(prog_data, "demo/hello.bin");

    if (argv[2][0] == 'w') {
      b.write_flash(&prog_data[0]);
    } else if (argv[2][0] == 'v') {
      b.verify_flash(&prog_data[0]);
    }
  }

  return 0;
}
