/*	Mirf Example

	This example code is in the Public Domain (or CC0 licensed, at your option.)

	Unless required by applicable law or agreed to in writing, this
	software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "mirf.h"

#if CONFIG_ADVANCED
void AdvancedSettings(NRF24_t * dev)
{
#if CONFIG_RF_RATIO_2M
	ESP_LOGW(pcTaskGetName(0), "Set RF Data Ratio to 2MBps");
	Nrf24_SetSpeedDataRates(dev, 1);
#endif // CONFIG_RF_RATIO_2M

#if CONFIG_RF_RATIO_1M
	ESP_LOGW(pcTaskGetName(0), "Set RF Data Ratio to 1MBps");
	Nrf24_SetSpeedDataRates(dev, 0);
#endif // CONFIG_RF_RATIO_2M

#if CONFIG_RF_RATIO_250K
	ESP_LOGW(pcTaskGetName(0), "Set RF Data Ratio to 250KBps");
	Nrf24_SetSpeedDataRates(dev, 2);
#endif // CONFIG_RF_RATIO_2M

	ESP_LOGW(pcTaskGetName(0), "CONFIG_RETRANSMIT_DELAY=%d", CONFIG_RETRANSMIT_DELAY);
	Nrf24_setRetransmitDelay(dev, CONFIG_RETRANSMIT_DELAY);
}
#endif // CONFIG_ADVANCED

#if CONFIG_RECEIVER
void receiver(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	NRF24_t dev;
	Nrf24_init(&dev);
	uint8_t payload = 32;
	uint8_t channel = CONFIG_RADIO_CHANNEL;
	Nrf24_config(&dev, channel, payload);

	//Set own address using 5 characters
	esp_err_t ret = Nrf24_setRADDR(&dev, (uint8_t *)"1RECV");
	if (ret != ESP_OK) {
		ESP_LOGE(pcTaskGetName(0), "nrf24l01 not installed");
		while(1) { vTaskDelay(1); }
	}

	//Add own address using 1 characters
	Nrf24_addRADDR(&dev, 2, '2'); // 2RECV
	Nrf24_addRADDR(&dev, 3, '3'); // 3RECV
	Nrf24_addRADDR(&dev, 4, '4'); // 4RECV
	Nrf24_addRADDR(&dev, 5, '5'); // 5RECV

#if CONFIG_ADVANCED
	AdvancedSettings(&dev);
#endif // CONFIG_ADVANCED

	//Print settings
	Nrf24_printDetails(&dev);
	ESP_LOGI(pcTaskGetName(0), "Listening...");

	uint8_t buf[32];

	// Clear RX FiFo
	while(1) {
		if (Nrf24_dataReady(&dev) == false) break;
		Nrf24_getData(&dev, buf);
	}

	while(1) {
		//When the program is received, the received data is output from the serial port
		if (Nrf24_dataReady(&dev)) {
			uint8_t pipe = Nrf24_getDataPipe(&dev);
			Nrf24_getData(&dev, buf);
			ESP_LOGI(pcTaskGetName(0), "Got data pipe(%d):%s", pipe, buf);
		}
		vTaskDelay(1);
	}
}
#endif // CONFIG_RECEIVER

#if CONFIG_SENDER
void sender(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	NRF24_t dev;
	Nrf24_init(&dev);
	uint8_t payload = 32;
	uint8_t channel = CONFIG_RADIO_CHANNEL;
	Nrf24_config(&dev, channel, payload);

	//Set the receiver address using 5 characters
	int sender_id;
#if CONFIG_TADDR1
	esp_err_t ret = Nrf24_setTADDR(&dev, (uint8_t *)"1RECV");
	sender_id = 1;
#elif CONFIG_TADDR2
	esp_err_t ret = Nrf24_setTADDR(&dev, (uint8_t *)"2RECV");
	sender_id = 2;
#elif CONFIG_TADDR3
	esp_err_t ret = Nrf24_setTADDR(&dev, (uint8_t *)"3RECV");
	sender_id = 3;
#elif CONFIG_TADDR4
	esp_err_t ret = Nrf24_setTADDR(&dev, (uint8_t *)"4RECV");
	sender_id = 4;
#elif CONFIG_TADDR5
	esp_err_t ret = Nrf24_setTADDR(&dev, (uint8_t *)"5RECV");
	sender_id = 5;
#endif
	if (ret != ESP_OK) {
		ESP_LOGE(pcTaskGetName(0), "nrf24l01 not installed");
		while(1) { vTaskDelay(1); }
	}

#if CONFIG_ADVANCED
	AdvancedSettings(&dev);
#endif // CONFIG_ADVANCED

	//Print settings
	Nrf24_printDetails(&dev);

	uint8_t buf[32];
	while(1) {
		TickType_t nowTick = xTaskGetTickCount();
		sprintf((char *)buf, "Hello World %"PRIu32" from %d", nowTick, sender_id);
		Nrf24_send(&dev, buf);
		vTaskDelay(1);
		ESP_LOGI(pcTaskGetName(0), "Wait for sending.....");
		if (Nrf24_isSend(&dev, 1000)) {
			ESP_LOGI(pcTaskGetName(0),"Send success:%s", buf);
		} else {
			ESP_LOGW(pcTaskGetName(0),"Send fail:");
		}
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}
#endif // CONFIG_SENDER


void app_main(void)
{
#if CONFIG_RECEIVER
	xTaskCreate(&receiver, "RECEIVER", 1024*3, NULL, 2, NULL);
#endif

#if CONFIG_SENDER
	xTaskCreate(&sender, "SENDER", 1024*3, NULL, 2, NULL);
#endif
}
