compile:
	gcc -o client -ansi -pedantic -Wall -std=c17 client.c client_modules/config.c client_modules/headers/config.h client_modules/conexions.c client_modules/headers/conexions.h client_modules/terminal.c client_modules/headers/terminal.h client_modules/alive.c client_modules/headers/alive.h client_modules/pdu.c client_modules/headers/pdu.h client_modules/register.c client_modules/headers/register.h client_modules/sendrecive.c client_modules/headers/sendrecive.h client_modules/socket.c client_modules/headers/socket.h client_modules/headers/globals.h client_modules/globals.c

test_all_clients: compile
	echo "TEST amb tots els clients"
	./client &
	./client -c client1.cfg &
	./client -c client2.cfg &
	./client -c client3.cfg &
	./client -c client4.cfg &
	./server.py
