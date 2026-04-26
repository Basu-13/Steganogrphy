#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h" 

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    //check stego image file
    if(argv[2] == NULL) 
    {
        printf("Error : stego image not provided\n");
        return e_failure;
    }
    
    if(strstr(argv[2], ".bmp") == NULL)//must be bmp file
    {
        printf("Error : stego image should be .bmp file\n");
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];

    //check output file
    if(argv[3] == NULL)
    {
        strcpy(decInfo->output_fname, "output");
    }
    else
    {
        strcpy(decInfo->output_fname, argv[3]);

        char *pos = strchr(decInfo->output_fname, '.');

        if(pos != NULL)
        {
            *pos = '\0';
        }
    }

    return e_success;
}

/* open files for decoding */
Status open_files_decode(DecodeInfo *decInfo)
{
    //stego image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");

    //do error handling
    if(decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "Error : Unable to open file %s\n", decInfo->stego_image_fname);

        return e_failure;
    }

    return e_success;
}

/*skip bmp header*/
Status skip_bmp_header(FILE *fptr_stego_image)
{
    //skip 54 byte bmp header
    if(fseek(fptr_stego_image, 54, SEEK_SET) != 0)
    {
        return e_failure;
    }

    return e_success;
}

/*decode magic string*/
Status decode_magic_string(FILE *fptr_stego_image)
{
    char magic_str[10], arr[8];

    int len = strlen(MAGIC_STRING);//len of magic string

    for(int i=0; i<len; i++)
    {
        //reading 8 byte from stego image
        if(fread(arr, 8, 1, fptr_stego_image) != 1)
        {
            return e_failure;
        }
        
        decode_bytes_from_lsb(&magic_str[i], arr);
    }

    magic_str[len] = '\0';//null termination

    //compare decoded magic string
    if(strcmp(magic_str, MAGIC_STRING) != 0)
    {
        return e_failure;
    }

    return e_success;
}

/* decoding extension size */
Status decode_extn_size(int *extn_size, FILE *fptr_stego_image)
{
    unsigned char byte;
    int size = 0;

    for(int i=0; i<32; i++)
    {
        //reading 1 byte from stego image 
        if(fread(&byte, 1, 1, fptr_stego_image) != 1)
        {
            return e_failure;
        }
        
        int bit = byte & 1;//extract lsb

        size = size | (bit << i);//reconstruct integer
    }

    //assigning decoded data to output
    *extn_size = size;

    return e_success;
}

/* decode bytes from lsb*/
Status decode_bytes_from_lsb(char *data, char *image_buffer)
{
    char ch = 0;

    for(int i=0; i<8; i++)
    {
        int bit = image_buffer[i] & 1;//Extract bit
        ch = ch | (bit << i);//build character
    }

    //assigning decoded data to output
    *data = ch;

    return e_success;
}

/* decode secret file extension */
Status decode_secret_file_extn(int extn_size, FILE *fptr_stego_image, DecodeInfo *decInfo)
{
    if(extn_size <= 0 || extn_size > 10)
    {
        printf("Error : Invalid extension size %d\n", extn_size);
        return e_failure;
    }
    
    char arr[8];

    for(int i=0; i<extn_size; i++)
    {
        //reading 8 bytes from stego image 
        if(fread(arr, 8, 1, fptr_stego_image) != 1)
        {
            return e_failure;
        }

        decode_bytes_from_lsb(&decInfo->extn_secret_file[i], arr);
    }

    decInfo->extn_secret_file[extn_size] = '\0'; //null terminate

    return e_success;
}

/* decode data size */
Status decode_data_size(int *size, FILE *fptr_stego_image)
{
    unsigned char byte;
    int data_size = 0;

    for(int i=0; i<32; i++)
    {
        //reading 1 byte from stego image file
        if(fread(&byte, 1, 1, fptr_stego_image) != 1)
        {
            return e_failure;
        }

        int bit = byte & 1;//extract the bit
        data_size = data_size | (bit << i); //reconstruct size
    }

    //assignig decode size to output file
    *size = data_size;

    return e_success;
}

/* decode secret file data */
Status decode_secert_file_data(FILE *fptr_stego_image, FILE *fptr_output, int size)
{
    unsigned char byte;
    unsigned char ch;

    for(int i=0; i<size; i++)//decode entire file
    {
        ch = 0;
        for(int j=0; j<8; j++)
        {
            //reading the 1 byte from stego image file
            if(fread(&byte, 1, 1, fptr_stego_image) != 1)
            {
                return e_failure;
            }

            int bit = byte & 1; //extract lsb
            ch = ch | (bit << j); //rebuild byte
        }

        fwrite(&ch, 1, 1, fptr_output); //write to output file
    }

    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    printf("<---------------------------------------->\n");
    printf("           DECODING PROCESS\n");
    printf("<---------------------------------------->\n");

    //open_files
    if(open_files_decode(decInfo) == e_failure)
    {
        printf("Error : unable to open files\n");
        return e_failure;
    }

    //skip bmp header
    if(skip_bmp_header(decInfo->fptr_stego_image) == e_failure)
    {
        printf("Error : unable to skip bmp header\n");
        return e_failure;
    }

    //magic string
    if(decode_magic_string(decInfo->fptr_stego_image) != e_success)
    {
        printf("Error : Magic string decoding failed\n");
        return e_failure;
    }
    printf("Magic String Decoded Successfully: %s\n", MAGIC_STRING);

    //decode extension size
    if(decode_extn_size(&decInfo->extn_size, decInfo->fptr_stego_image) != e_success)
    {
        printf("Error : Extension size decoding failed\n");
        return e_failure;
    }
    printf("Extension Size Decoded: %d bytes\n", decInfo->extn_size);

    //decode secret file extension
    if(decode_secret_file_extn(decInfo->extn_size, decInfo->fptr_stego_image, decInfo) != e_success)
    {
        printf("Error : Extension decoding failed\n");
        return e_failure;
    }
    printf("Extension Decoded: %s\n", decInfo->extn_secret_file);

    //create output file name
    strcat(decInfo->output_fname, decInfo->extn_secret_file);
    printf("Output File Created: %s\n", decInfo->output_fname);

    //open output file
    decInfo->fptr_output = fopen(decInfo->output_fname, "w");
    if(decInfo->fptr_output == NULL)
    {
        printf("Error : unable to open output files\n");
        return e_failure;
    }

    //decode data size
    if(decode_data_size(&decInfo->size_secret_file, decInfo->fptr_stego_image) != e_success)
    {
        printf("Error : Secret file size decoding failed\n");
        return e_failure;
    }
    printf("Secret File Size Decoded: %d bytes\n", decInfo->size_secret_file);

    //decode secret file data
    if(decode_secert_file_data(decInfo->fptr_stego_image, decInfo->fptr_output, decInfo->size_secret_file) != e_success)
    {
        printf("Error : Secret file data decoding failed\n");
        return e_failure;
    }
    printf("Secret File Data Decoded Successfully\n");

    //closing files
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);

    printf("Decoded successfully....\n");

    return e_success;
}