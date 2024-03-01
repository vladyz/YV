"""
This example demonstrates many of the 2D plotting capabilities
in pyqtgraph. All of the plots may be panned/scaled by dragging with
the left/right mouse buttons. Right click on any plot to show a context menu.
"""
import socket
import numpy as np

import pyqtgraph as pg
from pyqtgraph.Qt import QtCore
import PyQt6
from PyQt6.QtWidgets import (
    QMainWindow
)

# --------------------------------------------------------------
# connect to esp32
HOST = socket.gethostname()
PORT = 3333
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind((HOST, PORT))
sock.listen(1)
conn, addr = sock.accept()
print("connected with ",  addr)

# --------------------------------------------------------------

app = pg.mkQApp("Plotting Example")
# mw = QtWidgets.QMainWindow()
# mw.resize(800,800)

win = pg.GraphicsLayoutWidget(show=True, title="Basic plotting examples")
win.resize(1000, 600)
win.setWindowTitle('pyqtgraph example: Plotting')

# Enable antialiasing for prettier plots
pg.setConfigOptions(antialias=True)



p6 = win.addPlot(title="Updating plot")
curve = p6.plot(pen='y')
# data = np.random.normal(size=(10,1000))
ptr = 0

x = [0] * 2000
y = [0] * 2000
k = 0



def update():
    global curve, data, ptr, p6, x, y, k

    for j in range(0, 10):
        data = conn.recv(1440)
        # print('received: ', data)
        for i in range(0, 1440, 128):
            buff = ((data[i + 3] << 16) | (data[i + 2] << 8) | (data[i + 1]))
            if buff & 0x800000:
                buff = buff - 0xFFFFFF
            y.append(buff)
            #x.append(k)
            #k = k + 1
    #x = xs[-2000:]
    ys = y[-2000:]
    curve.setData(ys)#data[ptr%10])
    #if ptr == 0:
        #p6.enableAutoRange('xy', False)  ## stop auto-scaling after the first data set is plotted
    ptr += 1

timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(50)


            #print(k)






if __name__ == '__main__':
    pg.exec()
