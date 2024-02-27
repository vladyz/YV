import asyncio
import sys
from time import sleep
from bleak import BleakClient, BleakScanner

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import (
    QApplication,
    QLabel,
    QMainWindow,
    QPushButton,
    QVBoxLayout,
    QWidget,
    QLineEdit,
    QListWidget,
)

from PyQt6.QtCore import QObject, QThread, pyqtSignal

WIFI_SSID_DATA_UUID = 'CC08D0BD-9044-4CD2-85F5-A488BD4412C8'
WIFI_PASS_DATA_UUID = 'e10334f1-22f8-4b97-bcc6-ebd39a6f8e49'
FIRMWARE_VERSION_UUID = '3ae1a84d-ddd3-4ef1-922a-70921cc5a914'

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
    progress = pyqtSignal(str)

    wifi_ssid =''
    wifi_pass =''
    def run(self):
        #self.progress.emit('Search esp32...')
        asyncio.run(esp32_set_wifi_data(self))
        self.finished.emit()
# class for getting wifi data
class WorkerGetWiFiData(QObject):
    finished = pyqtSignal()
    progress = pyqtSignal(str)
    def run(self):
        #self.progress.emit('Search esp32...')
        asyncio.run(esp32_get_wifi_data(self))
        self.finished.emit()
# class for getting version of firmware
class WorkerGetVer(QObject):
    finished = pyqtSignal()
    progress = pyqtSignal(str)
    def run(self):
        #self.progress.emit('Search esp32...')
        asyncio.run(esp32_get_firmware_ver(self))
        self.finished.emit()



# Step 1: Create a worker class
class Worker(QObject):
    finished = pyqtSignal()
    progress = pyqtSignal(str)

    def run(self):
        #self.progress.emit('Search esp32...')
        asyncio.run( _search_for_esp32(self))
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

        self.SSIDLabel = QLabel("Wifi SSID:")
        layout.addWidget(self.SSIDLabel)
        self.SSIDEdit = QLineEdit()
        layout.addWidget(self.SSIDEdit)


        self.PassLabel = QLabel("Wifi Password:")
        layout.addWidget(self.PassLabel)
        self.PassEdit = QLineEdit()
        layout.addWidget(self.PassEdit)

        self.ReadWiFiDataButton = QPushButton("Read WiFi Data")
        layout.addWidget(self.ReadWiFiDataButton)
        self.ReadWiFiDataButton.clicked.connect(self.ReadWiFiDataButtonOnClick)

        self.WriteWiFiDataButton = QPushButton("Write WiFi Data")
        layout.addWidget(self.WriteWiFiDataButton)
        self.WriteWiFiDataButton.clicked.connect(self.WriteWiFiDataButtonOnClick)

        self.GetVerButton = QPushButton("Get Version")
        layout.addWidget(self.GetVerButton)
        self.GetVerButton.clicked.connect(self.GetVerButtonOnClick)


        self.longRunningBtn = QPushButton("Long-Running Task!", self)
        self.longRunningBtn.clicked.connect(self.runLongTask)

        # Set the layout
        layout.addStretch()
        layout.addWidget(self.longRunningBtn)

        self.ConsoleList = QListWidget()
        layout.addWidget(self.ConsoleList)

        self.centralWidget.setLayout(layout)

        wdth = 300
        hght = 400
        self.setFixedWidth(wdth)
        self.setFixedHeight(hght)

    def reportProgress(self, n):
        self.stepLabel.setText(n) #(f"Long-Running Step: {n}")

    def reportProgressGetVer(self, n):
        self.ConsoleList.addItem(n)#(f"Long-Running Step: {n}")
    def reportProgressGetWiFiData(self, n):
        self.ConsoleList.addItem(n)#(f"Long-Running Step: {n}")
    def reportProgressSetWiFiData(self, n):
        self.ConsoleList.addItem(n)#(f"Long-Running Step: {n}")

    def runLongTask(self):
        # Step 2: Create a QThread object
        self.thread = QThread()
        # Step 3: Create a worker object
        self.worker = Worker()
        # Step 4: Move worker to the thread
        self.worker.moveToThread(self.thread)
        # Step 5: Connect signals and slots
        self.thread.started.connect(self.worker.run)
        self.worker.finished.connect(self.thread.quit)
        self.worker.finished.connect(self.worker.deleteLater)
        self.thread.finished.connect(self.thread.deleteLater)
        self.worker.progress.connect(self.reportProgress)
        # Step 6: Start the thread
        self.thread.start()

        # Final resets
        self.longRunningBtn.setEnabled(False)
        self.thread.finished.connect(
            lambda: self.longRunningBtn.setEnabled(True)
        )
        #self.thread.finished.connect(
        #    lambda: self.stepLabel.setText("Long-Running Step: 0")
        #)
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
        self.WriteWiFiDataButton.setEnabled(False)
        self.threadSetWiFiData.finished.connect(
            lambda: self.WriteWiFiDataButton.setEnabled(True)
        )

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
        self.ReadWiFiDataButton.setEnabled(False)
        self.threadGetWiFiData.finished.connect(
            lambda: self.ReadWiFiDataButton.setEnabled(True)
        )
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
        self.GetVerButton.setEnabled(False)
        self.threadGetVer.finished.connect(
            lambda: self.GetVerButton.setEnabled(True)
        )


app = QApplication(sys.argv)
win = Window()
win.show()
sys.exit(app.exec())
