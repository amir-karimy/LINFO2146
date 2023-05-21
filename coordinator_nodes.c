#include "contiki.h"
#include "net/nullnet/nullnet.h"
#include "sys/log.h"
#include "net/packetbuf.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "net/netstack.h"
#include "lib/random.h"

#include <stdio.h>
#include <string.h>

#define LOG_MODULE "Coordinator Node"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SEND_INTERVAL (CLOCK_SECOND * 8)   // Interval between sending packets

typedef struct {
  linkaddr_t coordinator_addr;
  uint16_t time_slot;
} time_slot_t;

static time_slot_t time_slots[3];   
static void input_callback(const void *data, uint16_t len,
                           const linkaddr_t *src, const linkaddr_t *dest) {
  // Implement your logic here.
}

static void send_packet(void *ptr) {
  uint16_t time_slot = *(uint16_t *)ptr;
  nullnet_buf = (uint8_t *)&time_slot;
  nullnet_len = sizeof(time_slot);

  // Send the packet to the border router
  NETSTACK_NETWORK.output(&time_slots[time_slot].coordinator_addr);

  LOG_INFO("Sent packet to border router\n");
}

PROCESS(coordinator_node_process, "Coordinator Node Process");
AUTOSTART_PROCESSES(&coordinator_node_process);

PROCESS_THREAD(coordinator_node_process, ev, data) {
  static struct etimer periodic_timer;
  static struct ctimer backoff_timer;

  PROCESS_BEGIN();

  nullnet_set_input_callback(input_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL);

  while (1) {
    PROCESS_WAIT_EVENT();

    if (ev == PROCESS_EVENT_TIMER && etimer_expired(&periodic_timer)) {
      uint16_t time_slot = random_rand() % 3;   // Adjust the value according to the number of coordinator nodes
      time_slots[time_slot].time_slot = time_slot;
      time_slots[time_slot].coordinator_addr = linkaddr_node_addr;

      ctimer_set(&backoff_timer, SEND_INTERVAL, send_packet, &time_slot);

      etimer_reset(&periodic_timer);
    }
  }

  PROCESS_END();
}

