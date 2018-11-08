##  
##     Copyright 2004 The University of New South Wales (UNSW) and National  
##     ICT Australia (NICTA).
##  
##     This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
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
##     must be included in that publication.
##  
##     This rUNSWift source is distributed in the hope that it will be useful,  
##     but WITHOUT ANY WARRANTY; without even the implied warranty of  
##     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
##     General Public License for more details.
##  
##     You should have received a copy of the GNU General Public License along  
##     with this source code; if not, write to the Free Software Foundation,  
##     Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##  
##  
##  
##   Last modification background information
##   $Id$
##  
##   Copyright (c) 2004 UNSW
##   All Rights Reserved.
##  
##  



import numarray as Num
import optimize


##################### Caching and loading functions ######
LOG_FILE_NAME = "powell.log"
MARKING = "----"
cachingLineSearch = []

def parseFloatList(s):
    s = s.strip("\t\n []")
    l = s.split(",")
    return map(float,l)
    
def loadPowellFromFile(logFileName):
    global cachingLineSearch
    f = open(logFileName,"r")
    print "First line : ",f.readline()
    line = f.readline()
    while line[:4] == MARKING:
        base = parseFloatList(f.readline())
        dirVec = parseFloatList(f.readline())
        x,fx,fcalls = f.readline().split()
        x = float(x)
        fx = float(fx)
        fcalls = int(fcalls)
        cachingLineSearch.append( (base,dirVec,x,fx,fcalls))
        line = f.readline()
    f.close()

def storeCachedLineSearch(base,dirVec, x, fx, fcalls):
    f = open(LOG_FILE_NAME,"a")
    f.write("%s\n" % MARKING)
    f.write("\t%s\n" % base.tolist())
    f.write("\t%s\n" % dirVec.tolist())
    f.write("\t%f %f %d\n" % (x,fx,fcalls))
    f.close()

def storeDirectionVectors(xi):
    print __name__, "storing powell Vectors"
    f = open("powellVectors.log", "w")
    f.write("%s\n" % MARKING)
    for x in xi:
        f.write("%s\n" % x.tolist())
        print x.tolist()
    f.close()
    
    
FLOAT_ERROR = 0.00001
def equalFloat(a,b):
    return abs(a-b) < FLOAT_ERROR
    
def equalFloatList(listA,listB):
    if len(listA) != len(listB):
        return False
    for (a,b) in zip(listA,listB):
        if not equalFloat(a,b):
            return False
    return True
    
def loadCachedSearch(base,dirVec):
    for (_base,_dirVec,x,fx,fcalls) in cachingLineSearch:
        if equalFloatList(base,_base) and \
            equalFloatList(dirVec,_dirVec):
            return x,fx,fcalls
    return None
    
################################################

def SQR(x):
    return x*x

## def line_searchA(f, xk, pk, args=(), xtol=1e-4, ftol=1e-4):
##     print "minimizing from: "
##     print xk
##     print " Direction: "
##     print pk
    
##     gfk = apply(optimize.approx_fprime,(xk,f)+args)
##     alpha, fc, gc = optimize.line_search_BFGS(f, xk, pk, gfk, args)
##     p = xk + alpha*pk
##     ev = apply(f,(p,)+args)
##     print "Found alpha: %f" % alpha
##     return alpha, ev, fc

def SIGN(a, b):
    if (b > 0):
        return abs(a)
    else:
        return -abs(a)

def max(a, b):
    if (a >= b):
        return a
    else:
        return b

