/*
 * LoRaImageTransfer.c
 * 
 * Copyright 2023  <ubagley18@UldisRpi4BA>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <stdio.h>
#include <pigpio.h>
#include "LoRa.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

/**
 * @brief Funciton to return the size of a file in bytes
 * @param fp a pointer to the file
 * @returns the size of the file in bytes
*/
int fileSize(FILE *fp);

/**
 * @brief Counts the number of files in a directory (hard coded)
 * @returns the number of files in the directory
*/
int nbOfFilesInDirectory(void);

/**
 * @brief Reads the jpg file into the specified buffer 
 * @param jpgBuffer a pointer to the buffer that will send data over LoRa tx
 * @param filePath a pointer to the path to the file
 * @param fileSize a pointer to the size of the file
*/
void readJPGFileToBuffer(unsigned char** jpgBuffer, const char* filePath, int* fileSize);

/**
 * @brief Writes the jpg file into the specified path from a buffer 
 * @param jpgBuffer a pointer to the buffer that contains the jpg
 * @param filePath a pointer to the path to the file
 * @param fileSize a pointer to the size of the file
*/
void writeFileToPath(unsigned char* jpgBuffer, const char* newFilePath, int fileSize);


int sizeOfFile_Bytes;
bool transferSuccessFlag;
static int extractedInt;

void tx_f(txData *tx){
    LoRa_ctl *modem = (LoRa_ctl *)(tx->userPtr);
    printf("tx done;\t");
    printf("sent string: \"%02x\"\n\n", *tx->buf);//Data we've sent

    // Print each byte of the buffer as a hexadecimal value
    for (int i = 0; i < sizeOfFile_Bytes; i++) {
        printf("%02x ", tx->buf[i]);
    }

    printf("\n\n");
    
    LoRa_receive(modem);
}

void * rx_f(void *p){
    rxData *rx = (rxData *)p;
    LoRa_ctl *modem = (LoRa_ctl *)(rx->userPtr);
    LoRa_stop_receive(modem);//manually stoping RxCont mode
    printf("rx done;\t");
    printf("CRC error: %d;\t", rx->CRC);
    printf("Data size: %d;\t", rx->size);
    printf("received string: \"%s\";\t", rx->buf);//Data we've received
    printf("RSSI: %d;\t", rx->RSSI);
    printf("SNR: %f\n", rx->SNR);

    
    if (rx->size >= sizeof(int)) {
        extractedInt = atoi(rx->buf); //convert string to int
        // 'extractedInt' now contains the integer value.
    } else {
        // Handle the case where the buffer doesn't contain enough data for an int.
        printf("Not enough buffer size for int");
    }

    printf("Extracted int i.e. the number of received files: %d\n", extractedInt);

    // Set successful transfer flag:
    transferSuccessFlag = true;
    
    LoRa_sleep(modem);
    free(p);

    return NULL;
}

