/*
 * Copyright 2013-2019 Software Radio Systems Limited
 *
 * This file is part of srsLTE.
 *
 * srsLTE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsLTE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#ifndef SRSLTE_PDCP_BASE_TEST_H
#define SRSLTE_PDCP_BASE_TEST_H

#include "srslte/common/buffer_pool.h"
#include "srslte/common/log_filter.h"
#include "srslte/common/security.h"
#include "srslte/interfaces/ue_interfaces.h"
#include <iostream>

/*
 * Functions and macros for comparisions
 */
#define TESTASSERT(cond)                                                                                               \
  {                                                                                                                    \
    if (!(cond)) {                                                                                                     \
      std::cout << "[" << __FUNCTION__ << "][Line " << __LINE__ << "]: FAIL at " << (#cond) << std::endl;              \
      return -1;                                                                                                       \
    }                                                                                                                  \
  }

int compare_two_packets(const srslte::unique_byte_buffer_t& msg1, const srslte::unique_byte_buffer_t& msg2)
{
  TESTASSERT(msg1->N_bytes == msg2->N_bytes);
  for (uint32_t i = 0; i < msg1->N_bytes; ++i) {
    TESTASSERT(msg1->msg[i] == msg2->msg[i]);
  }
  return 0;
}

/*
 * Definition of helpful structs for testing
 */
struct pdcp_security_cfg {
  uint8_t*                            k_int_rrc;
  uint8_t*                            k_enc_rrc;
  uint8_t*                            k_int_up;
  uint8_t*                            k_enc_up;
  srslte::INTEGRITY_ALGORITHM_ID_ENUM int_algo;
  srslte::CIPHERING_ALGORITHM_ID_ENUM enc_algo;
};

/*
 * Dummy classes
 */
class rlc_dummy : public srsue::rlc_interface_pdcp
{
public:
  rlc_dummy(srslte::log* log_) : log(log_) {}

  void get_last_sdu(const srslte::unique_byte_buffer_t& pdu)
  {
    memcpy(pdu->msg, last_pdcp_pdu->msg, last_pdcp_pdu->N_bytes);
    pdu->N_bytes = last_pdcp_pdu->N_bytes;
    return;
  }
  void write_sdu(uint32_t lcid, srslte::unique_byte_buffer_t sdu, bool blocking = true)
  {
    log->info_hex(sdu->msg, sdu->N_bytes, "RLC SDU");
    last_pdcp_pdu.swap(sdu);
    rx_count++;
  }
  void discard_sdu(uint32_t lcid, uint32_t discard_sn)
  {
    log->info("Notifing RLC to discard SDU (SN=%u)\n", discard_sn);
    discard_count++;
    log->info("Discard_count=%" PRIu64 "\n", discard_count);
  }

  uint64_t rx_count      = 0;
  uint64_t discard_count = 0;

private:
  srslte::log*                 log;
  srslte::unique_byte_buffer_t last_pdcp_pdu;

  bool rb_is_um(uint32_t lcid) { return false; }
};

class rrc_dummy : public srsue::rrc_interface_pdcp
{
public:
  rrc_dummy(srslte::log* log_) {}

  void write_pdu(uint32_t lcid, srslte::unique_byte_buffer_t pdu) {}
  void write_pdu_bcch_bch(srslte::unique_byte_buffer_t pdu) {}
  void write_pdu_bcch_dlsch(srslte::unique_byte_buffer_t pdu) {}
  void write_pdu_pcch(srslte::unique_byte_buffer_t pdu) {}
  void write_pdu_mch(uint32_t lcid, srslte::unique_byte_buffer_t pdu) {}

  std::string get_rb_name(uint32_t lcid) { return "None"; }
};

class gw_dummy : public srsue::gw_interface_pdcp
{
public:
  gw_dummy(srslte::log* log_) : log(log_) {}

  void     write_pdu_mch(uint32_t lcid, srslte::unique_byte_buffer_t pdu) {}
  uint32_t rx_count = 0;

  void get_last_pdu(const srslte::unique_byte_buffer_t& pdu)
  {
    memcpy(pdu->msg, last_pdu->msg, last_pdu->N_bytes);
    pdu->N_bytes = last_pdu->N_bytes;
    return;
  }
  void write_pdu(uint32_t lcid, srslte::unique_byte_buffer_t pdu)
  {
    log->info_hex(pdu->msg, pdu->N_bytes, "GW PDU");
    rx_count++;
    last_pdu.swap(pdu);
  }

private:
  srslte::log*                 log;
  srslte::unique_byte_buffer_t last_pdu;
};

// Helper to print packets
void print_packet_array(const srslte::unique_byte_buffer_t& msg)
{
  printf("uint8_t msg[] = {\n");
  for (uint64_t i = 0; i < msg->N_bytes; ++i) {
    printf("0x%02x, ", msg->msg[i]);
  }
  printf("\n};\n");
}
#endif // SRSLTE_PDCP_BASE_TEST_H