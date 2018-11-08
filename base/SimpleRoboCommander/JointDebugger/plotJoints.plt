set terminal png
set output 'joints.png'
plot "plot.dat" using 1:2 , "plot.dat" using 1:3 
set output
