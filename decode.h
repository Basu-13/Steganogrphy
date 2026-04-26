#ifndef DECODE_H
#define DECODE_H
 
#include "types.h" // Contains user defined types

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4 //for .txt

//structure to hold decoding information
typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Output file info */
    char output_fname[50];
    FILE *fptr_output;

    //magic string
    char magic_string[3];

    /* secret file extension */
    char extn_secret_file[20];
    int extn_size;

    //secret file size
    int size_secret_file;

    /* Buffers */
    //char image_data[MAX_IMAGE_BUF_SIZE];
    //char secret_data[MAX_SECRET_BUF_SIZE];

} DecodeInfo;

/* Decoding function prototype */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Open files for decoding */
Status open_files_decode(DecodeInfo *decInfo);

/* skip 54-byte bmp header */
Status skip_bmp_header(FILE *fptr_stego_image);

/* Decode magic string */
Status decode_magic_string(FILE *fptr_stego_image);

/* decode extension size */
Status decode_extn_size(int *extn_size, FILE *fptr_stego_image);

/* decode single bytes from lsb*/
Status decode_bytes_from_lsb(char *data, char *image_buffer);

/* decode secret file extension */
Status decode_secret_file_extn(int extn_size, FILE *fptr_stego_image, DecodeInfo *decInfo);

/* decode secret file size */
Status decode_data_size(int *size, FILE *fptr_stego_image);

/* decode secret file data */
Status decode_secert_file_data(FILE *fptr_stego_image, FILE *fptr_output, int size);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

#endif