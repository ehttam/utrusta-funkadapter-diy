// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <cstdint>

#include "esphome/components/spi_device/spi_device.h"
#include "esphome/core/log.h"

namespace utrusta {

class Radio {
 public:
  using Device = esphome::spi_device::SPIDeviceComponent;

  bool setup(Device *device) {
    device_ = device;
    ready_ = false;

    // CC2500 manual reset sequence (TI datasheet, section 19.1.2).
    device_->enable();
    delayMicroseconds(10);
    device_->disable();
    delayMicroseconds(60);
    if (!select_ready())
      return false;
    device_->write_byte(0x30);  // SRES
    delayMicroseconds(150);
    const bool reset_ready = wait_for_so_low();
    device_->disable();
    if (!reset_ready)
      return false;

    uint8_t part = 0;
    uint8_t version = 0;
    if (!read_register(0xF0, part) || !read_register(0xF1, version) || part != 0x80 ||
        version == 0x00 || version == 0xFF)
      return false;

    // RF profile reconstructed in NDBCK/Ansluta-Remote-Controller (cc2500_VAL.h).
    // One-byte variable length, CRC, four-byte preamble, MSK, nominal 2436.2 MHz.
    constexpr Setting profile[] = {
        {0x00, 0x29}, {0x02, 0x06},  // IOCFG2, IOCFG0
        {0x04, 0xD3}, {0x05, 0x91},  // Sync word
        {0x06, 0xFF}, {0x07, 0x04}, {0x08, 0x05}, {0x09, 0x01},  // Packet settings
        {0x0A, 0x10}, {0x0B, 0x09}, {0x0C, 0x00},  // Channel and oscillator
        {0x0D, 0x5D}, {0x0E, 0x93}, {0x0F, 0xB1},  // RF carrier base
        {0x10, 0x2D}, {0x11, 0x3B}, {0x12, 0x73}, {0x13, 0xA2}, {0x14, 0xF8},
        {0x15, 0x01}, {0x16, 0x07}, {0x17, 0x30}, {0x18, 0x18},
        {0x19, 0x1D}, {0x1A, 0x1C}, {0x1B, 0xC7}, {0x1C, 0x00}, {0x1D, 0xB2},
        {0x1E, 0x87}, {0x1F, 0x6B}, {0x20, 0xF8},
        {0x21, 0xB6}, {0x22, 0x10}, {0x23, 0xEA}, {0x24, 0x0A},
        {0x25, 0x00}, {0x26, 0x11}, {0x27, 0x41}, {0x28, 0x00}, {0x29, 0x59},
        {0x2C, 0x88}, {0x2D, 0x31}, {0x2E, 0x0B},
        {0x3E, 0xFF},  // PATABLE[0], nominal +1 dBm
    };
    for (const Setting &setting : profile) {
      if (!write_register(setting.address, setting.value))
        return false;
    }

    uint8_t frequency_mid = 0;
    uint8_t modem_config = 0;
    ready_ = read_register(0x8E, frequency_mid) && frequency_mid == 0x93 &&
             read_register(0x92, modem_config) && modem_config == 0x73;
    if (ready_)
      ESP_LOGI("utrusta_radio", "CC2500 bereit: PARTNUM=0x%02X VERSION=0x%02X", part, version);
    return ready_;
  }

  bool send_frame(uint16_t sender, uint8_t command) {
    if (!ready_ || !strobe(0x36) || !wait_for_idle() || !strobe(0x3B) || !select_ready())
      return false;
    // Burst write TX FIFO. 0x06 is the payload length; 0xFF is the trailing
    // FIFO byte used by the reference sender and lies outside that length.
    const uint8_t fifo[] = {0x7F, 0x06, 0x55, 0x01,
                            static_cast<uint8_t>(sender >> 8), static_cast<uint8_t>(sender),
                            command, 0xAA, 0xFF};
    for (const uint8_t byte : fifo)
      device_->write_byte(byte);
    device_->disable();
    return strobe(0x35);  // STX
  }

  bool tx_complete() {
    uint8_t tx_bytes = 0;
    return ready_ && wait_for_idle() && read_register(0xFA, tx_bytes) && (tx_bytes & 0x80) == 0;
  }

 private:
  struct Setting {
    uint8_t address;
    uint8_t value;
  };

  bool wait_for_so_low() {
    const uint32_t start = micros();
    while (digitalRead(12)) {  // NodeMCU D6, CC2500 SO
      if (micros() - start >= 5000)
        return false;
      delayMicroseconds(10);
    }
    return true;
  }

  bool select_ready() {
    device_->enable();
    if (wait_for_so_low())
      return true;
    device_->disable();
    return false;
  }

  bool strobe(uint8_t command) {
    if (!select_ready())
      return false;
    device_->write_byte(command);
    device_->disable();
    return true;
  }

  bool write_register(uint8_t address, uint8_t value) {
    if (!select_ready())
      return false;
    device_->write_byte(address);
    device_->write_byte(value);
    device_->disable();
    return true;
  }

  bool read_register(uint8_t read_header, uint8_t &value) {
    if (!select_ready())
      return false;
    device_->write_byte(read_header);
    value = device_->read_byte();
    device_->disable();
    return true;
  }

  bool wait_for_idle() {
    const uint32_t start = micros();
    do {
      uint8_t state = 0;
      if (!read_register(0xF5, state))
        return false;
      if ((state & 0x1F) == 0x01)
        return true;
      delayMicroseconds(50);
    } while (micros() - start < 3000);
    return false;
  }

  Device *device_{nullptr};
  bool ready_{false};
};

inline Radio radio;

}  // namespace utrusta
