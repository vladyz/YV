# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.

import socket
import time

import matplotlib.pyplot as plt
import matplotlib.animation as animation

# Create figure for plotting
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = []
ys = []

x = 0



sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('192.168.0.105', 3333))
print('connected!')


def animate( i,  xs, ys):
    global x
    # Read temperature (Celsius) from TMP102
    data, addr = sock.recvfrom(160)  # buffer size is 1024 bytes
    N = 41
    val =[0] *N
    for idxi in range(1, 41):
        val[idxi] = (data[1 + (idxi-1)*4] << 8) | data[(idxi-1)*4]
        xs.append(x)
        ys.append(val[idxi])
        x = x + 1

    # print("received message: %s" % data)
    # Add x and y to lists

    # Limit x and y lists to 20 items
    xs = xs[-400:]
    ys = ys[-400:]

    # Draw x and y lists
    ax.clear()
    ax.plot(xs, ys)

    # Format plot
    # plt.xticks(rotation=45, ha='right')
    # plt.subplots_adjust(bottom=0.30)
    # plt.title('TMP102 Temperature over Time')
    # plt.ylabel('Temperature (deg C)')

def main():
    #sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    #sock.bind(('192.168.0.105', 3333))
    #print('connected!')
    #while True:
        #data, addr = sock.recvfrom(60)  # buffer size is 1024 bytes
        #print("received message: %s" % data)

        ani = animation.FuncAnimation(fig, animate, fargs=(xs, ys), interval=10)
        plt.show()

if __name__ == "__main__":
    main()

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
