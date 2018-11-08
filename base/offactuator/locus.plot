set xlabel "x:forward"
set ylabel "z:left"
set zlabel "y:down"
#set xrange[-200:100]
#set yrange[-150:150]
#set zrange[-200:100]
set view 67,242
width = 134.4
length = 130
splot "locus.dat" using 1:3:(-$2) title "front left" with linespoints, \
      "locus.dat" using ($4):(-$6-width):(-$5) title "front right" with linespoints, \
      "locus.dat" using (-$7-length):($9):(-$8) title "back left" with linespoints, \
      "locus.dat" using (-$10-length):(-$12-width):(-$11) title "back right" with linespoints
#      "locus2.dat" using 1:3:(-$2) title "front left" with linespoints
#      "locus2.dat" using ($4):(-$6-width):(-$5) title "front right" with linespoints, \
#      "locus2.dat" using (-$7-length):9:(-$8) title "back left" with linespoints, \
#      "locus2.dat" using (-$10-length):(-$12-width):(-$11) title "back right" with linespoints
pause -1
