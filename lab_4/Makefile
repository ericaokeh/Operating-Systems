# Target for the server
serv: serv.c list.c
	gcc serv.c list.c -lpthread -Wformat -Wall -o server

# Target for the client
# (FIXED: Added list.c to both lines)
cli: client.c list.c
	gcc client.c list.c -lpthread -Wformat -Wall -o client