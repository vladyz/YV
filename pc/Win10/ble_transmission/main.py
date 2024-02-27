import asyncio
import math
import sys
import os
import datetime
# from time import sleep
from bleak import BleakClient, BleakScanner
# from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import (
    QApplication,
    QLabel,
    QMainWindow,
    QPushButton,
    QVBoxLayout,
    QWidget,
    QLineEdit,
    QListWidget,
    QFileDialog,
    QProgressBar
)

from PyQt6.QtCore import QObject, QThread, pyqtSignal

WIFI_SSID_DATA_UUID = 'CC08D0BD-9044-4CD2-85F5-A488BD4412C8'
WIFI_PASS_DATA_UUID = 'e10334f1-22f8-4b97-bcc6-ebd39a6f8e49'
FIRMWARE_VERSION_UUID = '3ae1a84d-ddd3-4ef1-922a-70921cc5a914'

OTA_DATA_UUID = '23408888-1F40-4CD8-9B89-CA8D45F8A5B0'
OTA_CONTROL_UUID = '7AD671AA-21C0-46A4-B722-270E3AE3D830'

SVR_CHR_OTA_CONTROL_NOP = bytearray.fromhex("00")
SVR_CHR_OTA_CONTROL_REQUEST = bytearray.fromhex("01")
SVR_CHR_OTA_CONTROL_REQUEST_ACK = bytearray.fromhex("02")
SVR_CHR_OTA_CONTROL_REQUEST_NAK = bytearray.fromhex("03")
SVR_CHR_OTA_CONTROL_DONE = bytearray.fromhex("04")
SVR_CHR_OTA_CONTROL_DONE_ACK = bytearray.fromhex("05")
SVR_CHR_OTA_CONTROL_DONE_NAK = bytearray.fromhex("06")

async def _search_for_esp32(wrkr):
    print("Searching for ESP32...")
    wrkr.progress.emit("Searching for ESP32...")
    esp32 = None

    devices = await BleakScanner.discover()
    for device in devices:
        if device.name == "esp32":
            esp32 = device

    if esp32 is not None:
        print("ESP32 found!")
        wrkr.progress.emit("ESP32 found!")
    else:
        print("ESP32 has not been found.")
        wrkr.progress.emit("ESP32 has not been found.")
        assert esp32 is not None
    return esp32

async def esp32_get_firmware_ver(wrkr):
    esp32 = await _search_for_esp32(wrkr)
    async with BleakClient(esp32) as client:
        value = await client.read_gatt_char(FIRMWARE_VERSION_UUID)
        s = "Firmware version is " + value.decode('UTF-8')
        print(s)
        wrkr.progress.emit(s)

# class for getting version of firmware
class WorkerGetVer(QObject):
    finished = pyqtSignal()
    progress = pyqtSignal(str)
    def run(self):
        asyncio.run(esp32_get_firmware_ver(self))
        self.finished.emit()

async def esp32_get_wifi_data(wrkr):
    esp32 = await _search_for_esp32(wrkr)
    async with BleakClient(esp32) as client:
        wifi_ssid = await client.read_gatt_char(WIFI_SSID_DATA_UUID)
        wifi_pass = await client.read_gatt_char(WIFI_PASS_DATA_UUID)
        s = "WiFi SSID is " + wifi_ssid.decode('UTF-8')
        print(s)
        wrkr.progress.emit(s)
        s = "WiFi Pass is " + wifi_pass.decode('UTF-8')
        print(s)
        wrkr.progress.emit(s)

# class for getting wifi data
class WorkerGetWiFiData(QObject):
    finished = pyqtSignal()
    progress = pyqtSignal(str)
    def run(self):
        asyncio.run(esp32_get_wifi_data(self))
        self.finished.emit()

async def esp32_set_wifi_data(wrkr):
    esp32 = await _search_for_esp32(wrkr)
    async with BleakClient(esp32) as client:
        await client.write_gatt_char(WIFI_SSID_DATA_UUID, wrkr.wifi_ssid.encode('utf-8'), False)
        await asyncio.sleep(1.0)
        s = "WiFi SSID is changed to  " + wrkr.wifi_ssid  # wifi_ssid.decode('UTF-8')
        print(s)
        wrkr.progress.emit(s)

        await client.write_gatt_char(WIFI_PASS_DATA_UUID, wrkr.wifi_pass.encode('utf-8'), False)
        await asyncio.sleep(1.0)
        s = "WiFi PASS is changed to  " + wrkr.wifi_pass  # wifi_ssid.decode('UTF-8')
        print(s)
        wrkr.progress.emit(s)


