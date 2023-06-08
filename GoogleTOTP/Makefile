all: totp qrEncode

totp: totp.c base32.c
	gcc -std=c99 -g -o totp totp.c base32.c -lm -lssl -lcrypto

qrEncode: genQR.c
	gcc -std=c99 -g -o genQR genQR.c base32.c -lqrencode -ljpeg

clean:
	rm totp
	rm genQR
