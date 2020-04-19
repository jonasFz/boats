boats: *.c
	gcc *.c -o boats -I /usr/include/X11 -L /usr/X11/lib -l X11
