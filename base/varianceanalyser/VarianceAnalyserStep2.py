import math
import stats
import sys

if __name__ == "__main__":
    ballDistVars = ([],"ballDistVars.dat") 
    ballHeadVars = ([],"ballHeadVars.dat") 
    beaconDistVars = ([],"beaconDistVars.dat") 
    beaconHeadVars = ([],"beaconHeadVars.dat") 
    goalDistVars = ([],"goalDistVars.dat") 
    goalHeadVars = ([],"goalHeadVars.dat")
    
    allDistVars = [ballDistVars,
                   ballHeadVars,  
                   beaconDistVars,
                   beaconHeadVars,
                   goalDistVars,
                   goalHeadVars]
                 
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
            mean = stats.mean(data)
            var = math.sqrt(stats.var(data))
        
            print "file name :", fileName
            print "mean      :", mean
            print "variance  :", var
            
            if fileName.find("logBallDist") != -1:
                ballDistVars[0].append((mean,var))
            elif fileName.find("logBallHead") != -1: 
                ballHeadVars[0].append((mean,var))    
            elif fileName.find("logBeaconDist") != -1: 
                beaconDistVars[0].append((mean,var))
            elif fileName.find("logBeaconHead") != -1:
                beaconHeadVars[0].append((mean,var))
            elif fileName.find("logGoalDist") != -1:
                goalDistVars[0].append((mean,var))
            elif fileName.find("logGoalHead") != -1:
                goalHeadVars[0].append((mean,var))

    for (distVars,fileName) in allDistVars: 
        fileBuf = open(fileName,'w') 
        for (mean,var) in distVars: 
            fileBuf.write(str(mean) + " " + str(var) + "\n")
            