def mnbrak(f, base, dirVec, args, ax = 0, bx = 1, maxiter = 100):
    GOLD = 1.618034

    if (ax == bx):
        bx += 1

    #print "mnbrak started!"
    #print "base: ", base, "dir: ", dirVec, "ax:",ax,"bx:",bx
    
    fcalls = 0
    fa = apply(f,((base + ax*dirVec),)+args) # should never be too slow
    fcalls += 1
    fb = apply(f,((base + bx*dirVec),)+args)
    fcalls += 1

    #if (fb == fa):
    #    print "flat function!"
    
    if (fb > fa):
        dum = ax
        ax = bx
        bx = dum
        dum = fb
        fb = fa
        fa = dum
    
    cx = bx+GOLD*(bx-ax)    # Guess first point using the golden ratio
    fc = apply(f,((base + cx*dirVec),)+args)
    fcalls += 1

    iter = 0

    while (fb > fc):                            # Keep returning here until we bracket.
        iter += 1
        if (iter > maxiter):
            print "Too many iterations in mnbrak"
            return ax, fa, bx, fb, cx, fc, fcalls
        r = (bx-ax)*(fb-fc)                     # Compute u by parabolic extrapolation from a,b,c.
        q = (bx-cx)*(fb-fa)
        u = bx-((bx-cx)*q-(bx-ax)*r)/(2.0*SIGN(max(abs(q-r),TINY),q-r)) #TINY is used to prevent any possible division by zero
        ulim = bx+GLIMIT*(cx-bx) 

        if ((bx-u)*(u-cx) > 0.0):
            fu = apply(f,((base + u*dirVec),)+args)
            fcalls += 1
            if (fu < fc):
                ax = bx
                bx = u
                fa = fb
                fb = fu
                return ax, fa, bx, fb, cx, fc, fcalls
            elif (fu > fb):
                cx = u
                fc = fu
                return ax, fa, bx, fb, cx, fc, fcalls
            u=cx+GOLD*(cx-bx)
            fu = apply(f,((base + u*dirVec),)+args)
            fcalls += 1
        elif ((cx-u)*(u-ulim) > 0.0):
            fu = apply(f,((base + u*dirVec),)+args)
            fcalls += 1
            if (fu < fc):
                bx = cx
                cx = u
                u = cx+GOLD*(cx-bx)
                fb = fc
                fc = fu
                fu = apply(f,((base + u*dirVec),)+args)
                fcalls += 1
        elif ((u-ulim)*(ulim-cx) >= 0.0):
            u = ulim
            fu = apply(f,((base + u*dirVec),)+args)
            fcalls += 1
        else:
            u=cx+GOLD*(cx-bx);
            fu = apply(f,((base + u*dirVec),)+args)
            fcalls += 1
        ax = bx
        bx = cx
        cx = u
        fa = fb
        fb = fc
        fc = fu
    return ax, fa, bx, fb, cx, fc, fcalls


TINY = 1.0e-20
GLIMIT = 100.0
    
def line_search(f, base, dirVec, args=(), xtol=1e-4, ftol=1e-4, maxiter=None, ax = 0, bx = 1):
    res = loadCachedSearch(base,dirVec)
    if res is not None:
        return res
    
    CGOLD = 0.3819660
    ZEPS = 1.0e-10
    
    if maxiter is None:
        maxiter = 2 # was 100
    iter = 0
    
    fa = 0
    fb = 0
    fc = 0
    fbx = 0
    xmin = 0
    fx = 0
    e = 0

    ax, fa, bx, fb, cx, fc, fcalls = mnbrak(f, base, dirVec, args, ax, bx, maxiter=100)

    #print "ax:",ax,"bx:",bx,"cx:",cx
    
    if (ax < cx):
        a = ax
        b = cx
    else:
        a = cx
        b = ax
    
    x = w = v = bx
    fbx = fw = fv = fx = fb
    while (iter < maxiter):
        iter += 1
        xm = (a+b)/2.0
        tol1 = xtol*abs(x)+ZEPS
        tol2 = 2.0*tol1
        if abs(b-a) < 2*xtol:
            return x, fx, fcalls, min(1,max(10*xtol,abs(b-a)))
##         if abs(x-xm) <= (tol2-(b-a)/2) or abs(x-xm) < xtol:
##             print "linesearch finished! alpha: %f f(alpha*dirVec+base): %f nCalls: %d" % (x, fx, fcalls)
##             storeCachedLineSearch( base,dirVec,x,fx,fcalls )
##             return x, fx, fcalls    Exit with best values 
        
        if (abs(e) > tol1): # Construct a trial parabolic fit.
            print "line search: trying parabolic fit"
            r = (x-w)*(fx-fv)
            q = (x-v)*(fx-fw)
            p = (x-v)*q-(x-w)*r
            q = 2.0*(q-r)
            if (q > 0):
                p = -p
            q = abs(q)
            etemp = e
            e = d
            if (abs(p) >= abs(q*etemp/2)) or (p <= q*(a-x)) or (p >= q*(b-x)):
                if (x >= xm):
                    e = a-x
                else:
                    e = b-x
                d = CGOLD*e
            else:
                d = p/q
                u = x+d
                if ((u-a < tol2) or (b-u < tol2)):
                    d = SIGN(tol1, xm-x)
        else:
            print "line search: trying golden section search"
            if (x >= xm):
                e = a-x
            else:
                e = b-x
            d = CGOLD*e
        if (abs(d) >= tol1):
            u = x+d
        else:
            u = x+SIGN(tol1,d)
        fu = apply(f,((base + u*dirVec),)+args)
        fcalls += 1
        if (fu <= fx):
            if (u >= x):
                a = x
            else:
                b = x
            v = w
            w = x
            x = u
            fv = fw
            fw = fx
            fx = fu
        else:
            if (u < x):
                a = u
            else:
                b = u
            if ((fu <= fw) or (w == x)):
                v = w
                w = u
                fv = fw
                fw = fu
            elif ((fu <= fv) or (v == x) or (v == w)):
                v = u
                fv = fu
    print "linesearch: Too many iterations: ", iter
    storeCachedLineSearch( base,dirVec,x,fx,fcalls )
    return x, fx, fcalls, min(1,max(10*xtol,abs(b-a)))


        
