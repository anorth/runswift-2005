##     Copyright 2005 The University of New South Wales (UNSW) and National  
##     ICT Australia (NICTA).
##  
##     This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
##     redistribute it and/or modify it under the terms of the GNU General  
##     Public License as published by the Free Software Foundation; either  
##     version 2 of the License, or (at your option) any later version as  
##     modified below.  As the original licensors, we add the following  
##     conditions to that license:
##  
##     In paragraph 2.b), the phrase "distribute or publish" should be  
##     interpreted to include entry into a competition, and hence the source  
##     of any derived work entered into a competition must be made available  
##     to all parties involved in that competition under the terms of this  
##     license.
##  
##     In addition, if the authors of a derived work publish any conference  
##     proceedings, journal articles or other academic papers describing that  
##     derived work, then appropriate academic citations to the original work  
##     should be included in that publication.
##  
##     This rUNSWift source is distributed in the hope that it will be useful,  
##     but WITHOUT ANY WARRANTY; without even the implied warranty of  
##     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
##     General Public License for more details.
##  
##     You should have received a copy of the GNU General Public License along  
##     with this source code; if not, write to the Free Software Foundation,  
##     Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


###############################################################################
# These are all math-related helper functions
################################################################################
import Constant
import math

def get95cf(stddev):
    return SQUARE(stddev) / 4.0


# takes in a value in cm     (as radius for position)
#               or in degree (as angle for heading)
# returns a variance value.
# usage example -->
# if MyPositionVariance < hMath.get95var(20):
#   print "I am 95% certain i am within a 20cm radius of my given position"
def get95var(twoStdDev):
    return SQUARE(twoStdDev) / 4.0

    
def DECREMENT(x):
    if x > 0:
        x -= 1
    else:
        x = 0
    return x
    
def yuvKey(y,u,v):
    tmp = (\
    (math.floor(y/2) * 128 * 128) + \
    (math.floor(u/2) * 128) + \
    (math.floor(v/2)))
    return (tmp, hex(int(tmp)))
    
# Convert micro-"something" to "something"
def MICRO2NORMAL(x):
    return (x / 1000000)

    
# Convert "something" to micro-"something"
def NORMAL2MICRO(x):
    return (x * 1000000)

    
# Convert coordinates from top-left corner oriented to center oriented.
def TOPLEFT2CENTER_X(x):
    return ((x) - (Constant.IMAGE_WIDTH / 2))

def TOPLEFT2CENTER_Y(y):
    return ((Constant.IMAGE_HEIGHT / 2) - (y))

# Convert radians to degrees
def RAD2DEG(x):
    return x * 180.0 / math.pi 
    
def MICRORAD2DEG(x): 
    return RAD2DEG(x) / 1000000

    
# Convert degrees to radians
def DEG2RAD(x):
    return x * math.pi / 180.0
    
def SQRT(x):
    return math.sqrt(x)
    
# var a is getting the sign from var s
# if s = 0, then a = 0, too
def getSign(a, s):
    if s > 0:
        return abs(a)
    elif s < 0:
        return -abs(a)
    else:
        return 0
    
# Returns x or LIM, whichever is smaller
def CLIP(x, LIM):
    if x > LIM : return LIM
    elif x < -LIM : return -LIM
    else: return x
    
# Returns max if X > max, min if x < min, else x.
def CLIPTO(x, min, max):
    if x > max : return max
    elif x < min : return min
    else: return x

# Returns x or LIM, whichever is larger
def EXTEND(x,LIM):
    if 0 <= x < LIM:
        return LIM
    elif -LIM < x < 0:
        return -LIM
    else:
        return x
    
# Clip a coordinate to a point in the field    
def clipPosition((x,y)):
    return (CLIPTO(x, 0, Constant.FIELD_WIDTH) , \
            CLIPTO(y, 0, Constant.FIELD_LENGTH))
    
    
# Return angle in degrees satisfying 0 <= angle < 360
def normalizeAngle_0_360(angle):
    while angle < 0:
        angle += 360
    while angle >= 360:
        angle -= 360
    return angle

    
# Return angle in degrees satisfying -180 < angle <= 180
def normalizeAngle_180(angle):
    while angle <= -180:
        angle += 360
    while angle > 180:
        angle -= 360
    return angle

# Return angle in radians satisfying 0 <= angle < 2*pi
def normalizeAngle_0_2pi(angle):
    while angle < 0:
        angle += 2*math.pi
    while angle >= 2*math.pi:
        angle -= 2*math.pi
    return angle

    
# Return angle in radians satisfying -pi < angle <= pi
def normalizeAngle_pi(angle):
    while angle <= -math.pi:
        angle += 2*math.pi
    while angle > math.pi:
        angle -= 2*math.pi
    return angle
    
# Returns the cartesian length of a vector of any dimension
def getLength(vec):
    length = 0
    for dimension in vec:
        length = length + dimension * dimension
    return math.sqrt(length)

    
