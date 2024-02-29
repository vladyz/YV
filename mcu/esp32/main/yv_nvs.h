#ifndef __YV_NVS_H
#define __YV_NVS_H

#define USERDATA_STORAGE_NAMESPACE "storage"

#define NVS_KEY_WIFI_SSID "WiFi_SSID"
#define NVS_KEY_WIFI_PASS "WiFi_PASS"
#define USER_DATA_LEN_BYTES (16)
//const char *WiFi_SSID_NVS_key = "WiFi_SSID";
//const char *WiFi_PASS_NVS_key = "WiFi_PASS";

void yv_nvs_init(void);
esp_err_t yv_nvs_write(uint8_t *pData, uint8_t DataLen, char *NVSKey);
esp_err_t yv_nvs_read(uint8_t *pData, uint8_t DataLen, char *NVSKey);
int yv_nvs_get_size(char *Key);

#endif /*__YV_NVS_H*/