int main(){

    char txbuf[255]; //255
    char rxbuf[255]; //255
    LoRa_ctl modem;
	int width, height, channels;

    transferSuccessFlag = false;

    //unsigned char *img2;
    //int* sizeOffile = malloc(sizeof(int));;

    //*sizeOffile = 100;

    //See for typedefs, enumerations and there values in LoRa.h header file
    modem.spiCS = 0;//Raspberry SPI CE pin number
    modem.tx.callback = tx_f;
    modem.tx.data.buf = txbuf;
    modem.rx.callback = rx_f;
    modem.rx.data.userPtr = (void *)(&modem);//To handle with chip from rx callback
    modem.tx.data.userPtr = (void *)(&modem);//To handle with chip from tx callback
    //memcpy(modem.tx.data.buf, "Ping", 5);//copy data we'll sent to buffer
    modem.eth.preambleLen=6;
    modem.eth.bw = BW250;//Bankdwidth 250kHz//BW62_5;//Bandwidth 62.5KHz
    modem.eth.sf = SF7;//Spreading Factor 7//SF12;//Spreading Factor 12
    modem.eth.ecr = CR5;//Error coding rate CR4/8 //CR8
    modem.eth.CRC = 0;//Turn off CRC checking
    modem.eth.freq = 434800000;// 434.8MHz
    modem.eth.resetGpioN = 4;//GPIO4 on lora RESET pin
    modem.eth.dio0GpioN = 17;//GPIO17 on lora DIO0 pin to control Rxdone and Txdone interrupts
    modem.eth.outPower = OP20;//Output power
    modem.eth.powerOutPin = PA_BOOST;//Power Amplifire pin
    modem.eth.AGC = 1;//Auto Gain Control
    modem.eth.OCP = 240;// 45 to 240 mA. 0 to turn off protection
    modem.eth.implicitHeader = 0;//1;//Implicit header mode//0;//Explicit header mode
    modem.eth.syncWord = 0x12;
    //For detail information about SF, Error Coding Rate, Explicit header, Bandwidth, AGC, Over current protection and other features refer to sx127x datasheet https://www.semtech.com/uploads/documents/DS_SX1276-7-8-9_W_APP_V5.pdf

    // Count the number of files in the directory
    int nbOfFiles;
    nbOfFiles = nbOfFilesInDirectory();

    printf("Number of files in the directory: %d\n",nbOfFiles);
    // Wait for file nb response


    // printf("rx callback test 1\n");
    
    // //img2 = modem.tx.data.buf;//stbi_load(modem.tx.data.buf, width2, height2, channels2, 0);
    // unsigned char *img2 = stbi_load("/home/ubagley18/Documents/projects/LoRaImageTransfer/src/skyReallyReallySmallGray.jpg", width2, height2, channels2, 0);
    // // Print each byte of the buffer as a hexadecimal value
    // for (int i = 0; i < (256*2); i++) {
    //     printf("%02x ", img2[i]);
    // }
    

    // printf("Read jpg into buffer\n");
    // const char* filePathTest;
    // filePathTest = "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/skyReallySmall.jpg";
    // printf("%s\n", filePathTest);
    // readJPGFileToBuffer(img2, filePathTest, sizeOffile);

    // printf("Write jpg into directory\n");
    // const char* filePathTest2;
    // filePathTest2 = "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/copiedImage.jpg";
    // writeFileToDisk(img2, filePathTest2, sizeOffile);
    printf("Test1\n");
    unsigned char* img2 = NULL;
    int sizeOfFile2 = 0;
    const char* filePathTest = "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/skyReallySmall.jpg";

    readJPGFileToBuffer(&img2, filePathTest, &sizeOfFile2);
    // Check for errors and handle them.

    const char* filePathTest2 = "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/copiedImage.jpg";
    writeFileToPath(img2, filePathTest2, sizeOfFile2);
    // Check for errors and handle them.


    
    //stbi_write_jpg("/home/ubagley18/Documents/projects/LoRaImageTransfer/src/copy.jpg", *width2, *height2, *channels2, img2, 100);
    char fileName[100]; //100 characters allowed in a filename string
    unsigned char* img = NULL;
    int sizeOfFile = 0;
    char* filePath;
    int nbOfFilesSent = 1;

    while(1)
    {
        

        readJPGFileToBuffer(&img, filePath, &sizeOfFile);
        // Check for errors and handle them.

        FILE* imageFile;
        static int i = 0;
        // if (nbOfFilesSent < nbOfFiles)
        // {
        //     sprintf(filePath, "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/testSplit/image_%04d.jpg", nbOfFilesSent - 1);
        //     sprintf(fileName, "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/testSplit/image_%04d.jpg", nbOfFilesSent - 1);
        //     printf("%s\n", filePath);
        //     nbOfFilesSent++;
        // }
        if (nbOfFilesSent < nbOfFiles)
        {
            //Avoiding nonsensical segmentation fault
            nbOfFilesSent++;
            nbOfFilesSent--;

            printf("Test6\n");
            sprintf(filePath, "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/imageSplit/imageBit_%04d.jpg", nbOfFilesSent - 1);
            printf("Test7\n");
            sprintf(fileName, "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/imageSplit/imageBit_%04d.jpg", nbOfFilesSent - 1);
            printf("%s\n", filePath);
        }
        

        imageFile = fopen(fileName, "r");

        sizeOfFile_Bytes = fileSize(imageFile);

        printf("Size of file in bytes: %d\n",sizeOfFile_Bytes);

        modem.tx.data.size = sizeOfFile_Bytes;//Payload len

        readJPGFileToBuffer(&img, filePath, &sizeOfFile);

        if(img == NULL) {
            printf("Error in loading the image\n");
            exit(1);
        }

        memcpy(modem.tx.data.buf, img, modem.tx.data.size); //sizeof(txbuf));//copy data we'll send to buffer

        printf("image string: \"%02x\"\n\n", *modem.tx.data.buf);

        LoRa_begin(&modem);
        LoRa_send(&modem);
        
        printf("Time on air data - Tsym: %f;\t", modem.tx.data.Tsym);
        printf("Tpkt: %f;\t", modem.tx.data.Tpkt);
        printf("payloadSymbNb: %u\n", modem.tx.data.payloadSymbNb);
        
        int sleepTimeLeft = 0;
        while((LoRa_get_op_mode(&modem) != SLEEP_MODE) && (nbOfFilesSent != extractedInt)){
            printf("Number of files sent: %d\n", nbOfFilesSent);
            sleepTimeLeft = sleep(1);
            printf("Senconds of sleeptime left %d seconds\n", sleepTimeLeft);

            // If files sent does not match the number of files returned then send again
            if ((nbOfFilesSent > extractedInt))// && (firstTime != true))
            {
                    LoRa_end(&modem);
                    LoRa_begin(&modem);
                    LoRa_send(&modem);
                    printf("resend string: \"%02x\"\n\n", *modem.tx.data.buf);
            }
        }

        nbOfFilesSent = extractedInt; //Set files sent to equal number received so that above condition works next time

        // while(LoRa_get_op_mode(&modem) != SLEEP_MODE){
        //     sleep(1);
        // }

        // while (transferSuccessFlag != true)
        // {
        //     printf("Sending again\n");
        //     LoRa_begin(&modem);
        //     LoRa_send(&modem);
        //     sleep(1);
        // }
        // Release memory
        //stbi_image_free(img);
        transferSuccessFlag = false;
        LoRa_end(&modem);

        if (nbOfFilesSent == nbOfFiles) break;

        nbOfFilesSent++;
        
    }
}
// int main(){

