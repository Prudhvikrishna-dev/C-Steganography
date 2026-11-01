/*
 * File: encode.c
 * Description: Contains functions required to encode secret data into a
 *              BMP image using the Least Significant Bit (LSB) technique.
 */

#include <stdio.h>
#include "encode.h"
#include <string.h>
#include "common.h"
/*
 * Function: check_operation
 * Description: Identifies whether the user selected encoding or decoding.
 * Input: argv - Command-line argument specifying the operation
 * Return: encode for "-e", decode for "-d", otherwise unsupported
 */
Opr_type check_operation(char *argv)
{
    if(!strcmp(argv,"-e"))
        return encode;
    else if(!strcmp(argv,"-d"))
        return decode;
    else
        return unsupported;
}

/*
 * Function: validate_encode_args
 * Description: Validates the input filenames and stores the source image,
 *              secret file, and destination image filenames in encInfo.
 * Input: argc - Number of command-line arguments
 *        argv - Array containing command-line arguments
 *        encInfo - Pointer to encoding information
 * Return: success if the filenames are valid, otherwise failure
 */
Status validate_encode_args(int argc,char *argv[], EncodeInfo *encInfo)
{
    char *x=strrchr(argv[2],'.');
    if(x!=NULL && strcmp(x,".bmp")==0)
    {
        encInfo->src_image_fname = argv[2];
        char *y=strrchr(argv[3],'.');
        if(y!=NULL && y!=argv[3])
        {
            if(strlen(y)>=sizeof(encInfo->secret_extn))
                return failure;
            encInfo->secret_fname = argv[3];
            strcpy(encInfo->secret_extn,y);
            encInfo->secret_extn_size=strlen(y);
        }
        else
        {
            return failure;
        }

        if(argc == 4)
        {
            encInfo->output_image_fname = "stego.bmp";
        }
        else
        {
            char *z=strrchr(argv[4],'.');
            if(z!=NULL && strcmp(z,".bmp")==0)
                encInfo->output_image_fname = argv[4];
            else
                return failure;
        }
    }
    else
    {
        return failure;
    }
    return success;
}

/*
 * Function: do_encoding
 * Description: Performs the complete encoding process by opening the files,
 *              checking capacity, copying the BMP header, encoding the secret
 *              information, and copying the remaining image data.
 * Input: argv - Array containing command-line arguments
 *        encInfo - Pointer to encoding information
 * Return: success if all encoding steps succeed, otherwise failure
 */
Status do_encoding(char *argv[], EncodeInfo *encInfo)
{
    Status result=failure;
    encInfo->src_image_fptr=NULL;
    encInfo->secret_fptr=NULL;
    encInfo->output_image_fptr=NULL;
    if(!open_files(encInfo))
        goto cleanup;
    if(!check_capacity(encInfo))
        goto cleanup;
    if(copy_bmp_header(encInfo)==failure)
        goto cleanup;
    if(encode_magic_string(MAGIC_STRING,encInfo)==failure)
        goto cleanup;
    if(encode_secret_file_extn_size(encInfo->secret_extn_size,encInfo)==failure)
        goto cleanup;
    if(encode_secret_file_extn(encInfo)==failure)
        goto cleanup;
    if(encode_secret_file_size(encInfo)==failure)
        goto cleanup;
    if(encode_secret_file_data(encInfo)==failure)
        goto cleanup;
    if(copy_remaining_img_data(encInfo)==failure)
        goto cleanup;
    result=success;
    cleanup:
    if(encInfo->src_image_fptr!=NULL)
        fclose(encInfo->src_image_fptr);
    if(encInfo->secret_fptr!=NULL)
        fclose(encInfo->secret_fptr);
    if(encInfo->output_image_fptr!=NULL)
        fclose(encInfo->output_image_fptr);
    return result;
}

/*
 * Function: copy_bmp_header
 * Description: Copies the first 54 bytes of the source BMP image to the
 *              destination image without modifying them.
 * Input: encInfo - Pointer to encoding information
 * Return: success if the header is copied successfully, otherwise failure
 */