def fminPowellNew(func, x0, args=(), startFromScratch = True, shouldBreak = True, xtol=1e-4, ftol=1e-4, xi=None, maxiter=None, maxfun=None, fulloutput=0, printmessg=1):
    if startFromScratch:
        print "Starting Powell from scratch..."
        f = open(LOG_FILE_NAME,"w")
        f.write("Powell log file\n")
        f.close()
    else:
        print "Loading Powell from file..."
        loadPowellFromFile(LOG_FILE_NAME)
    return fminPowell(func, x0, args, shouldBreak, xtol, ftol, xi, maxiter, maxfun, fulloutput, printmessg)
    
def fminPowell(func, x0, args=(), shouldBreak=True, xtol=1e-4, ftol=1e-4,
               xi=None, maxiter=None, maxfun=None, fulloutput=0, printmessg=1):
    """xopt,{fval,warnflag} = fmin(function, x0, args=(), xtol=1e-4, ftol=1e-4,
    maxiter=200*len(x0), maxfun=200*len(x0), fulloutput=0, printmessg=0)

    Uses a Powell's algorithm to find the minimum of function
    of one or more variables.  See Section 10.5 of Numerical Recipes in C.
    """
    p = Num.asarray(x0)
    assert (len(p.shape)==1)
    n = len(p)
    if maxiter is None:
        maxiter = 200
    iter = 0
    if maxfun is None:
        maxfun = n * 50000
    func_calls = 0
    if xi is None:
        xi = Num.identity(n, p.typecode())

    steps = [5]*n

    fret = apply(func,(p,)+args)
    func_calls += 1
    pt = p.copy()
    ptt = p.copy()
    xit = xi[0].copy()
    
    while True:
        ibig = -1
        delta = -1e20
        iter += 1
        if (iter > maxiter):
            if printmessg:
                print "Warning: Maximum number of iterations has been exceeded: ", iter
            warnflag = True
            break
        if (func_calls > maxfun):
            if printmessg:
                print "Warning: Maximum number of function evaluations has been exceeded: ", func_calls
            warnflag = True
            break
        # perform a line search in each direction    
        fptt = fp = fret
        for i in range(0,n):
            fptt = fret
            alpha, fret, fc, steps[i] = line_search(func, p, xi[i], args, xtol, ftol, 3, 0, steps[i])
            print "alpa:", alpha, "a-b/2", steps[i]
            #print __name__, "xi[i]", xi[i], "alpha: ", alpha
            print __name__, "p:", p
            p += alpha * xi[i]
            print __name__, "p:", p
            func_calls += fc
            if ((fptt - fret) > delta):
                delta = fptt - fret
                ibig = i
        if (delta < 0):
            if printmessg:
                print "Warning: All directions head up hill"
            warnflag = True
            break

        # find the total movement - and extrapolate in that direction
        #ptt = 2*p-pt

        xit = p-pt
        pt = p.copy()

        steps[ibig]=steps[0]
        xi[ibig]=xi[0]
        steps[0] = 1
        xi[0] = xit
        if not shouldBreak:
            print __name__, "Big Old Direction Vector:", xi[ibig]
            print __name__, "New Direction Vector:", xit
        storeDirectionVectors(xi)
                
        if (shouldBreak and 2.0*abs(fp - fret) <= ftol*(abs(fp)+abs(fret))):
            if printmessg:
                print "Optimisation successful"
            warnflag = False
            break



    if printmessg:
        print "         Current function value: %f" % fret
        print "         Iterations: %d" % iter
        print "         Function evaluations: %d" % func_calls
    try:
        apply(func, (None,)) #tell the function thread to stop
    except TypeError:
        print "Function not support None argument"
        
    if fulloutput:
        return p, fret, func_calls, warnflag
    else:        
        return p
    

if __name__ == "__main__":
    import string
    import time
    
    times = []
    algor = []
    x0 = [0.8,1.2,0.7]
    
##     start = time.time()
##     x = optimize.fmin(optimize.rosen,x0)
##     print x
##     times.append(time.time() - start)
##     algor.append('Nelder-Mead Simplex\t')
    
    start = time.time()
    x = fminPowellNew(optimize.rosen,x0)
    print x
    times.append(time.time() - start)
    algor.append('Powells Method\t')

##     start = time.time()
##     x = optimize.fminBFGS(optimize.rosen, x0, avegtol=1e-4, maxiter=100)
##     print x
##     times.append(time.time() - start)
##     algor.append('BFGS without gradient\t')

    print "\nMinimizing the Rosenbrock function of order 3\n"
    print " Algorithm \t\t\t       Seconds"
    print "===========\t\t\t      ========="
    for k in range(len(algor)):
        print algor[k], "\t\t -- ", times[k]
