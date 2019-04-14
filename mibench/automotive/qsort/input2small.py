f = open("input_small.dat")
p = open
i=0

for line in f:
    line = "\""+line.strip()+"\""
    print "strncpy(array[{0:4}].qstring, {1:15} ,128);".format(i,line);
    i+=1

f.close()
