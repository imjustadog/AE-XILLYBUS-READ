import threading
import visdom
import numpy as np
import struct
import time

def receivedata(win,viz):
	#while True:
	    #fb = open("/dev/xillybus_read_32", "rb")
	    fb = open("/home/adoge/xillybus_save_32", "rb")
	    x = 0
            datax = []
            datay = []
	    while True:
		data = fb.read(4)
		if not data:
		    break
		ch1, ch2 = struct.unpack('<HH', data)
		ch1 = (float(ch1) - 8192) / 8192 * 2.5
		ch2 = (float(ch2) - 8192) / 8192 * 2.5
		ch1 = float(ch1)
		ch2 = float(ch2)
                x = x + 1
                datax.append(x * 0.0000001)
                datay.append(ch1)
	    viz.updateTrace(
                X=np.array(datax),
                Y=np.array(datay),
                win=win,
                name = "1"
	    )
            #viz.updateTrace(
	    #	X=np.array([x]),
	    #	Y=np.array([ch2]),
	    #	win=win,
            #        name = "2"
	    #)
		
	    fb.close()
	    #time.sleep(1)


viz = visdom.Visdom()
win=viz.line(
    X=np.array([0]),
    Y=np.array([0]),
    name="1"
)
#viz.line(
#    X=np.array([0]),
#    Y=np.array([0]),
#    win = win,
#    update = "new",
#    name="2"
#)
tData = threading.Thread(name="dataGenerator", target=receivedata, args=(win,viz))
tData.start()
