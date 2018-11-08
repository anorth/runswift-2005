import math 
import sys

def var(data):
    m = mean(data)
        
    sum = 0    
    for d in data: 
        sum += (d - m) ** 2        
    return (sum + 0.0) / len(data)
    

def mean(data): 
    
    sum = 0
    for d in data:
        sum += d

    return (sum + 0.0) / len(data)


if __name__ == "__main__": 

    fileNames = sys.stdin.readlines() 
    for fileName in fileNames:                  
        fileName = fileName.strip()
        fileBuf = open(fileName)      
        lines = fileBuf.readlines()
        data = []
        for l in lines:             
            try:
                data.append(float(l))
            except ValueError: 
                pass
        if len(data) > 0:
            print "file name :", fileName
            print "mean      :", mean(data)
            print "variance  :", var(data)

        else:
            print "file name :", fileName
            print "----------: No data"            
        print ""
