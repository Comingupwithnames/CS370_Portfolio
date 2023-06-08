all: weakResistance strongResistance

weakResistance: weakResistance.c
	gcc -std=c99 -g -o weakR weakResistance.c -lssl -lcrypto

strongResistance: strongResistance.c
	gcc -std=c99 -g -o strongR strongResistance.c -lssl -lcrypto

clean:
	rm weakR
	rm strongR
