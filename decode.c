/*
 * File: decode.c
 * Description: Contains functions required to decode secret data from a
 *              BMP image using the Least Significant Bit (LSB) technique.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decode.h"
#include "common.h"
/*
 * Function: valid_decode_args
 * Description: Validates the stego image filename and stores the input
 *              image and optional output filename in decodeInfo.
 * Input: argc - Number of command-line arguments
 *        argv - Array containing command-line arguments
 *        decodeInfo - Pointer to decoding information
 * Return: success if the arguments are valid, otherwise failure
 */
Status valid_decode_args(int argc,char *argv[],DecodeInfo *decodeInfo)
{
    char *x=strrchr(argv[2],'.');
    if(x==NULL)
    return failure;
    if(strcmp(x,".bmp")!=0)
    {
        printf("not a valid extension\n");
        return failure;
    }
    decodeInfo->stego_image_fname=argv[2];
    if(argc==4)
        decodeInfo->output_fname=argv[3];
    else
        decodeInfo->output_fname=NULL;
    return success;
}

/*
 * Function: do_decoding
 * Description: Performs the complete decoding process by opening the
 *              stego image, decoding and validating the embedded metadata,
 *              creating the output file, and extracting the secret data.
 * Input: decInfo - Pointer to decoding information
 * Return: success if all decoding steps succeed, otherwise failure
 */
Status do_decoding(DecodeInfo *decInfo)
{
    if(open_decode_files(decInfo)==failure)
    return failure;
    if(decode_magic_string(decInfo)==failure)
    {
        fclose(decInfo->stego_image_fptr);
        return failure;
    }
    printf("Magic string decoded\n");
    if(decode_secret_file_extn_size(decInfo)==failure)
    {
        fclose(decInfo->stego_image_fptr);
        return failure;
    }
    printf("Extension size decoded: %d\n",decInfo->secret_extn_size);
    if(decode_secret_file_extn(decInfo)==failure)
    {
        fclose(decInfo->stego_image_fptr);
        return failure;
    }
    printf("Extension decoded: %s\n",decInfo->secret_extn);
    char filename[100];
    if(decInfo->output_fname==NULL)
    {
        printf("Enter output filename (without extension): ");
        scanf("%99s",filename);
    }
    else
    {
        if(strlen(decInfo->output_fname)>=sizeof(filename))
        {
            fclose(decInfo->stego_image_fptr);
            return failure;
        }
        strcpy(filename,decInfo->output_fname);
    }
    decInfo->output_fname=malloc(strlen(filename)+strlen(decInfo->secret_extn)+1);
    if(decInfo->output_fname==NULL)
    {
        fclose(decInfo->stego_image_fptr);
        return failure;
    }
    strcpy(decInfo->output_fname,filename);
    strcat(decInfo->output_fname,decInfo->secret_extn);
    decInfo->output_fptr=fopen(decInfo->output_fname,"wb");
    if(decInfo->output_fptr==NULL)
    {
        perror("fopen");
        free(decInfo->output_fname);
        decInfo->output_fname=NULL;
        fclose(decInfo->stego_image_fptr);
        return failure;
    }
    printf("Output file: %s\n",decInfo->output_fname);
    if(decode_secret_file_size(decInfo)==failure)
    {
        fclose(decInfo->stego_image_fptr);
        fclose(decInfo->output_fptr);
        free(decInfo->output_fname);
        decInfo->output_fname=NULL;
        return failure;
    }
    printf("Secret file size: %d\n",decInfo->secret_file_size);
    if(decode_secret_file_data(decInfo)==failure)
    {
        fclose(decInfo->stego_image_fptr);
        fclose(decInfo->output_fptr);
        free(decInfo->output_fname);
        decInfo->output_fname=NULL;
        return failure;
    }
    printf("Secret file data decoded\n");
    fclose(decInfo->stego_image_fptr);
    fclose(decInfo->output_fptr);
    free(decInfo->output_fname);
    decInfo->output_fname=NULL;
    return success;
}

/*
 * Function: decode_magic_string
 * Description: Reads the encoded magic string from the stego image and
 *              compares it with MAGIC_STRING to verify the image.
 * Input: decInfo - Pointer to decoding information
 * Return: success if the magic string matches, otherwise failure
 */
