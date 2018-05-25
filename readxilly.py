import struct
fb = open("/dev/xillybus_read_32","rb")
def r_event():
    data = fb.read(4)
    if not data:
	return
    ch1,ch2 = struct.unpack('<HH',data)
    ch1 = (float(ch1) - 8192) / 8192 * 2.5
    ch2 = (float(ch2) - 8192) / 8192 * 2.5
    print "ch1 %.2f, ch2 %.2f" % (ch1,ch2)

while(1):
    r_event()

fb.close()
