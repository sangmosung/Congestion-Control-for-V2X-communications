set datafile separator ','

plot 'Book.csv' u 1:2 w 1p 1w 1.5 1c 7 title 'square fo numbers'

set terminal png
set output "CBR.png"
set title "2-D Plot"
set xlabel "TIME"
set ylabel "CBR"
set xrange [0:10]
set yrange [1:100]


