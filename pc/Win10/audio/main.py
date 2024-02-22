# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.

import socket
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np

fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
ax.clear
xs = [0] * 20000
ys = [0] * 20000
k=0
#line, = ax.plot(xs, ys)

def animate(i, xs, ys):
    # Limit x and y lists to 20 items
    global k

    for j in range(0, 10):
        data = conn.recv(1440)
        #print('received: ', np.array)
        for i in range(0, 1440, 128):
            buff = ((data[i+3]<<16) | (data[i+2]<<8) | (data[i+1])  )
            if buff & 0x800000:
                buff = buff - 0xFFFFFF
            ys.append( buff )
            xs.append(k)
            k = k + 1
    #print(k)

    x = xs[-2000:]
    y = ys[-2000:]

   # line.set_xdata(x)
   # line.set_ydata(y)
    #return line
    # Draw x and y lists
    ax.clear()
    ax.plot(x, y)

#def main():

HOST = socket.gethostname()
PORT = 3333
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind((HOST, PORT))
sock.listen(1)
conn, addr = sock.accept()
print("connected with ",  addr)
#while True:
    #data = conn.recv(1440)
    #print('received: %s', data)

    #for j in range(0, 8):
    #    data = conn.recv(1440)
    #    print('received: %s', data)
    #    for i in range(0, 1440, 8):
     #       ys.append( ( (data[i+3]<<16) | (data[i+2]<<8) | (data[i+1])  ) )
    #        xs.append(x)
     #       x = x + 1
        #print('received: %s', ys)


ani = animation.FuncAnimation(fig, animate, fargs=( xs, ys), interval=50)
plt.show()




#if __name__ == "__main__":
#    main()

