/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * $Id: uip6-bridge-tap.c,v 1.1 2009/05/08 12:49:36 joxe Exp $
 *
 */

/**
 * \file
 *         A brief description of what this file is
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#include "contiki.h"
#include "net/uip.h"
#include "dev/slip.h"
#include "dev/leds.h"
#include "sicslow_ethernet.h"
#include <stdio.h>
#include <string.h>

#define UIP_IP_BUF ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

PROCESS(uip6_bridge, "IPv6/6lowpan TAP bridge");
PROCESS(tcpip_process, "tcpip dummy");
AUTOSTART_PROCESSES(&uip6_bridge);

/*---------------------------------------------------------------------------*/
static uint8_t (* outputfunc)(uip_lladdr_t *a);
uint8_t
tcpip_output(uip_lladdr_t *a)
{
  if(outputfunc != NULL) {
    outputfunc(a);
    leds_invert(LEDS_GREEN);
  }
  return 0;
}
void
tcpip_ipv6_output(void)
{
}
void
tcpip_set_outputfunc(uint8_t (*f)(uip_lladdr_t *))
{
  outputfunc = f;
}
PROCESS_THREAD(tcpip_process, ev, data)
{
  PROCESS_BEGIN();
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* Packet from SICSLoWPAN */
void
tcpip_input(void)
{
  if(uip_len > 0) {
    mac_LowpanToEthernet();
    if(uip_len > 0) {
/*       slip_write(UIP_IP_BUF, uip_len); */
      slip_write(uip_buf, uip_len);
      leds_invert(LEDS_RED);
      uip_len = 0;
    }
  }
}
/*---------------------------------------------------------------------------*/
/* Packet from SLIP */
static void
slip_tcpip_input(void)
{
  /* TODO Should fix this in slip configuration */
  memmove(uip_buf, &uip_buf[UIP_LLH_LEN], uip_len);
  mac_ethernetToLowpan(uip_buf);
}
/*---------------------------------------------------------------------------*/
static void
slip_activity(void)
{
  leds_invert(LEDS_BLUE);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(uip6_bridge, ev, data)
{
  PROCESS_BEGIN();

  printf("Setting up SLIP\n");

  mac_ethernetSetup();

  slip_arch_init(BAUD2UBR(115200));
  slip_set_input_callback(slip_activity);
  slip_set_tcpip_input_callback(slip_tcpip_input);
  process_start(&slip_process, NULL);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