//     char txbuf[255]; //255
//     char rxbuf[255]; //255
//     LoRa_ctl modem;
// 	int width, height, channels;

//     transferSuccessFlag = false;

//     //unsigned char *img2;
//     //int* sizeOffile = malloc(sizeof(int));;

//     //*sizeOffile = 100;

//     //See for typedefs, enumerations and there values in LoRa.h header file
//     modem.spiCS = 0;//Raspberry SPI CE pin number
//     modem.tx.callback = tx_f;
//     modem.tx.data.buf = txbuf;
//     modem.rx.callback = rx_f;
//     modem.rx.data.userPtr = (void *)(&modem);//To handle with chip from rx callback
//     modem.tx.data.userPtr = (void *)(&modem);//To handle with chip from tx callback
//     //memcpy(modem.tx.data.buf, "Ping", 5);//copy data we'll sent to buffer
//     modem.eth.preambleLen=6;
//     modem.eth.bw = BW250;//Bankdwidth 250kHz//BW62_5;//Bandwidth 62.5KHz
//     modem.eth.sf = SF12;//Spreading Factor 7//SF12;//Spreading Factor 12
//     modem.eth.ecr = CR5;//Error coding rate CR4/8 //CR8
//     modem.eth.CRC = 0;//Turn off CRC checking
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

