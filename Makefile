all: scheduler.c child.c FIFO.c RR.c SJF.c PSJF.c
	gcc scheduler.c -o scheduler
	gcc child.c -o child
	gcc FIFO.c -o fifo
	gcc RR.c -o rr
	gcc SJF.c -o sjf
	gcc PSJF.c -o psjf

clean:
	rm -f scheduler child fifo rr sjf psjf
