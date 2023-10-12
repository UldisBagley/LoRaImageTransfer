// /*
//  * LoRaImageTransfer.c
//  * 
//  * Copyright 2023  <ubagley18@UldisRpi4BA>
//  * 
//  * This program is free software; you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as published by
//  * the Free Software Foundation; either version 2 of the License, or
//  * (at your option) any later version.
//  * 
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  * 
//  * You should have received a copy of the GNU General Public License
//  * along with this program; if not, write to the Free Software
//  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//  * MA 02110-1301, USA.
//  * 
//  * 
//  */

// #include <stdio.h>
// #include <pigpio.h>
// #include "LoRa.h"
// #include <stdio.h>
// #include <stdlib.h>

// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image/stb_image.h"
// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb_image/stb_image_write.h"

// void tx_f(txData *tx){
//     LoRa_ctl *modem = (LoRa_ctl *)(tx->userPtr);
//     printf("tx done;\t");
//     printf("sent string: \"%s\"\n\n", tx->buf);//Data we've sent
    
//     LoRa_receive(modem);
// }

// void * rx_f(void *p){
//     rxData *rx = (rxData *)p;
//     LoRa_ctl *modem = (LoRa_ctl *)(rx->userPtr);
//     LoRa_stop_receive(modem);//manually stoping RxCont mode
//     printf("rx done;\t");
//     printf("CRC error: %d;\t", rx->CRC);
//     printf("Data size: %d;\t", rx->size);
//     printf("received string: \"%s\";\t", rx->buf);//Data we've received
//     printf("RSSI: %d;\t", rx->RSSI);
//     printf("SNR: %f\n", rx->SNR);
    
//     LoRa_sleep(modem);
//     free(p);

//     return NULL;
// }



// int main(){

//     char txbuf[1000]; //255
//     char rxbuf[1000]; //255
//     LoRa_ctl modem;
// 	int width, height, channels;

//     //See for typedefs, enumerations and there values in LoRa.h header file
//     modem.spiCS = 0;//Raspberry SPI CE pin number
//     modem.tx.callback = tx_f;
//     modem.tx.data.buf = txbuf;
//     modem.rx.callback = rx_f;
//     modem.rx.data.userPtr = (void *)(&modem);//To handle with chip from rx callback
//     modem.tx.data.userPtr = (void *)(&modem);//To handle with chip from tx callback
//     //memcpy(modem.tx.data.buf, "Ping", 5);//copy data we'll sent to buffer
//     modem.tx.data.size = 0;//Payload len
//     modem.eth.preambleLen=6;
//     modem.eth.bw = BW62_5;//Bankdwidth 250kHz//BW62_5;//Bandwidth 62.5KHz
//     modem.eth.sf = SF12;//Spreading Factor 7//SF12;//Spreading Factor 12
//     modem.eth.ecr = CR8;//Error coding rate CR4/8
//     modem.eth.CRC = 1;//Turn on CRC checking
//     modem.eth.freq = 434800000;// 434.8MHz
//     modem.eth.resetGpioN = 4;//GPIO4 on lora RESET pin
//     modem.eth.dio0GpioN = 17;//GPIO17 on lora DIO0 pin to control Rxdone and Txdone interrupts
//     modem.eth.outPower = OP20;//Output power
//     modem.eth.powerOutPin = PA_BOOST;//Power Amplifire pin
//     modem.eth.AGC = 1;//Auto Gain Control
//     modem.eth.OCP = 240;// 45 to 240 mA. 0 to turn off protection
//     modem.eth.implicitHeader = 0;//1;//Implicit header mode//0;//Explicit header mode
//     modem.eth.syncWord = 0x12;
//     //For detail information about SF, Error Coding Rate, Explicit header, Bandwidth, AGC, Over current protection and other features refer to sx127x datasheet https://www.semtech.com/uploads/documents/DS_SX1276-7-8-9_W_APP_V5.pdf

//     unsigned char *img = stbi_load("/home/ubagley18/Documents/projects/LoRaImageTransfer/src/skyReallyReallySmall.jpg", &width, &height, &channels, 0);
// 	printf("Width: %d\n", width);
// 	printf("Height: %d\n", height);
// 	printf("Channels: %d\n", channels);
// 	uint8_t imageSize_Bytes = (uint8_t)sizeof(*img);
// 	printf("Image size in bytes: %d\n", imageSize_Bytes);

//     if(img == NULL) {
//         printf("Error in loading the image\n");
//         exit(1);
//     }
//     printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);

// 	memcpy(modem.tx.data.buf, img, sizeof(txbuf));//copy data we'll sent to buffer

//     LoRa_begin(&modem);
//     LoRa_send(&modem);
    
//     printf("Time on air data - Tsym: %f;\t", modem.tx.data.Tsym);
//     printf("Tpkt: %f;\t", modem.tx.data.Tpkt);
//     printf("payloadSymbNb: %u\n", modem.tx.data.payloadSymbNb);
    
//     while(LoRa_get_op_mode(&modem) != SLEEP_MODE){
//         sleep(1);
//     }

//     // Release memory
// 	stbi_image_free(img);

//     printf("end\n");

//     LoRa_end(&modem);
// }


