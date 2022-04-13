compile:
	gcc -o client -ansi -pedantic -Wall -std=c17 client.c client_modules/config.c client_modules/conexions.c client_modules/terminal.c client_modules/alive.c client_modules/pdu.c  client_modules/register.c  client_modules/sendrecive.c client_modules/socket.c client_modules/globals.c

test_all_clients: compile
	echo "TEST amb tots els clients"
	./client &
	./client -c client1.cfg &
	./client -c client2.cfg &
	./client -c client3.cfg &
	./client -c client4.cfg &
	./server.py
	wait
	wait
	wait
	wait
	wait
	wait