# Scales the given n-vector to unit length
def normalise(vec):
    length = getLength(vec)
    if (length > 0):
        return map(lambda x:x/length, vec)
    else:
        return vec

        
# Rotates a 2D vector counter-clockwise by the given angle in degrees
def rotate(vec, angle, rad = False):
    if rad:
        sinA = math.sin(angle)
        cosA = math.cos(angle)
    else:
        sinA = math.sin(DEG2RAD(angle))
        cosA = math.cos(DEG2RAD(angle))
    x = cosA * vec[0] - sinA * vec[1]
    y = sinA * vec[0] + cosA * vec[1]
    return (x, y)


# Returns the global heading corresponding to the specified local heading
# (degrees)
def local2GlobalHeading(loc, local):
    return normalizeAngle_180(loc.getHeading() + local)

# Returns the local heading corresponding to the specified global heading
# (degrees)
def global2LocalHeading(loc, glob):
    return normalizeAngle_180(glob - loc.getHeading())
    
# Returns a relative heading to the specified absolute coordinates (degrees)
# (Positive y is zero)
def getHeadingToAbsolute(loc, objX, objY):
    return getHeadingToMe(loc.getX(), loc.getY(), loc.getHeading(), objX,objY)


# Returns a relative heading to the specified relative coordinates
# in -pi/2 .. pi/2
def getHeadingToRelative(x,y):
    if x == 0: return 0
    if x > 0: return math.atan( y / x) - math.pi / 2 
    else: return math.atan( y / x) + math.pi / 2 

    
# Returns the cartesian distance between two points
def getDistanceBetween(fromX, fromY, toX, toY):
    relx = toX - fromX
    rely = toY - fromY
    return math.sqrt((relx * relx) + (rely * rely)) 
    
    
# Returns the cartesian distance squared between two points
def getDistSquaredBetween(fromX, fromY, toX, toY):
    relx = toX - fromX
    rely = toY - fromY
    return (relx * relx) + (rely * rely)
    
    
# Returns the heading (degrees) from point X to point Y (positive y is
# zero degrees)
def getHeadingBetween(fromX, fromY, toX, toY):
    relx = toX - fromX
    rely = toY - fromY
    if relx == 0 and rely == 0:
        return 0
    return RAD2DEG(math.atan2(rely, relx))

    
# Returns the point with the given heading (degrees) and distance from
# the specified base point
def getPointRelative(baseX, baseY, offsetHead, offsetDist):
    rx = baseX + offsetDist * math.cos(DEG2RAD(offsetHead))
    ry = baseY + offsetDist * math.sin(DEG2RAD(offsetHead))
    return (rx,ry)
    
    
# Return Cartesian coordinates (x,y) given polar coordinates (dist, degree).
# Positive x axis is zero degrees, increasing counter-clockwise
def polarToCart(r,alpha):
    t = DEG2RAD(alpha)
    return (r * math.cos(t) , r * math.sin(t) )

    
# Return polar coordinates (dist, degree) given Cartesian coordinates
# relative to origin. Result will be >= 0 and < 360.
def cartToPolar(dx,dy):
    return normalizeAngle_0_360(RAD2DEG(math.atan2(dy, dx)))
    
# Returns a list of nrow lists of ncol zeros
def array2D(nrow,ncol):
    l = []
    for _ in range(nrow):
        sl = []
        for _ in range(ncol):
            sl.append(0)
        l.append(sl)
    return l

    
# Given global coordinates x y, calculate distance and heading (+ve left) to it.
def findAngleDist(loc, x, y):
    dx = x - loc.getX()
    dy = y - loc.getY()
    dist = math.sqrt((dx * dx) + (dy * dy))
    
    head = normalizeAngle_180(RAD2DEG(math.atan2(dy, dx)) - loc.getHeading())

    return [dist, head]
    
    
# Return the abs angle between 2 points, taking this pivot point as the origin
# eg of use, a is robot ,b is goal, pivot is ball
# ie. the angle between the robot and the goal via the ball ie. angle a-pivot-b.
def absAngleBetweenTwoPointsFromPivotPoint(pointax, pointay, pointbx, pointby, pivotx, pivoty):
    asquared = SQUARE(pointbx - pointax) + SQUARE(pointby - pointay)
    bsquared = SQUARE(pointbx - pivotx)  + SQUARE(pointby - pivoty)
    csquared = SQUARE(pivotx - pointax)  + SQUARE(pivoty - pointay)

    squaredSum  = bsquared + csquared - asquared
    twobc       = 2 * math.sqrt(bsquared) * math.sqrt(csquared)
    
    if abs(twobc) <= 0.0001:
        twobc += 0.01

    result = abs(RAD2DEG(math.acos(squaredSum / twobc)))
    return result
    

# squaring function for above
def SQUARE(x):
    return (x*x);
    

#TODO: make the above use this one for coherence
def angleBetweenTwoPointsFromPivotPoint(pointax, pointay, pointbx, pointby, pivotx, pivoty):
    h1 = getHeadingBetween(pivotx,pivoty,pointax,pointay)
    h2 = getHeadingBetween(pivotx,pivoty,pointbx,pointby)
    return normalizeAngle_180(h2-h1)
    
