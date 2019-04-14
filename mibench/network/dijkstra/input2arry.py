f = open("input.dat")
p = open

i=0
for line in f:
    j=0;
    line = line.split()
    for item in line:
        print "AdjMatrix[{0:4}][{1:4}] = {2:4} ;".format(i,j,item);
        j +=1
    i+=1;

f.close()
