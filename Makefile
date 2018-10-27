all : client.c aws.c monitor.c serverA.c serverB.c serverC.c
	gcc client.c -o client
	gcc aws.c -o aws
	gcc monitor.c -o monitor 
	gcc serverA.c -o serverA
	gcc serverB.c -o serverB
	gcc serverC.c -o serverC -lm

.PHONY: aws monitor serverA serverB serverC
aws : 
	./aws
monitor :
	./monitor
serverA :
	./serverA
serverB :
	./serverB
serverC :
	./serverC



