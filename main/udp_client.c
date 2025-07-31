/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
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

#ifdef CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN
#include "addr_from_stdin.h"
#endif

#if defined(CONFIG_EXAMPLE_IPV4)
#elif defined(CONFIG_EXAMPLE_IPV6)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
#else
#define HOST_IP_ADDR ""
#endif

#define PORT CONFIG_EXAMPLE_PORT

static const char *TAG = "UDP Connection";
static struct PacketMotionData motionData;
static struct PacketSessionData sessionData;
static struct PacketLapData lapData;
static struct PacketEventData eventData;
static struct PacketParticipantsData participantData;
static struct PacketCarSetupData carSetupData;
static struct PacketCarTelemetryData telemetryData;
static struct PacketCarStatusData statusData;
static struct PacketFinalClassificationData finalClassificationData;
static struct PacketLobbyInfoData lobbyInfoData;
static struct PacketCarDamageData carDammageData;
static struct PacketSessionHistoryData sessionHistoryData;
static struct PacketTyreSetsData tyreSetsData;
static struct PacketMotionExData motionDataExData;
static struct PacketTimeTrialData timeTrialData;

struct DataPointers
{
    static struct PacketMotionData *pMotionData;
    static struct PacketSessionData *pSessionData;
    static struct PacketLapData *pLapData;
    static struct PacketEventData *pEventData;
    static struct PacketParticipantsData *pParticipantData;
    static struct PacketCarSetupData *pCarSetupData;
    static struct PacketCarTelemetryData *pTelemetryData;
    static struct PacketCarStatusData *pStatusData;
    static struct PacketFinalClassificationData *pFinalClassificationData;
    static struct PacketLobbyInfoData *pLobbyInfoData;
    static struct PacketCarDamageData *pCarDammageData;
    static struct PacketSessionHistoryData *pSessionHistoryData;
    static struct PacketTyreSetsData *pTyreSetsData;
    static struct PacketMotionExData *pMotionDataExData;
    static struct PacketTimeTrialData *pTimeTrialData;
} DataPointers_init = {&motionData,$sessionData,&lapData,&eventData,&participantData,&carSetupData,&telemetryData,&statusData,&finalClassificationData,&lobbyInfoData,&carDammageData,&sessionHistoryData,&tyreSetsData,&motionDataExData,&timeTrialData};

typedef struct DataPointers DataPointers;
static DataPointers dataPointers = DataPointers_init;

static void display_motion(struct PacketCarTelemetryData *pvParameters){
    static float *throtle = &telemetryData.m_throttle
    static float *brake = &telemetryData.m_brake;
    while(1){
        if(*throttle > 0) gpio_set_level(GPIO_NUM_5,1); //accelerateur
        else gpio_set_level(GPIO_NUM_5,0); //accelerateur

        if(*brake > 0) gpio_set_level(GPIO_NUM_18,1); //frein
        else gpio_set_level(GPIO_NUM_18,0); //frein
        vTaskDelay(pdMS_TO_TICKS(200));
    }
};

static void udp_client_task(DataPointers *dataPointers)
{
    uint8_t rx_buffer[183];
    int addr_family = 0;
    int ip_protocol = 0;

    while (1) {

#if defined(CONFIG_EXAMPLE_IPV4)
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
#elif defined(CONFIG_EXAMPLE_IPV6)
        struct sockaddr_in6 dest_addr = { 0 };
        inet6_aton(HOST_IP_ADDR, &dest_addr.sin6_addr);
        dest_addr.sin6_family = AF_INET6;
        dest_addr.sin6_port = htons(PORT);
        dest_addr.sin6_scope_id = esp_netif_get_netif_impl_index(EXAMPLE_INTERFACE);
        addr_family = AF_INET6;
        ip_protocol = IPPROTO_IPV6;
#elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
        struct sockaddr_storage dest_addr = { 0 };
        ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_DGRAM, &ip_protocol, &addr_family, &dest_addr));
#endif

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        int broadcast_perm = 1;
        setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_perm, sizeof(broadcast_perm));

        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = 20;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

        struct sockaddr_in local_addr;
        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(PORT);
        local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
            close(sock);
            break;
        }
            

        while (1) {
            struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,(struct sockaddr *)&source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                switch (rx_buffer[6])
                {
                case 0:
                    dataPointers->pMotionData = (struct PacketMotionData) rx_buffer;
                break;
                case 1:
                    dataPointers->pSessionData = (struct PacketSessionData) rx_buffer;
                break;
                case 2:
                    dataPointers->pLapData = (struct PacketLapData) rx_buffer;
                break;
                case 3:
                    dataPointers->pEventData = (struct PacketEventData) rx_buffer;
                break;
                case 4:
                    dataPointers->pParticipantData = (struct PacketParticipantsData) rx_buffer;
                break;
                case 5:
                    dataPointers->pCarSetupData = (struct PacketCarSetupData) rx_buffer;
                break;
                case 6:
                    dataPointers->pElemetryData = (struct PacketCarTelemetryData) rx_buffer;
                    break;
                default:
                    break;
                }
            }
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    xTaskCreate(udp_client_task, "udp_client", 8192, &dataPointers, 5, NULL);
    xTaskCreate(display_motion,"display_motion",1024,&telemetryData,5,NULL);
}