//     // Count the number of files in the directory
//     int nbOfFiles;
//     nbOfFiles = nbOfFilesInDirectory();

//     printf("Number of files in the directory: %d\n",nbOfFiles);
//     // Wait for file nb response


//     // printf("rx callback test 1\n");
    
//     // //img2 = modem.tx.data.buf;//stbi_load(modem.tx.data.buf, width2, height2, channels2, 0);
//     // unsigned char *img2 = stbi_load("/home/ubagley18/Documents/projects/LoRaImageTransfer/src/skyReallyReallySmallGray.jpg", width2, height2, channels2, 0);
//     // // Print each byte of the buffer as a hexadecimal value
//     // for (int i = 0; i < (256*2); i++) {
//     //     printf("%02x ", img2[i]);
//     // }
    

//     // printf("Read jpg into buffer\n");
//     // const char* filePathTest;
//     // filePathTest = "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/skyReallySmall.jpg";
//     // printf("%s\n", filePathTest);
//     // readJPGFileToBuffer(img2, filePathTest, sizeOffile);

//     // printf("Write jpg into directory\n");
//     // const char* filePathTest2;
//     // filePathTest2 = "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/copiedImage.jpg";
//     // writeFileToDisk(img2, filePathTest2, sizeOffile);
//     // printf("Test1\n");
//     // unsigned char* img2 = NULL;
//     // int sizeOfFile2 = 0;
//     // const char* filePathTest = "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/skyReallySmall.jpg";

//     // printf("Test2\n");
//     // readJPGFileToBuffer(&img2, filePathTest, &sizeOfFile2);
//     // // Check for errors and handle them.

//     // const char* filePathTest2 = "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/copiedImage.jpg";
//     // writeFileToPath(img2, filePathTest2, sizeOfFile2);
//     // // Check for errors and handle them.

//     // printf("Test3\n");
    
//     //stbi_write_jpg("/home/ubagley18/Documents/projects/LoRaImageTransfer/src/copy.jpg", *width2, *height2, *channels2, img2, 100);

//     char fileName[100]; //120 characters allowed in a filename string
//     unsigned char* img = NULL;
//     int sizeOfFile = 0;
//     char* filePath;
//     int nbOfFilesSent = 1;

//     printf("Test4\n");

//     while(1)
//     {
        

//         //readJPGFileToBuffer(&img, filePath, &sizeOfFile);
        
//         printf("Test5\n");

//         FILE* imageFile;
//         static int i = 0;
//         if (nbOfFilesSent < nbOfFiles)
//         {
//             sprintf(filePath, "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/testSplit/image_%04d.jpg", nbOfFilesSent - 1);
//             sprintf(fileName, "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/testSplit/image_%04d.jpg", nbOfFilesSent - 1);
//             printf("%s\n", filePath);
//             nbOfFilesSent++;
//         }
//         if (nbOfFilesSent < nbOfFiles)
//         {
//             //Avoiding nonsensical segmentation fault
//             nbOfFilesSent++;
//             nbOfFilesSent--;

//             printf("Test6\n");
//             sprintf(filePath, "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/imageSplit/imageBit_%04d.jpg", nbOfFilesSent - 1);
//             printf("Test7\n");
//             sprintf(fileName, "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/imageSplit/imageBit_%04d.jpg", nbOfFilesSent - 1);
//             printf("%s\n", filePath);
//         }

//         imageFile = fopen(fileName, "r");

//         sizeOfFile_Bytes = fileSize(imageFile);

//         printf("Size of file in bytes: %d\n",sizeOfFile_Bytes);

//         modem.tx.data.size = sizeOfFile_Bytes;//Payload len

//         readJPGFileToBuffer(&img, filePath, &sizeOfFile);

//         if(img == NULL) {
//             printf("Error in loading the image\n");
//             exit(1);
//         }

//         memcpy(modem.tx.data.buf, img, modem.tx.data.size); //sizeof(txbuf));//copy data we'll send to buffer