Status copy_bmp_header(EncodeInfo *encInfo)
{
    char ch;
    int c=0;
    fseek(encInfo->src_image_fptr,0,SEEK_SET);
    while(c<54)
    {
        if(fread(&ch,1,1,encInfo->src_image_fptr)!=1)
            return failure;
        if(fwrite(&ch,1,1,encInfo->output_image_fptr)!=1)
            return failure;
        c++;
    }
    return success;
}

/*
 * Function: encode_magic_string
 * Description: Encodes the magic string into the LSBs of the image data
 *              so the decoder can identify the encoded image.
 * Input: magic_string - String used to identify the encoded image
 *        encInfo - Pointer to encoding information
 * Return: success if the magic string is encoded, otherwise failure
 */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    fseek(encInfo->src_image_fptr,54,SEEK_SET);
    int i=0;
    while(magic_string[i]!='\0')
    {
        char buffer[8];
        if(fread(buffer,1,8,encInfo->src_image_fptr)!=8)
            return failure;
        encode_1byte_to_lsb(magic_string[i],buffer);
        if(fwrite(buffer,1,8,encInfo->output_image_fptr)!=8)
            return failure;
        i++;
    }
    return success;
}

/*
 * Function: encode_secret_file_extn_size
 * Description: Encodes the size of the secret file's extension into
 *              32 image bytes using their LSBs.
 * Input: data - Size of the secret file extension
 *        encInfo - Pointer to encoding information
 * Return: success if the extension size is encoded, otherwise failure
 */
Status encode_secret_file_extn_size(int data,EncodeInfo *encInfo)
{
    char buffer[32];
    if(fread(buffer,1,32,encInfo->src_image_fptr)!=32)
        return failure;
    encode_4byte_to_lsb(data,buffer);
    if(fwrite(buffer,1,32,encInfo->output_image_fptr)!=32)
        return failure;
    return success;
}

/*
 * Function: encode_secret_file_extn
 * Description: Encodes the secret file's extension into the LSBs of
 *              the image data.
 * Input: encInfo - Pointer to encoding information
 * Return: success if the extension is encoded, otherwise failure
 */
Status encode_secret_file_extn(EncodeInfo *encInfo)
{
    int i=0;
    while(encInfo->secret_extn[i]!='\0')
    {
        char buffer[8];
        if(fread(buffer,1,8,encInfo->src_image_fptr)!=8)
            return failure;
        encode_1byte_to_lsb(encInfo->secret_extn[i],buffer);
        if(fwrite(buffer,1,8,encInfo->output_image_fptr)!=8)
            return failure;
        i++;
    }
    return success;
}

/*
 * Function: encode_secret_file_size
 * Description: Encodes the size of the secret file into 32 image bytes
 *              using their LSBs.
 * Input: encInfo - Pointer to encoding information
 * Return: success if the secret file size is encoded, otherwise failure
 */
Status encode_secret_file_size(EncodeInfo *encInfo)
{
    char buffer[32];
    if(fread(buffer,1,32,encInfo->src_image_fptr)!=32)
        return failure;
    encode_4byte_to_lsb(encInfo->secret_file_size,buffer);
    if(fwrite(buffer,1,32,encInfo->output_image_fptr)!=32)
        return failure;
    return success;
}

/*
 * Function: encode_secret_file_data
 * Description: Reads each byte of the secret file and encodes it into
 *              the LSBs of eight image bytes.
 * Input: encInfo - Pointer to encoding information
 * Return: success if all secret data is encoded, otherwise failure
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    int i=0;
    while(i<encInfo->secret_file_size)
    {
        char buffer[8];
        char data;
        if(fread(&data,1,1,encInfo->secret_fptr)!=1)
            return failure;
        if(fread(buffer,1,8,encInfo->src_image_fptr)!=8)
            return failure;
        encode_1byte_to_lsb(data,buffer);
        if(fwrite(buffer,1,8,encInfo->output_image_fptr)!=8)
            return failure;
        i++;
    }
    return success;
}

/*
 * Function: encode_1byte_to_lsb
 * Description: Stores the eight bits of one data byte in the LSBs
 *              of eight image bytes.
 * Input: data - Byte to encode
 *        buffer_8 - Array containing eight image bytes
 * Return: success after encoding the byte
 */
