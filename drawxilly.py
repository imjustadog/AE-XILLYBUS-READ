import threading
import visdom
import numpy as np
import struct
import time
import sys
import os

class Draw_Visdom:
    def __init__(self):
        self.folderlist = os.listdir("/home/adoge/eclipse-workspace/savexilly")
        tFolder = threading.Thread(name="folderDetector", target=self.detectfolder)
        tFolder.start()

 
    def detectfolder(self):
        while True:
            flag = False
            folderlisttemp = os.listdir("/home/adoge/eclipse-workspace/savexilly")
            for folder in folderlisttemp:
                if folder not in self.folderlist:
                    flag = True
                    break
            if flag == True:
                break
            time.sleep(1)
        self.foldername = folder
        tData = threading.Thread(name="dataGenerator", target=self.detectdata)
        tData.start()
    
    

    def detectdata(self):
        folderpath = "/home/adoge/eclipse-workspace/savexilly" + os.path.sep + self.foldername + os.path.sep
        count = 0 
	while True:
	    datalisttemp = os.listdir(folderpath)
            if str(count) in datalisttemp:
                path = folderpath + str(count)
                viz = visdom.Visdom()
                win = viz.line(
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
                self.drawline(path,viz,win)
                count = count + 1                 
	    time.sleep(0.5)
    
    def drawline(self,path,viz,win):
        fb = open(path, "rb")
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
            datay.append(ch1)
            x = x + 1

	viz.updateTrace(
            X=np.array(range(x)) * 0.0000001,
            Y=np.array(datay),
            win=win,
            name = "1"
	)
	fb.close()


if __name__=="__main__":
    draw = Draw_Visdom()