Status decode_magic_string(DecodeInfo *decInfo)
{
    fseek(decInfo->stego_image_fptr,54,SEEK_SET);
    char buffer[8];
    char data;
    int i=0;
    while(i<strlen(MAGIC_STRING))
    {
        if(fread(buffer,1,8,decInfo->stego_image_fptr)!=8)
        return failure;
        decode_1byte_from_lsb(buffer,&data);
        if(data!=MAGIC_STRING[i])
        return failure;
        i++;
    }
    return success;
}

/*
 * Function: decode_secret_file_extn_size
 * Description: Reads 32 image bytes and extracts the secret file
 *              extension size from their LSBs.
 * Input: decInfo - Pointer to decoding information
 * Return: success if the extension size is decoded, otherwise failure
 */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];
    if(fread(buffer,1,32,decInfo->stego_image_fptr)!=32)
    return failure;
    if(decode_4byte_from_lsb(buffer,&decInfo->secret_extn_size)==failure)
    return failure;
    return success;
}

/*
 * Function: decode_secret_file_extn
 * Description: Extracts the secret file extension from the LSBs of the
 *              stego image and adds a null terminator to the extension.
 * Input: decInfo - Pointer to decoding information
 * Return: success if the extension is decoded, otherwise failure
 */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    if(decInfo->secret_extn_size<1 || decInfo->secret_extn_size>=sizeof(decInfo->secret_extn))
    return failure;
    char buffer[8];
    int i=0;
    while(i<decInfo->secret_extn_size)
    {
        if(fread(buffer,1,8,decInfo->stego_image_fptr)!=8)
        return failure;
        if(decode_1byte_from_lsb(buffer,&decInfo->secret_extn[i])==failure)
        return failure;
        i++;
    }
    decInfo->secret_extn[i]='\0';
    return success;
}

/*
 * Function: decode_secret_file_size
 * Description: Reads 32 image bytes and extracts the original secret
 *              file size from their LSBs.
 * Input: decInfo - Pointer to decoding information
 * Return: success if the file size is decoded and non-negative,
 *         otherwise failure
 */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];
    if(fread(buffer,1,32,decInfo->stego_image_fptr)!=32)
    return failure;
    if(decode_4byte_from_lsb(buffer,&decInfo->secret_file_size)==failure)
    return failure;
    if(decInfo->secret_file_size<0)
    return failure;
    return success;
}

/*
 * Function: decode_secret_file_data
 * Description: Extracts each secret data byte from eight image bytes
 *              and writes it to the output file.
 * Input: decInfo - Pointer to decoding information
 * Return: success if all secret data is extracted, otherwise failure
 */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char data;
    int i=0;
    while(i<decInfo->secret_file_size)
    {
        char buffer[8];
        if(fread(buffer,1,8,decInfo->stego_image_fptr)!=8)
        return failure;
        if(decode_1byte_from_lsb(buffer,&data)==failure)
        return failure;
        if(fputc(data,decInfo->output_fptr)==EOF)
        return failure;
        i++;
    }
    return success;
}

/*
 * Function: decode_4byte_from_lsb
 * Description: Reconstructs a 4-byte integer from the LSBs of
 *              32 bytes stored in the buffer.
 * Input: buffer_32 - Array containing 32 encoded bytes
 *        data - Pointer to the integer where the decoded value is stored
 * Return: success after reconstructing the integer
 */
Status decode_4byte_from_lsb(char *buffer_32,int *data)
{
    *data=0;
    int i=0;
    while(i<32)
    {
        int bit=buffer_32[i]&1;
        *data=(*data)|(bit<<i);
        i++;
    }
    return success;
}

/*
 * Function: decode_1byte_from_lsb
 * Description: Reconstructs one data byte from the LSBs of eight
 *              bytes stored in the buffer.
 * Input: buffer_8 - Array containing eight encoded bytes
 *        data - Pointer to the variable where the decoded byte is stored
 * Return: success after reconstructing the byte
 */
Status decode_1byte_from_lsb(char *buffer_8,char *data)
{
    *data=0;
    int i=0;
    while(i<8)
    {
        int bit=buffer_8[i]&1;
        *data=*data|(bit<<i);
        i++;
    }
    return success;
}

/*
 * Function: open_decode_files
 * Description: Opens the stego image in binary read mode for decoding.
 * Input: decInfo - Pointer to decoding information
 * Return: success if the image opens successfully, otherwise failure
 */
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->stego_image_fptr=fopen(decInfo->stego_image_fname,"rb");
    if(decInfo->stego_image_fptr==NULL)
    {
        perror("fopen");
        return failure;
    }
    return success;
}
