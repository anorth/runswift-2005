from WalkBasePyLink import setCallbacks
import sys
sys.path.append("./PyWalkLearner")
from GradientDescent import GradientDescent
from PowellDescent import PowellDescent

EllipticalWalkInitParams= [48, 94, 124, 44, -2, -64, 8, 18, 36, 24, 40, 100]
#[50, 92, 124, 45, -1, -63, 7, 20, 34, 22, 40, 95]
EllipticalWalkInitDeltaParams=[ 2,  2,  2,   1, 1 ,  1 ,1,  2,  2,  2,  2,  5]

SkellipticalWalkStartingParams = []
#30 80 115 15 15 30 25 55 15 -55 5 50 55 1 1 1 1 10 20 20 40  0 0 3 0 0

SkellipticalWalkDelta =          [ 1,  2,   2,  1,  1,  1,  1,  1,  1,   5, 1, \
                                   5,  5, 5, 5, 5, 5,  5,  5,  5,  5, \
                                   1, 0, 0, 0, 0]

# this can be altered later
#LearningAlgorithm = GradientDescent
LearningAlgorithm = PowellDescent
initialParams = EllipticalWalkInitParams
initialDeltaParams = EllipticalWalkInitDeltaParams

learningAlgo = None #LearningAlgorithm(True)

verbose = True

def setEvaluation(policyID, value):
    if verbose:
        print __name__, "setEvaluation(): got ", policyID, value
    learningAlgo.setEvaluation(policyID, value)
    


def getNewPolicy():    
    id, params = learningAlgo.getNextEvaluation()
    if params:
        params = roundToOneDecimal(params)
    return (id, params)

def roundToOneDecimal(params):
    params = list(params)
    l = len(params)
    for i in xrange(l):
        params[i] = round(params[i], 1)
    return tuple(params)


def getBestParams():
    return learningAlgo.getBestParameter()


def setInitParams(paramStr):
    global learningAlgo
    print paramStr
    
    strList = paramStr.strip().split()
    numList = map(float, strList)
    n = len(numList)
    if n % 3 != 0:
        print __name__, "setInitParams(): the length of number list:", n, "is NOT divisible by 3!"
        return

    print numList
    
    params = []
    minList = []
    maxList = []
    for i in xrange(n):
        num = numList[i]
        if i % 3 == 0:
            params.append(num)
        elif i % 3 == 1:
            minList.append(num)
        else: # if i % 3 == 2:
            maxList.append(num)    
    
    if verbose:
        print __name__, "setInitParams(): sending initial parameter list",
        print "to the learning algorithm:",
        print "Paramters: ", params
        print "Lower Limits: ", minList
        print "Uppert Limits: ", maxList
        
    learningAlgo = LearningAlgorithm(True) # reset learning algo
    learningAlgo.setParameters(params)
    learningAlgo.setLowerLimits(minList)
    learningAlgo.setUpperLimits(maxList)


print "Start initialisation of", __name__
setCallbacks(getNewPolicy, setEvaluation,
                          getBestParams, setInitParams)
#learningAlgo.setDeltaParameters(initialDeltaParams)
print "Finish initialisation of", __name__
