/*
 * Project: Image Steganography
 * Author: DAMARAJU PRUDHVIKRISHNA
 * Description: A command-line application to encode secret data into a BMP
 *              image and decode the hidden data from the encoded image.
 *              Supports encoding and decoding operations.
 */
#include <stdio.h>
#include "encode.h"
#include "decode.h"

/*
 * Function: main
 * Description: Entry point of the steganography program.
 *              Identifies the requested operation (encoding or decoding),
 *              validates command-line arguments, and performs the operation.
 * Input: argc - Number of command-line arguments
 *        argv - Array of command-line argument strings
 * Return: 0 on success, 1 on failure
 */

int main(int argc, char *argv[])
{
    // Structures to store encoding and decoding information
    EncodeInfo encodeInfo;
    DecodeInfo decodeInfo;

    // Check whether any command-line arguments were provided
    if (argc == 1)
    {
        printf("Error: No arguments passed\n");
        printf("Usage:\n");
        printf("Encoding: ./a.out -e <source.bmp> <secret.txt> [output.bmp]\n");
        printf("Decoding: ./a.out -d <source.bmp> [output.txt]\n");
        return 1;
    }

    // Identify the requested operation using the first argument
    int opr = check_operation(argv[1]);

    // Handle the encoding operation
    if (opr == encode)
    {
        // Encoding requires 3 or 4 arguments after the executable name
        if (argc < 4 || argc > 5)
        {
            printf("Error: Invalid number of arguments for encoding\n");
            return 1;
        }

        // Validate the input files and initialize encoding information
        if (validate_encode_args(argc, argv, &encodeInfo) == success)
        {
            printf("Validation successful\n");

            // Perform the encoding process
            if (do_encoding(argv, &encodeInfo))
            {
                printf("encoded successfully\n");
            }
            else
            {
                printf("encoding failed\n");
                return 1;
            }
        }
        else
        {
            printf("Error: Invalid encoding arguments\n");
            return 1;
        }
    }

    // Handle the decoding operation
    else if (opr == decode)
    {
        // Decoding requires a source BMP and an optional output filename
        if (argc < 3 || argc > 4)
        {
            printf("Error: Invalid number of arguments for decoding\n");
            printf("Usage: ./a.out -d <source.bmp> [output.txt]\n");
            return 1;
        }

        // Validate the input image and initialize decoding information
        if (valid_decode_args(argc, argv, &decodeInfo) == success)
        {
            printf("Validation successful\n");

            // Perform the decoding process
            if (do_decoding(&decodeInfo) == success)
            {
                printf("decoded successfully\n");
            }
            else
            {
                printf("decoding failed\n");
                return 1;
            }
        }
        else
        {
            printf("Error: Invalid decoding arguments\n");
            return 1;
        }
    }

    // Handle an unsupported operation
    else
    {
        printf("Error: Unsupported operation\n");
        return 1;
    }

    return 0;
}