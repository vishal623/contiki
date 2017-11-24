/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      CoAP module for reliable transport
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#ifndef COAP_TRANSACTIONS_H_
#define COAP_TRANSACTIONS_H_

#include "er-coap.h"

/*
 * Modulo mask (thus +1) for a random number to get the tick number for the random
 * retransmission time between COAP_RESPONSE_TIMEOUT and COAP_RESPONSE_TIMEOUT*COAP_RESPONSE_RANDOM_FACTOR.
 */
#define COAP_RESPONSE_TIMEOUT_TICKS         (CLOCK_SECOND * COAP_RESPONSE_TIMEOUT)
#define COAP_RESPONSE_TIMEOUT_BACKOFF_MASK  (long)((CLOCK_SECOND * COAP_RESPONSE_TIMEOUT * ((float)COAP_RESPONSE_RANDOM_FACTOR - 1.0)) + 0.5) + 1



/* container for transactions with message buffer and retransmission info */
typedef struct coap_transaction {
  struct coap_transaction *next;        /* for LIST */

  uint16_t mid;
  struct etimer retrans_timer;
  uint8_t retrans_counter;

  uip_ipaddr_t addr;
  uint16_t port;

  restful_response_handler callback;
  void *callback_data;

  uint16_t packet_len;
  uint8_t packet[COAP_MAX_PACKET_SIZE + 1];     /* +1 for the terminating '\0' which will not be sent

                                                 * Use snprintf(buf, len+1, "", ...) to completely fill payload */
  #ifdef COCOA
  //August:
    clock_time_t timestamp;
    clock_time_t previous_rto;
    clock_time_t start_rto;
  #endif
} coap_transaction_t;

void coap_register_as_transaction_handler();

coap_transaction_t *coap_new_transaction(uint16_t mid, uip_ipaddr_t * addr,
                                         uint16_t port);
void coap_send_transaction(coap_transaction_t * t);
void coap_clear_transaction(coap_transaction_t * t);
coap_transaction_t *coap_get_transaction_by_mid(uint16_t mid);

void coap_check_transactions();

#ifdef COCOA
//AUGUST:
/* August: container for maintaining RTT estimations*/
typedef struct coap_rtt_estimations {
  struct coap_rtt_estimations *next; /* for LIST */

  uip_ipaddr_t addr;

  clock_time_t rtt[2];

 //delay gradient variables start
  clock_time_t rttmin;
  clock_time_t prev_rttmin;
  clock_time_t rttmax;
  clock_time_t prev_rttmax;
  unsigned long delta_min;
  unsigned long delta_max;
  uint8_t rtt_count;
 //delay gradient variables end

  clock_time_t rttvar[2];
  clock_time_t rto[3];
  clock_time_t lastupdated[3];

  uint8_t rttsmissed;
} coap_rtt_estimations_t;

#define MAXRTO_VALUE 60 * CLOCK_SECOND
#define COAP_MAX_RTT_ESTIMATIONS 2
#define NSTART 1
#define COAP_INITIAL_RTO 2 * CLOCK_SECOND

clock_time_t coap_check_rto_state(clock_time_t rto, clock_time_t oldrto,uip_ipaddr_t *addr);
uint8_t countTransactionsForAddress(uip_ipaddr_t *addr,  list_t transactions_list);
void coap_update_rtt_estimation(uip_ipaddr_t* transactionAddr, clock_time_t rtt, uint8_t retransmissions);
void coap_delete_rtt_by_freshness();
clock_time_t coap_check_rtt_estimation(uip_ipaddr_t *transactionAddr, list_t transcations_list);
coap_rtt_estimations_t *coap_new_rtt_estimation(clock_time_t rtt, uip_ipaddr_t *addr, uint8_t retransmissions);
#endif


#endif /* COAP_TRANSACTIONS_H_ */