Status encode_1byte_to_lsb(char data, char *buffer_8)
{
    int i=0;
    while(i<8)
    {
        int bit=(data>>i)&1;
        buffer_8[i]=(buffer_8[i]&~1)|(bit);
        i++;
    }
    return success;
}

/*
 * Function: encode_4byte_to_lsb
 * Description: Stores the 32 bits of an integer in the LSBs
 *              of 32 image bytes.
 * Input: data - Integer to encode
 *        buffer_32 - Array containing 32 image bytes
 * Return: success after encoding the integer
 */
Status encode_4byte_to_lsb(int data, char *buffer_32)
{
    int i=0;
    while(i<32)
    {
        int bit=(data>>i)&1;
        buffer_32[i]=(buffer_32[i]&~1)|(bit);
        i++;
    }
    return success;
}

/*
 * Function: copy_remaining_img_data
 * Description: Copies all remaining bytes from the source image to
 *              the destination image after the secret data is encoded.
 * Input: encInfo - Pointer to encoding information
 * Return: success if the remaining image data is copied, otherwise failure
 */
Status copy_remaining_img_data(EncodeInfo *encInfo)
{
    char ch;
    while(fread(&ch,1,1,encInfo->src_image_fptr)==1)
    {
        if(fwrite(&ch,1,1,encInfo->output_image_fptr)!=1)
            return failure;
    }
    return success;
}

/*
 * Function: check_capacity
 * Description: Calculates the image's available encoding capacity and
 *              the total capacity required for the secret information.
 * Input: encInfo - Pointer to encoding information
 * Return: success if the image has enough capacity, otherwise failure
 */
Status check_capacity(EncodeInfo *encInfo)
{
    unsigned int image_capacity=get_image_size_for_bmp(encInfo->src_image_fptr);
    encInfo->secret_file_size=get_file_size(encInfo->secret_fptr);
    unsigned int required_capacity=(strlen(MAGIC_STRING)+sizeof(int)+encInfo->secret_extn_size+sizeof(int)+encInfo->secret_file_size)*8;
    if(image_capacity>=required_capacity)
        return success;
    else
        return failure;
}

/*
 * Function: get_file_size
 * Description: Finds the size of a file by moving to its end, reading
 *              the position, and resetting the file pointer.
 * Input: fptr - Pointer to the file
 * Return: Size of the file in bytes
 */
unsigned int get_file_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_END);
    int size=ftell(fptr);
    rewind(fptr);
    return size;
}

/*
 * Function: get_image_size_for_bmp
 * Description: Reads the BMP image's width and height from its header
 *              and calculates the image capacity using three bytes per pixel
 *              with each row padded to a multiple of 4 bytes.
 * Input: fptr_image - Pointer to the BMP image file
 * Return: Estimated image capacity in bytes
 */
unsigned int get_image_size_for_bmp(FILE *fptr_image)
{
    unsigned int width,height;
    fseek(fptr_image,18,SEEK_SET);
    fread(&width,sizeof(int),1,fptr_image);
    fread(&height,sizeof(int),1,fptr_image);
    if((int)height<0)
        height=-height;
    return ((width*3+3)&~3u)*height;
}

/*
 * Function: open_files
 * Description: Opens the source BMP image, secret file, and destination
 *              BMP image for the encoding process.
 * Input: encInfo - Pointer to encoding information
 * Return: success if all files open successfully, otherwise failure
 */
Status open_files(EncodeInfo *encInfo)
{
    encInfo->src_image_fptr=fopen(encInfo->src_image_fname,"rb");
    if(encInfo->src_image_fptr==NULL)
    {
        perror("fopen");
        fprintf(stderr,"ERROR: Unable to open file %s\n",encInfo->src_image_fname);
        return failure;
    }

    encInfo->secret_fptr=fopen(encInfo->secret_fname,"rb");
    if(encInfo->secret_fptr==NULL)
    {
        perror("fopen");
        fprintf(stderr,"ERROR: Unable to open file %s\n",encInfo->secret_fname);
        return failure;
    }

    encInfo->output_image_fptr=fopen(encInfo->output_image_fname,"wb");
    if(encInfo->output_image_fptr==NULL)
    {
        perror("fopen");
        fprintf(stderr,"ERROR: Unable to open file %s\n",encInfo->output_image_fname);
        return failure;
    }

    return success;
}
