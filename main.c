#include "main.h"
#include "keys.h"

#include <stdio.h>

#include "ble/sm.h"
#include "bluetooth.h"
#include "btstack.h"
#include "btstack_defines.h"
#include "btstack_run_loop.h"
#include "gap.h"
#include "hardware/gpio.h"
#include "hci.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#include "ble/gatt-service/battery_service_server.h"
#include "ble/gatt-service/device_information_service_server.h"
#include "ble/gatt-service/hids_device.h"

const int LED_PIN = CYW43_WL_GPIO_LED_PIN;
static int battery = 100;

// clang-format off
const uint8_t adv_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
    // Name
    0x0d, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'H', 'I', 'D', ' ', 'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd',
    // 16-bit Service UUIDs
    0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xff, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8,
    // Appearance HID - Keyboard (Category 15, Sub-Category 1)
    0x03, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC1, 0x03,
};
// clang-format on
const uint8_t adv_data_len = sizeof(adv_data);

static void packet_handler(uint8_t packet_type, uint16_t channel,
                           uint8_t *packet, uint16_t size) {
  UNUSED(channel);
  UNUSED(size);
  // Early return if we don't care about packet
  if (packet_type != HCI_EVENT_PACKET)
    return;
}

static void hardware_init() {
  stdio_init_all();
  cyw43_arch_init();
  l2cap_init();
  sm_init();
  sm_set_io_capabilities(IO_CAPABILITY_DISPLAY_ONLY);
  sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION |
                                     SM_AUTHREQ_BONDING);

  att_server_init(profile_data, NULL, NULL);
  battery_service_server_init(battery);
  device_information_service_server_init();
  hids_device_init(0, hid_desc_boot, sizeof(hid_desc_boot));

  uint16_t adv_int_min = 0x0030;
  uint16_t adv_int_max = 0x0030;
  uint8_t adv_type = 0;
  bd_addr_t null_addr;
  memset(null_addr, 0, 6);
  gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0,
                                null_addr, 0x07, 0x00);
  gap_advertisements_set_data(adv_data_len, (uint8_t *)adv_data);
  gap_advertisements_enable(1);

  btstack_packet_callback_registration_t bt_packet_cb;
  bt_packet_cb.callback = &packet_handler;
  hci_add_event_handler(&bt_packet_cb);

  btstack_packet_callback_registration_t sm_packet_cb;
  sm_packet_cb.callback = &packet_handler;
  sm_add_event_handler(&sm_packet_cb);

  hids_device_register_packet_handler(packet_handler);
}

int main() {
  hardware_init();

  hci_power_control(HCI_POWER_ON);

  multicore_launch_core1(btstack_run_loop_execute);

  while (true) {
    printf("blink ");
    cyw43_arch_gpio_put(LED_PIN, 0);
    sleep_ms(250);
    cyw43_arch_gpio_put(LED_PIN, 1);
    sleep_ms(250);
  }
  // if we get here there is trouble
  return 0;
}