#class for setting wifi data
class WorkerSetWiFiData(QObject):
    finished = pyqtSignal()
    progress = pyqtSignal(int)
    status = pyqtSignal(str)

    wifi_ssid =''
    wifi_pass =''
    def run(self):
        asyncio.run(esp32_set_wifi_data(self))
        self.finished.emit()



async def esp32_update_firmware(wrkr):
    t0 = datetime.datetime.now()
    queue = asyncio.Queue()
    firmware = []

    esp32 = await _search_for_esp32(wrkr)
    async with BleakClient(esp32) as client:
        async def _ota_notification_handler(sender: int, data: bytearray):
            if data == SVR_CHR_OTA_CONTROL_REQUEST_ACK:
                print("ESP32: OTA request acknowledged.")
                wrkr.progress.emit("ESP32: OTA request acknowledged.")
                await queue.put("ack")
            elif data == SVR_CHR_OTA_CONTROL_REQUEST_NAK:
                print("ESP32: OTA request NOT acknowledged.")
                wrkr.progress.emit("ESP32: OTA request NOT acknowledged.")
                await queue.put("nak")
                await client.stop_notify(OTA_CONTROL_UUID)
            elif data == SVR_CHR_OTA_CONTROL_DONE_ACK:
                print("ESP32: OTA done acknowledged.")
                wrkr.progress.emit("ESP32: OTA done acknowledged.")
                await queue.put("ack")
                await client.stop_notify(OTA_CONTROL_UUID)
            elif data == SVR_CHR_OTA_CONTROL_DONE_NAK:
                print("ESP32: OTA done NOT acknowledged.")
                await queue.put("nak")
                await client.stop_notify(OTA_CONTROL_UUID)
            else:
                print(f"Notification received: sender: {sender}, data: {data}")
                wrkr.progress.emit("Notification received")

        # subscribe to OTA control
        await client.start_notify(
            OTA_CONTROL_UUID,
            _ota_notification_handler
        )

        # compute the packet size
        packet_size = (client.mtu_size - 3)

        # write the packet size to OTA Data
        print(f"Sending packet size: {packet_size}.")
        # wrkr.progress.emit("Sending packet size:
        await client.write_gatt_char(
            OTA_DATA_UUID,
            packet_size.to_bytes(2, 'little'),
            response=True
        )

        # split the firmware into packets
        with open(wrkr.binfile, "rb") as file:
            while chunk := file.read(packet_size):
                firmware.append(chunk)

        # write the request OP code to OTA Control
        print("Sending OTA request.")
        wrkr.progress.emit("Sending OTA request.")
        await client.write_gatt_char(
            OTA_CONTROL_UUID,
            SVR_CHR_OTA_CONTROL_REQUEST,
            response=True
        )

        # wait for the response
        await asyncio.sleep(0.5)
        if await queue.get() == "ack":

            # sequentially write all packets to OTA data
            for i, pkg in enumerate(firmware):
                print(f"Sending packet {i + 1}/{len(firmware)}.")
                wrkr.status.emit(math.trunc((i+1)*100/len(firmware)))
                await client.write_gatt_char(
                    OTA_DATA_UUID,
                    pkg,
                    response=True
                )

            # write done OP code to OTA Control
            print("Sending OTA done.")
            wrkr.progress.emit("Sending OTA done.")
            await client.write_gatt_char(
                OTA_CONTROL_UUID,
                SVR_CHR_OTA_CONTROL_DONE
            )

            # wait for the response
            await asyncio.sleep(0.5)
            if await queue.get() == "ack":
                dt = datetime.datetime.now() - t0
                print(f"OTA successful! Total time: {dt}")
                wrkr.progress.emit("OTA successful!")
            else:
                print("OTA failed.")
                wrkr.progress.emit("OTA failed.")

        else:
            print("ESP32 did not acknowledge the OTA request.")
            wrkr.progress.emit("ESP32 did not acknowledge the OTA request.")



# Step 1: Create a worker class
class WorkerUpdateFirmware(QObject):
    finished = pyqtSignal()
    progress = pyqtSignal(str)
    status = pyqtSignal(int)

    binfile = ''
    def run(self):
        asyncio.run(esp32_update_firmware(self))
        self.finished.emit()

