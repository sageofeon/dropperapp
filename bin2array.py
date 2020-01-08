import sys


def bin2array(s, name):
    s = bytearray(s)
    o = ""
    o += ("static const unsigned char %s[]={" % name)
    for i in range(0, len(s)):
        if i % 32 == 0:
            o += "\n"
        a = s[i]
        a ^= 0x5
        o += ("0x%2.2X," % a)
    o += "\n};\n"
    return o


l = len(sys.argv)
if l < 1:
    print("Error: invalid argument\npython bin2array.py file.exe")
    sys.exit()
f = open(sys.argv[1], "rb")
data = f.read()
f.close()

data = bin2array(data, "x_array")
f = open("xor_array.h", "wb+")
f.write(data.encode())  # string to bytes
f.close()
print("Done")
