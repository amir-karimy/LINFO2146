#include "contiki.h"
#include "net/nullnet/nullnet.h"
#include "sys/log.h"
#include "net/packetbuf.h"
#include "sys/etimer.h"
#include "net/netstack.h"
#include "lib/random.h"

#include <stdio.h>
#include <string.h>

#define LOG_MODULE "Sensor Node"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SEND_INTERVAL (CLOCK_SECOND * 2)   // Interval between sending sensor data
#define SENSOR_READING_PERIOD (CLOCK_SECOND * 10)   // Duration of the reading period

typedef struct {
  linkaddr_t coordinator_addr;
  uint16_t time_slot;
} time_slot_t;

static time_slot_t my_time_slot;

static struct etimer send_timer;
static struct etimer reading_period_timer;

static void send_data(void) {
  // Generate random sensor data
  uint16_t sensor_data = (uint16_t)random_rand();

  // Set the nullnet buffer and length
  nullnet_buf = (uint8_t *)&sensor_data;
  nullnet_len = sizeof(sensor_data);

  // Send the sensor data to the assigned coordinator node
  NETSTACK_NETWORK.output(&my_time_slot.coordinator_addr);

  LOG_INFO("Sensor data sent: %u\n", sensor_data);
}

PROCESS(sensor_node_process, "Sensor Node Process");
AUTOSTART_PROCESSES(&sensor_node_process);

PROCESS_THREAD(sensor_node_process, ev, data) {
  PROCESS_BEGIN();

  // Generate a random coordinator address for demonstration purposes
  my_time_slot.coordinator_addr.u8[0] = random_rand() % 256;
  my_time_slot.coordinator_addr.u8[1] = random_rand() % 256;

  while (1) {
    // Wait for the reading period timer to expire
    etimer_set(&reading_period_timer, SENSOR_READING_PERIOD);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reading_period_timer));

    // Set the time slot based on the current time
    my_time_slot.time_slot = clock_seconds() % 3;   // Adjust the value according to the number of coordinator nodes

    // Wait for the assigned time slot
    etimer_set(&send_timer, SEND_INTERVAL * my_time_slot.time_slot);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));

    // Send data during the assigned time slot
    send_data();
  }

  PROCESS_END();
}

