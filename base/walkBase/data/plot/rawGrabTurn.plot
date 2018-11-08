p(x) = a*x*x + b*x + c
l(x) = m*x + d
fit p(x) "grabTurn.dat" using 1:2 via a,b,c
fit l(x) "grabTurn.dat" using 1:2 via m,d
plot "grabTurn.dat" with yerrorbars, p(x), l(x)
pause -1