//         printf("image string: \"%02x\"\n\n", *modem.tx.data.buf);

//         LoRa_begin(&modem);
//         LoRa_send(&modem);
        
//         printf("Time on air data - Tsym: %f;\t", modem.tx.data.Tsym);
//         printf("Tpkt: %f;\t", modem.tx.data.Tpkt);
//         printf("payloadSymbNb: %u\n", modem.tx.data.payloadSymbNb);
        
//         int sleepTimeLeft = 0;
//         bool firstTime = true;

//         while((LoRa_get_op_mode(&modem) != SLEEP_MODE) && (nbOfFilesSent != extractedInt)){
//             printf("Number of files sent: %d\n", nbOfFilesSent);
//             sleepTimeLeft = sleep(5);
//             printf("Senconds of sleeptime left %d seconds\n", sleepTimeLeft);

//             // If files sent does not match the number of files returned then send again
//             if ((nbOfFilesSent != extractedInt) && (firstTime != true))
//             {
//                 LoRa_begin(&modem);
//                 LoRa_send(&modem);
//             }
//             firstTime = false;
//         }

        

//         // while (transferSuccessFlag != true)
//         // {
//         //     printf("Sending again\n");
//         //     LoRa_begin(&modem);
//         //     LoRa_send(&modem);
//         //     sleep(1);
//         // }
//         // Release memory
//         //stbi_image_free(img);
//         nbOfFilesSent++;
//         transferSuccessFlag = false;
//         LoRa_end(&modem);
//     }
// }

int fileSize(FILE *fp)
{
    int prev=ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int size=ftell(fp);
    fseek(fp,prev,SEEK_SET); //go back to where we were
    return size;
}

int nbOfFilesInDirectory(void)
{
    int file_count = 0;
    DIR * dirp;
    struct dirent * entry;

    dirp = opendir("/home/ubagley18/Documents/projects/LoRaImageTransfer/src/imageSplit"); /* There should be error handling after this */
    if (dirp == NULL)
    {
        printf("Error opening directory");
    }
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG) { /* If the entry is a regular file */
            file_count++;
        }
    }
    closedir(dirp);

    return file_count;
}

void readJPGFileToBuffer(unsigned char** jpgBuffer, const char* filePath, int* fileSize)
{
    FILE* jpgFile = fopen(filePath, "rb");
    if (jpgFile == NULL) {
        // Handle file opening error.
        printf("JPG is NULL");
        return;
    }

    fseek(jpgFile, 0, SEEK_END);
    *fileSize = ftell(jpgFile);
    rewind(jpgFile);

    *jpgBuffer = (unsigned char*)malloc(*fileSize);
    if (*jpgBuffer == NULL) {
        // Handle memory allocation error.
        printf("JPG buffer is NULL");
        fclose(jpgFile);
        return;
    }

    fread(*jpgBuffer, 1, *fileSize, jpgFile);
    fclose(jpgFile);
}

void writeFileToPath(unsigned char* jpgBuffer, const char* newFilePath, int fileSize)
{
    FILE* receivedJpgFile = fopen(newFilePath, "wb");
    if (receivedJpgFile == NULL) {
        // Handle file creation error.
        printf("Received JPG is NULL");
        return;
    }

    fwrite(jpgBuffer, 1, fileSize, receivedJpgFile);
    fclose(receivedJpgFile);
}






