#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "common.h"

/* Stores filenames, file pointers, and secret file details for decoding. */
typedef struct _DecodeInfo
{
    char *stego_image_fname;
    char *output_fname;

    FILE *stego_image_fptr;
    FILE *output_fptr;

    /* Stores the secret file extension and its size. */
    char secret_extn[7];
    int secret_extn_size;
    int secret_file_size;

} DecodeInfo;

/* Validates the command-line arguments for decoding. */
Status valid_decode_args(int argc, char *argv[], DecodeInfo *decodeInfo);

/* Coordinates the complete decoding process. */
Status do_decoding(DecodeInfo *decInfo);

/* Opens the stego image file for reading. */
Status open_decode_files(DecodeInfo *decInfo);

/* Extracts and verifies the magic string from the stego image. */
Status decode_magic_string(DecodeInfo *decInfo);

/* Extracts the secret file extension size. */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Extracts the secret file extension. */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Extracts the original secret file size. */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Extracts the secret file data and writes it to the output file. */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Reconstructs one byte from the LSBs of eight encoded bytes. */
Status decode_1byte_from_lsb(char *buffer_8, char *data);

/* Reconstructs an integer from the LSBs of 32 encoded bytes. */
Status decode_4byte_from_lsb(char *buffer_32, int *data);

#endif