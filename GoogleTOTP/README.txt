PREREQUISITES:
  -Install Kali Linux version 2023.1 from the Microsoft Store.
  -Install OpenSSL version 3.0.8.7 (Feb 2023) via sudo apt install openssl.
  -Install the qrencode library via sudo apt-get install libqrencode-dev.
  -Install the libjpeg library via sudo apt-get install libjpeg-dev.

TO RUN:
  -Make sure all files including the base32 files are in the same directory.
  -Run the command make all and run genQR to scan the QR code into Google's
   authenticator.
  -Delete the jpg that generates before each run through to minimize faulty QR codes
  -Run totp to make sure both codes match.
  -Run make clean to cleanup both executables.
