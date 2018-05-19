program: trafperf.o gnuplot_i.o
	gcc -o traffic trafperf.o gnuplot_i.o -lpthread

trafperf.o: trafperf.c
	gcc -c trafperf.c

gnuplot_i.o: gnuplot_i.c
	gcc -c gnuplot_i.c
