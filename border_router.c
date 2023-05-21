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

#define LOG_MODULE "Border Router"
#define LOG_LEVEL LOG_LEVEL_INFO

#define TIME_WINDOW_DURATION (CLOCK_SECOND * 60)   // Duration of the time window
#define MIN_TIME_SLOT_DURATION (CLOCK_SECOND * 5)   // Minimum duration of a time slot

typedef struct {
  linkaddr_t coordinator_addr;
  uint16_t time_slot;
} time_slot_t;

static time_slot_t time_slots[3];   

static struct etimer time_window_timer;
static struct ctimer backoff_timer;

static void input_callback(const void *data, uint16_t len,
                           const linkaddr_t *src, const linkaddr_t *dest) {
 
}

static void send_packet(void *ptr) {
  uint16_t counter = *(uint16_t *)ptr;
  nullnet_buf = (uint8_t *)&counter;
  nullnet_len = sizeof(counter);

 
  NETSTACK_NETWORK.output(NULL);

  LOG_INFO("Sent packet %u to server\n", counter);
}

PROCESS(border_router_process, "Border Router Process");
AUTOSTART_PROCESSES(&border_router_process);

PROCESS_THREAD(border_router_process, ev, data) {
  static struct etimer periodic_timer;
  static struct etimer time_slot_timer;
  static uint16_t current_time_slot;

  PROCESS_BEGIN();

  nullnet_set_input_callback(input_callback);

  // Generate a random time slot for demonstration purposes
  current_time_slot = random_rand() % 3;   

  // Set the time slot and coordinator address
  time_slots[current_time_slot].time_slot = current_time_slot;
  time_slots[current_time_slot].coordinator_addr = linkaddr_node_addr;

  etimer_set(&periodic_timer, CLOCK_SECOND * 8);
  etimer_set(&time_window_timer, TIME_WINDOW_DURATION);

  while (1) {
    PROCESS_WAIT_EVENT();

    if (ev == PROCESS_EVENT_TIMER) {
      if (etimer_expired(&periodic_timer)) {
        // Start the backoff timer
        ctimer_set(&backoff_timer, MIN_TIME_SLOT_DURATION, send_packet, &current_time_slot);

        etimer_reset(&periodic_timer);
      } else if (etimer_expired(&time_window_timer)) {
        // Update the time slot and coordinator address at the end of the time window
        current_time_slot = random_rand() % 3;   // Adjust the value according to the number of coordinator nodes
        time_slots[current_time_slot].time_slot = current_time_slot;
        time_slots[current_time_slot].coordinator_addr = linkaddr_node_addr;

        etimer_set(&time_window_timer, TIME_WINDOW_DURATION);
      } else if (etimer_expired(&time_slot_timer)) {
        // Start the time slot timer for the current time slot
        etimer_set(&time_slot_timer, MIN_TIME_SLOT_DURATION * (current_time_slot + 1));
      }
    }
  }

  PROCESS_END();
}

