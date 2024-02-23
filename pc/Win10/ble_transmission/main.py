import asyncio
import datetime
from bleak import BleakClient, BleakScanner


OTA_DATA_UUID = '23408888-1F40-4CD8-9B89-CA8D45F8A5B0'
OTA_CONTROL_UUID = '7AD671AA-21C0-46A4-B722-270E3AE3D830'

WIFI_DATA_UUID = 'CC08D0BD-9044-4CD2-85F5-A488BD4412C8'

SVR_CHR_OTA_CONTROL_NOP = bytearray.fromhex("00")
SVR_CHR_OTA_CONTROL_REQUEST = bytearray.fromhex("01")
SVR_CHR_OTA_CONTROL_REQUEST_ACK = bytearray.fromhex("02")
SVR_CHR_OTA_CONTROL_REQUEST_NAK = bytearray.fromhex("03")
SVR_CHR_OTA_CONTROL_DONE = bytearray.fromhex("04")
SVR_CHR_OTA_CONTROL_DONE_ACK = bytearray.fromhex("05")
SVR_CHR_OTA_CONTROL_DONE_NAK = bytearray.fromhex("06")


async def _search_for_esp32():
    print("Searching for ESP32...")
    esp32 = None

    devices = await BleakScanner.discover()
    for device in devices:
        if device.name == "esp32":
            esp32 = device

    if esp32 is not None:
        print("ESP32 found!")
    else:
        print("ESP32 has not been found.")
        assert esp32 is not None

    return esp32


async def send_ota(file_path):
    t0 = datetime.datetime.now()
    queue = asyncio.Queue()
    ssid = 28

    esp32 = await _search_for_esp32()
    async with BleakClient(esp32) as client:

        # compute the packet size
        #packet_size = (client.mtu_size - 3)
        #paired = await client.pair(protection_level=2)
        #print(f"Paired: {paired}")

        value = await client.read_gatt_char(WIFI_DATA_UUID)
        print(f"Read value: ", value)

        await client.write_gatt_char(WIFI_DATA_UUID, b'Info=100', False)
        # write the packet size to OTA Data
        #await client.write_gatt_char(
        #    WIFI_DATA_UUID,
        #    ssid,
        #    response=False
        #)
        await asyncio.sleep(1.0)


if __name__ == '__main__':
    asyncio.run(send_ota("esp32_ble_ota.bin"))