# Returns a xor b
def xor(a, b):
    # Truth values in Python are usually "bits" (0 or 1)
    # so bit-manipulation operators work fine with them.
    return a ^ b
    #return ( a and not b ) or (b and not a)


    
# Returns the two points of intersection of the specified two circles.
def intersectCircle(x1,y1,r1,x2,y2,r2):
    d = getDistanceBetween(x1,y1,x2,y2)
    res = []
    if d > r1+r2: return res
    
    semi_p = (d + r1 + r2)/2.0
    area = math.sqrt(semi_p * (semi_p - d) * (semi_p - r1) * (semi_p - r2))
    h = 2*area / d;
    alpha = math.asin(h/r1)
    if r2 > d and r2 > r1: # alpha > 90
        alpha = math.pi - alpha
    
    heading = getHeadingBetween(x1,y1,x2,y2)
    (x,y) = getPointRelative( x1,y1, heading, r1)
    dx,dy = x-x1,y-y1
    dx1,dy1 = rotate( (dx,dy) , alpha, rad=True)
    dx2,dy2 = rotate( (dx,dy) , -alpha, rad=True)
    res.append( (x1 + dx1, y1 + dy1 ) )
    res.append( (x1 + dx2, y1 + dy2 ) )
    return res
    
#convert global to local coordinate
#for eg: myX,myY,myH = self position, destX,destY = global ball
#return local ball. (zero y axis)
def getLocalCoordinate(myX,myY,myH,x,y):
    x -= myX
    y -= myY
    return rotate( (x,y), 90-myH)

# convert local to global coordinate (zero y axis)
def getGlobalCoordinate(myX,myY,myH,x,y):
    dx,dy = rotate((x,y), myH - 90)    
    return (dx + myX, dy + myY)

# get local heading from global coordinate (degrees) (positive y is zero
# degrees)
def getHeadingToMe(myX,myY,myH,x,y):
    return normalizeAngle_180( getHeadingBetween(myX,myY,x,y) - myH)

def distanceToLeftEdge(x,h):
    return x / math.cos(DEG2RAD(180 - h))
    
def distanceToRightEdge(x,h):
    return (Constant.FIELD_WIDTH - x) / math.cos(DEG2RAD(h))
    
def distanceToBottomEdge(y,h):
    return y / math.cos(DEG2RAD(270 - h))
    
def distanceToTopEdge(y,h):
    return (Constant.FIELD_LENGTH - y) / math.cos(DEG2RAD(90 - h))
    
#how far I am from the edge, facing that heading
def distanceToEdge(x,y,h):
    minD = 1000000
    if h > 90 and h < 270:
        minD = min(minD, distanceToLeftEdge(x,h))
    elif h < 90 or h > 270:   
        minD = min(minD, distanceToRightEdge(x,h))
        
    if h > 0 and h < 180:
        minD = min(minD, distanceToTopEdge(y,h))
    elif h > 180:
        minD = min(minD, distanceToBottomEdge(y,h))
    
    return minD

#
def getDistanceToClosestEdge(x,y):
    minD = 1000000
    if x < minD: minD = x
    if Constant.FIELD_WIDTH - x < minD : minD = Constant.FIELD_WIDTH - x
    if y < minD: minD = y
    if Constant.FIELD_LENGTH - y < minD : minD = Constant.FIELD_LENGTH - y
    return minD


#---------------------------------------
def getHeadingToBall(x,y):
    """Convert coordinate (x,y) relatively to the robot
    to heading (-pi/2 : left , pi/2 : right )
    """
    if x == 0: return 0
    if x > 0: return math.atan( y / x) - math.pi / 2 
    else: return math.atan( y / x) + math.pi / 2 

    
#---------------------------------------
# Return a heading to global X, Y. Returned heading is local with y axis
# at zero degrees
def getHeadingToFaceAt(loc, objX, objY):
    return getHeadingToMe(loc.getX(), loc.getY(), loc.getHeading(), objX,objY)
    

#---------------------------------
def mean(vec): 
    if len(vec) > 0: 
        return sum(vec) / len(vec)
    else: 
        return 0

# RETURN: the time elapsed in MILLI-seconds
# takes 2 time tuples in (sec, mirco-sec) form,
def getTimeElapsed(oldTime, newTime):
    return (newTime[0] - oldTime[0]) * 1000.0 +\
            (newTime[1] - oldTime[1]) / 1000.0
# The algo below is unnecessary
#    if newTime[1] > oldTime[1]:
#        return (newTime[0] - oldTime[0]) * 1000.0 + \
#                (newTime[1] - oldTime[1]) / 1000.0
#    else:
#        return (newTime[0] - 1 - oldTime[0]) * 1000.0 + \
#                (newTime[1] + 1000000 - oldTime[1]) / 1000.0