class Window(QMainWindow):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.clicksCount = 0
        self.setupUi()

    def setupUi(self):
        self.setWindowTitle("YV Config")
        #self.resize(300, 150)
        self.centralWidget = QWidget()
        self.setCentralWidget(self.centralWidget)
        layout = QVBoxLayout()

        # Create and connect widgets

        self.SSIDLabel = QLabel("Enter Wifi SSID:")
        layout.addWidget(self.SSIDLabel)
        self.SSIDEdit = QLineEdit()
        layout.addWidget(self.SSIDEdit)


        self.PassLabel = QLabel("Enter Wifi Password:")
        layout.addWidget(self.PassLabel)
        self.PassEdit = QLineEdit()
        layout.addWidget(self.PassEdit)

        self.WriteWiFiDataButton = QPushButton("Write WiFi Data")
        layout.addWidget(self.WriteWiFiDataButton)
        self.WriteWiFiDataButton.clicked.connect(self.WriteWiFiDataButtonOnClick)

        self.ReadWiFiDataButton = QPushButton("Read WiFi Data")
        layout.addWidget(self.ReadWiFiDataButton)
        self.ReadWiFiDataButton.clicked.connect(self.ReadWiFiDataButtonOnClick)

        self.GetVerButton = QPushButton("Get Version")
        layout.addWidget(self.GetVerButton)
        self.GetVerButton.clicked.connect(self.GetVerButtonOnClick)


        self.UpdateFirmwareButton = QPushButton("Update firmware", self)
        layout.addWidget(self.UpdateFirmwareButton)
        self.UpdateFirmwareButton.clicked.connect(self.UpdateFirmwareButtonOnClick)

        self.UpdateFirmwareProgressbar = QProgressBar(self)
        self.UpdateFirmwareProgressbar.setValue(0)
        layout.addWidget(self.UpdateFirmwareProgressbar)

        # Set the layout
        layout.addStretch()


        self.ConsoleList = QListWidget()
        layout.addWidget(self.ConsoleList)

        self.centralWidget.setLayout(layout)

        wdth = 300
        hght = 400
        self.setFixedWidth(wdth)
        self.setFixedHeight(hght)

    def reportProgressUpdateFirmware(self, n):
        self.ConsoleList.addItem(n)


    def reportStatusUpdateFirmware(self, n):
        self.UpdateFirmwareProgressbar.setValue(n)

    def reportProgressGetVer(self, n):
        self.ConsoleList.addItem(n)
    def reportProgressGetWiFiData(self, n):
        self.ConsoleList.addItem(n)
    def reportProgressSetWiFiData(self, n):
        self.ConsoleList.addItem(n)
    def EnableAllButtons(self):
        self.UpdateFirmwareButton.setEnabled(True)
        self.ReadWiFiDataButton.setEnabled(True)
        self.WriteWiFiDataButton.setEnabled(True)
        self.GetVerButton.setEnabled(True)
    def DisableAllButtons(self):
        self.UpdateFirmwareButton.setEnabled(False)
        self.ReadWiFiDataButton.setEnabled(False)
        self.WriteWiFiDataButton.setEnabled(False)
        self.GetVerButton.setEnabled(False)

    def UpdateFirmwareButtonOnClick(self):

        fname = QFileDialog.getOpenFileName(self, 'Open file', '', '*.bin')[0]
        if os.path.isfile(fname):
            print("Firmware file is ", fname)
            s = "Firmware file is " + fname
            self.ConsoleList.addItem(s)

            # Step 2: Create a QThread object
            self.threadUpdateFirmware = QThread()
            # Step 3: Create a worker object
            self.workerUpdateFirmware = WorkerUpdateFirmware()
            # Step 4: Move worker to the thread
            self.workerUpdateFirmware.moveToThread(self.threadUpdateFirmware)
            # Step 5: Connect signals and slots
            self.threadUpdateFirmware.started.connect(self.workerUpdateFirmware.run)
            self.workerUpdateFirmware.finished.connect(self.threadUpdateFirmware.quit)
            self.workerUpdateFirmware.finished.connect(self.workerUpdateFirmware.deleteLater)
            self.threadUpdateFirmware.finished.connect(self.threadUpdateFirmware.deleteLater)
            self.workerUpdateFirmware.progress.connect(self.reportProgressUpdateFirmware)
            self.workerUpdateFirmware.status.connect(self.reportStatusUpdateFirmware)

            self.workerUpdateFirmware.binfile = fname
            # Step 6: Start the thread
            self.threadUpdateFirmware.start()

            # Final resets
            self.DisableAllButtons()
            self.threadUpdateFirmware.finished.connect(self.EnableAllButtons)
            #self.threadUpdateFirmware.finished.connect(
            #    lambda: self.UpdateFirmwareButton.setEnabled(True)
            #)
            # self.thread.finished.connect(
            #    lambda: self.stepLabel.setText("Long-Running Step: 0")
            # )
        else:
            print("No firmware file is chosen")
            self.ConsoleList.addItem("No firmware file is chosen")

    def WriteWiFiDataButtonOnClick(self):
        self.ConsoleList.addItem('Writing WiFi data...')
        # Step 2: Create a QThread object
        self.threadSetWiFiData = QThread()
        # Step 3: Create a worker object
        self.workerSetWiFiData = WorkerSetWiFiData()
        # Step 4: Move worker to the thread
        self.workerSetWiFiData.moveToThread(self.threadSetWiFiData)
        # Step 5: Connect signals and slots
        self.threadSetWiFiData.started.connect(self.workerSetWiFiData.run)
        self.workerSetWiFiData.finished.connect(self.threadSetWiFiData.quit)
        self.workerSetWiFiData.finished.connect(self.workerSetWiFiData.deleteLater)
        self.threadSetWiFiData.finished.connect(self.threadSetWiFiData.deleteLater)
        self.workerSetWiFiData.progress.connect(self.reportProgressSetWiFiData)

        self.workerSetWiFiData.wifi_ssid = self.SSIDEdit.text()
        self.workerSetWiFiData.wifi_pass = self.PassEdit.text()
        # Step 6: Start the thread
        self.threadSetWiFiData.start()

        # Final resets
        self.DisableAllButtons()
        self.threadSetWiFiData.finished.connect(self.EnableAllButtons)
        #self.WriteWiFiDataButton.setEnabled(False)
        #self.threadSetWiFiData.finished.connect(
        #    lambda: self.WriteWiFiDataButton.setEnabled(True)
        #)

    def ReadWiFiDataButtonOnClick(self):
        self.ConsoleList.addItem('Reading WiFi data...')
        # Step 2: Create a QThread object
        self.threadGetWiFiData = QThread()
        # Step 3: Create a worker object
        self.workerGetWiFiData = WorkerGetWiFiData()
        # Step 4: Move worker to the thread
        self.workerGetWiFiData.moveToThread(self.threadGetWiFiData)
        # Step 5: Connect signals and slots
        self.threadGetWiFiData.started.connect(self.workerGetWiFiData.run)
        self.workerGetWiFiData.finished.connect(self.threadGetWiFiData.quit)
        self.workerGetWiFiData.finished.connect(self.workerGetWiFiData.deleteLater)
        self.threadGetWiFiData.finished.connect(self.threadGetWiFiData.deleteLater)
        self.workerGetWiFiData.progress.connect(self.reportProgressGetWiFiData)

        # Step 6: Start the thread
        self.threadGetWiFiData.start()

        # Final resets
        self.DisableAllButtons()
        self.threadGetWiFiData.finished.connect(self.EnableAllButtons)
             #lambda: self.ReadWiFiDataButton.setEnabled(True)
        #)
    def GetVerButtonOnClick(self):
        self.ConsoleList.addItem('Getting firmware version...')
        # Step 2: Create a QThread object
        self.threadGetVer = QThread()
        # Step 3: Create a worker object
        self.workerGetVer = WorkerGetVer()
        # Step 4: Move worker to the thread
        self.workerGetVer.moveToThread(self.threadGetVer)
        # Step 5: Connect signals and slots
        self.threadGetVer.started.connect(self.workerGetVer.run)
        self.workerGetVer.finished.connect(self.threadGetVer.quit)
        self.workerGetVer.finished.connect(self.workerGetVer.deleteLater)
        self.threadGetVer.finished.connect(self.threadGetVer.deleteLater)
        self.workerGetVer.progress.connect(self.reportProgressGetVer)

        # Step 6: Start the thread
        self.threadGetVer.start()

        # Final resets
        self.DisableAllButtons()
        self.threadGetVer.finished.connect(self.EnableAllButtons)
        #self.GetVerButton.setEnabled(False)
        #self.threadGetVer.finished.connect(
        #    lambda: self.GetVerButton.setEnabled(True)
        #)


app = QApplication(sys.argv)
win = Window()
win.show()
sys.exit(app.exec())