//START IMAGE LOAD TEST

    // int* width2 = malloc(sizeof(int));
    // int* height2 = malloc(sizeof(int));
    // int* channels2 = malloc(sizeof(int));

    // *width2 = 4;
    // *height2 = 3;
    // *channels2 = 1;

    

    // unsigned char *img2;

    // printf("rx callback test 1\n");
    
    // img2 = modem.tx.data.buf;//stbi_load(modem.tx.data.buf, width2, height2, channels2, 0);

    // printf("Image loaded from buffer\n");

    // if(img == NULL) {
    //     printf("Error in loading the image\n");
    //     exit(1);
    // }
    // printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", *width2, *height2, *channels2);

    // stbi_write_jpg("/home/ubagley18/Documents/projects/LoRaImageTransfer/src/copy.jpg", *width2, *height2, *channels2, img2, 100);


    // // Don't forget to free the dynamically allocated memory when you're done with it
    // stbi_image_free(img2);
    // free(width2);
    // free(height2);
    // free(channels2);

    // END IMAGE LOAD TEST






    // START IMAGE CREATE FROM PIXEL DATA TEST

    // // Print each byte of the buffer as a hexadecimal value
    // for (int i = 0; i < (256*2); i++) {
    //     printf("%02x ", img2[i]);
    // }

    // printf("\n\n");

    // Image* image;

    // printf("Trying to create image from binary data\n");
    // sprintf(fileName, "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/testSplit/BBBBB.jpg");
    // printf("%s\n", fileName);

    // uint8_t* data = malloc(10);

    // printf("Data Allocated Memory\n");

    // Image_create(image, *width2, *height2, *channels2, 0); //Create an "Image" structure

    // printf("Image Created\n");

    // for (int k = 0; k < (256*2); k++) 
    // {
    //     image->data[k] = img2[k];
    //     printf("%02x ", image->data[k]);
    // }

    // // if (memoryFile == NULL) {
    // //     perror("fmemopen error");
    // // }

    // printf("\n\n");
    // printf("Created image from binary data\n");

    // printf("Image loaded from buffer\n");

    // if(img2 == NULL) {
    //     printf("Error in loading the image\n");
    //     exit(1);
    // }

    // printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", *width2, *height2, *channels2);

    // Image_save(image, fileName); //Save this structure to a file and write jpg
    // //stbi_write_jpg("/home/ubagley18/Documents/projects/LoRaImageTransfer/src/copy.jpg", *width2, *height2, *channels2, img2, 100);


    // END IMAGE CREATE FROM PIXEL DATA TEST





    // START ORIGINAL "WORKING" WHILE LOOP
    // THIS SENDS PIXEL DATA WHICH GETS DIGESTED BY THE RECEIVE NODE
    
    // while(1)
    // {
    //     FILE* imageFile;
    //     static int i = 0;
    //     if (i < nbOfFiles)
    //     {
    //         sprintf(fileName, "/home/ubagley18/Documents/projects/LoRaImageTransfer/src/testSplit/image_%04d.jpg", i);
    //         printf("%s\n", fileName);
    //         i++;
    //     }

    //     imageFile = fopen(fileName, "r");

    //     sizeOfFile_Bytes = fileSize(imageFile);

    //     printf("Size of file in bytes: %d\n",sizeOfFile_Bytes);

    //     modem.tx.data.size = sizeOfFile_Bytes;//Payload len


    //     unsigned char *img = stbi_load(fileName, &width, &height, &channels, 0);
    //     printf("Width: %d\n", width);
    //     printf("Height: %d\n", height);
    //     printf("Channels: %d\n", channels);

    //     if(img == NULL) {
    //         printf("Error in loading the image\n");
    //         exit(1);
    //     }
    //     printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);

    //     memcpy(modem.tx.data.buf, img, modem.tx.data.size); //sizeof(txbuf));//copy data we'll send to buffer

    //     printf("image string: \"%02x\"\n\n", *modem.tx.data.buf);

    //     LoRa_begin(&modem);
    //     LoRa_send(&modem);
        
    //     printf("Time on air data - Tsym: %f;\t", modem.tx.data.Tsym);
    //     printf("Tpkt: %f;\t", modem.tx.data.Tpkt);
    //     printf("payloadSymbNb: %u\n", modem.tx.data.payloadSymbNb);
        
    //     while(LoRa_get_op_mode(&modem) != SLEEP_MODE){
    //         sleep(1);
    //     }

    //     // Release memory
    //     stbi_image_free(img);

    //     LoRa_end(&modem);
    // }

    // END ORIGINAL "WORKING" WHILE LOOP