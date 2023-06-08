#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <qrencode.h>
#include <jpeglib.h>
#include "base32.h"

/* Function Declaration */
void generateQRCode(const char *key);

/*********************************************************
 * generateQRCode:
 *
 *  This function will utilize the jpeglib and qrencode
 *  libraries to output a jpg image of an encoded string
 *  in QR code form.
 *********************************************************/
void generateQRCode(const char *key)
{
  /* Declare and populate our url string in the desired otp format */
  char url[256];
  snprintf(url, sizeof(url), "otpauth://totp/OTP:?secret=%s", key);
  
  /* Now we utilize the qrencode library to initially set up and populate a jpg image with our QR data */
  QRcode *qrCode = QRcode_encodeString(url, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
  if(qrCode != NULL)
  {
    FILE *jpg = fopen("qrcode.jpg", "wb");
    if(jpg != NULL)
    {
      struct jpeg_compress_struct info;
      struct jpeg_error_mgr err;

      /* Initialize our error field of our info struct and set up our jpg image for compression */
      info.err = jpeg_std_error(&err);
      jpeg_create_compress(&info);
      jpeg_stdio_dest(&info, jpg);
      
      /* Now we set the width and height of out image along with the requirements for RGB to be used in the image */
      info.image_width = qrCode->width;
      info.image_height = qrCode->width;
      info.input_components = 3;
      info.in_color_space = JCS_RGB;

      /* Now we set the default compression params for our info struct and set our quality to a rather decent 90 percent */
      jpeg_set_defaults(&info);
      jpeg_set_quality(&info, 90, TRUE);
      jpeg_start_compress(&info, TRUE);

      /* Now we declare our row pointer and our buffer to fit each RGB field */
      JSAMPROW rowPtr[1];
      uint8_t *jpgBuffer = (uint8_t *) calloc(qrCode->width * 3, sizeof(uint8_t));

      /* Now we actually write the data to our jpg image using the qrcode data */
      while(info.next_scanline < info.image_height)
      {
        for(int i = 0; i < qrCode->width; i++)
        {
          uint8_t pixel = qrCode->data[info.next_scanline * qrCode->width + i] & 0x01;
          jpgBuffer[i * 3] = jpgBuffer[i * 3 + 1] = jpgBuffer[i * 3 + 2] = pixel ? 255:0;
        }
        rowPtr[0] = jpgBuffer;
        jpeg_write_scanlines(&info, rowPtr, 1);
      }

      /* Now we clean up */
      jpeg_finish_compress(&info);
      jpeg_destroy_compress(&info);
      fclose(jpg);
      free(jpgBuffer);
    }
    QRcode_free(qrCode);
  }
}


int main(void)
{
  /* Declare and initialize our shared key and then generate the qr png */
  const char *key = "6gRQskb3j0ObS2tTvSgDRjOa2rr2l64xOdHqvYJWVP1mrlA9yasm6VwLChEM01AUa";
  unsigned char encodedKey[256];
  base32_encode((unsigned char *)key, strlen(key), (unsigned char *)encodedKey);
  generateQRCode((char *)encodedKey);
  return 0;
}
