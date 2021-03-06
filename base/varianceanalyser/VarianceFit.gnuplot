# A script to calculate beacon distance parameters from measured data in 
# Gnuplot.
# The data should be present in beacon_dist.dat with first column height in
# pixels, second column distance in cm
# The parameters a and b become beaconDistanceConstant and beaconDistanceOffset
# in VisualCortex.cc

#f1(x) = bconst/(x-boff)
#f2(x) = gconst/(x-goff)
#f3(x) = ballconst/(x-balloff)
#bconst = 
#boff = -10
#gconst = 1000
#goff = -10
#ballconst = 600
balloff = -9
#fit f1(x) "beacon_dist.dat" using 1:2 via bconst, boff
#fit f2(x) "goal_dist.dat" using 1:2 via gconst, goff
#fit f3(x) "ball_dist.dat" using 1:2 via ballconst, balloff

set xlabel "Distance"
set ylabel "Variance"
plot "ballDistVars.dat","beaconDistVars.dat","goalDistVars.dat"
