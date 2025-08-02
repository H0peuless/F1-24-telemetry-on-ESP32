#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "F1.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "driver/gpio.h"
#include "esp_mac.h"
#define STRIP_SIZE 9
#define NULL ((void *)0)
#define LED_PIN 34
#define TH0 8
#define TH1 16
#define TL0 9
#define TL1 17
#define TRESET 1000