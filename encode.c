#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include "encode.h"
#include "types.h"
#include "common.h" 

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if(argv[2] ==  NULL || argv[3] == NULL)
    {
        printf("Error : Insufficient arguments\n");
        return e_failure;
    }
    //argv[2] -> .bmp -> store argv[2] in structure encInfo->src_image_fname = argv[2];

    char *check_bmp = strstr(argv[2], ".bmp");

    if(check_bmp == NULL)
    {
        printf("Error : Invalid source file\n");
        return e_failure;
    }
    else
    {
        encInfo->src_image_fname= argv[2];
    }

    //argv[3] -> .txt -> encInfo->secret_fname = argv[3] -> strcpy(encInfo->extn_secret_file, strstr(argv[3], "."))

    encInfo->secret_fname = argv[3];

    char *extn = strstr(argv[3], ".");

    if(extn == NULL)
    {
        printf("Error : Invalid secret file\n");
        return e_failure;
    }

    //argv[4] -> NULL-> store "stego.bmp" in structure
        //   -> no NULL -> .bmp -> store argv[4] in structure

    strcpy(encInfo->extn_secret_file, extn);

    if(argv[4] == NULL)
    {
        encInfo->stego_image_fname = "stego.bmp";
    }
    else
    {
        if(strstr(argv[4], ".bmp")==NULL)
        {
            printf("Error : Invalid output file must be a .bmp file\n");
            return e_failure;
        }
        encInfo->stego_image_fname = argv[4];
    }

    return e_success;
}

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

//getting file size
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);

    return ftell(fptr);
}

//check capacity of  image
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    if((encInfo->image_capacity-54) > (strlen(MAGIC_STRING) + 
    sizeof(int) + strlen(encInfo->extn_secret_file) + sizeof(int) + 
    encInfo->size_secret_file )*8)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

//encoding data to image
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char arr[8];
    for(int i = 0; i < size; i++)
    {
        fread(arr, 8, 1, fptr_src_image);//reads 8 byte from source image
        encode_byte_to_lsb(data[i], arr);
        fwrite(arr, 8, 1, fptr_stego_image);//write 8 bytes to stego image
    }

    return e_success;
}

//encoding bytes to lsb
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i=0; i<8; i++)
    {
        //clear the lsb
        image_buffer[i] = image_buffer[i] & (~1);
        char get = (data & 1 << i) >> i;//getting bits
        image_buffer[i] = image_buffer[i] | get;//set the bits
    }
    return e_success;
}

//encoding integer to lsb
Status encode_integer_to_lsb(int data, char *image_buffer)
{
    for(int i=0; i<32; i++)
    {
        //clear the lsb
        image_buffer[i] = image_buffer[i] & (~1);
        char get = (data & 1 << i) >> i;//get the bits
        image_buffer[i] = image_buffer[i] | get;//set the bits
    }
    return e_success;
}

//copy the bmp header
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char arr[54];

    fread(arr, 54, 1, fptr_src_image);//reading bmp header
    fwrite(arr, 54, 1, fptr_dest_image);//writing bmp header

    return e_success;
}

//encoding magic string
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image(MAGIC_STRING, strlen(MAGIC_STRING), 
    encInfo->fptr_src_image, encInfo->fptr_stego_image);

    return e_success;
}

//encoding secret file extn size
Status encode_secret_file_extn_size(const char *file_extn, EncodeInfo *encInfo)
{
    char arr[32];
    int size = strlen(file_extn);
    fread(arr, 32, 1, encInfo->fptr_src_image);//reads 32 bytes from source image
    encode_integer_to_lsb(size, arr);
    fwrite(arr, 32, 1, encInfo->fptr_stego_image);//write 32 bytes to stego image

    return e_success;
}

//encoding secret fiel extn
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    return encode_data_to_image(file_extn, strlen(file_extn), 
    encInfo->fptr_src_image, encInfo->fptr_stego_image);
}

//encoding secret fiel size
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char arr[32];

    fread(arr, 32, 1, encInfo->fptr_src_image);//reads 32 bytes from source image
    encode_integer_to_lsb(file_size, arr);
    fwrite(arr, 32, 1, encInfo->fptr_stego_image);//writes 32 bytes into stego image

    return e_success;
}

//encoding secret file data
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    rewind(encInfo->fptr_secret);
    //memory allocation
    char *data_str = malloc(encInfo->size_secret_file);
    if(!data_str)
    {
        printf("Error : Memory allocation failed\n");
        return e_failure;
    }

    fread(data_str, encInfo->size_secret_file, 1, encInfo->fptr_secret);
    encode_data_to_image(data_str, encInfo->size_secret_file, encInfo->fptr_src_image, encInfo->fptr_stego_image);

    free(data_str);//free memory
    return e_success;
}

//copy remaining data
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char remaining_info;
    while(fread(&remaining_info, 1, 1, fptr_src)==1)
    {
        fwrite(&remaining_info, 1, 1, fptr_dest);
    }

    return e_success;
}


Status do_encoding(EncodeInfo *encInfo)
{
    printf("<---------------------------------------->\n");
    printf("           ENCODING PROCESS\n");
    printf("<---------------------------------------->\n");

    // open_files() == e_failure -> error -> return e_failure
    if(open_files(encInfo)==e_failure)
    {
        printf("Error : unable to open files\n");
        return e_failure;
    }

    //check_capacity() == e_failure -> error -> return e_failure

    if(check_capacity(encInfo)==e_failure)
    {
        printf("Error : insufficient image capacity\n");
        return e_failure;
    }

    //copy_bmp_header() == e_failure -> error -> return e_failure

    rewind(encInfo->fptr_src_image);

    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image)==e_failure)
    {
        printf("Error : copying is failed\n");
        return e_failure;
    }

    //encode_magic_string() == e_failure -> error -> return e_failure 
    
    if(encode_magic_string(MAGIC_STRING, encInfo)==e_failure)
    {
        printf("Error : encoding is failed\n");
        return e_failure;
    }
    printf("Magic String Encoded Successfully: %s\n", MAGIC_STRING);

    //encode_secret_file_extn_size == e_failure -> error -> return e_failure

    if(encode_secret_file_extn_size(encInfo->extn_secret_file, encInfo)==e_failure)
    {
        printf("Error : finding seceret file extension size is failed\n");
        return e_failure;
    }
    printf("Extension Size Encoded: %lu bytes\n", strlen(encInfo->extn_secret_file));

    //encode_secret_file_extn == e_failure -> error -> return e_failre

    if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo)==e_failure)
    {
        printf("Error : Encoding seceret file extension is failed\n");
        return e_failure;
    }
    printf("Extension Encoded: %s\n", encInfo->extn_secret_file);

    //encode_secret_file_size == e_failure -> error -> return e_fialure

    if(encode_secret_file_size(encInfo->size_secret_file, encInfo)==e_failure)
    {
        printf("Error : encoding secret file size is failed\n");
        return e_failure;
    }
    printf("Secret File Size Encoded: %ld bytes\n", encInfo->size_secret_file);

    //encode_secret_file == e_failure -> error -> return e_failure

    if(encode_secret_file_data(encInfo)==e_failure)
    {
        printf("Error : encoding secret file data is failed\n");
        return e_failure;
    }
    printf("Secret File Data Encoded Successfully..\n");

    //copy_remaining_img_data() == e_failure -> error -> return e_failure

    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image)==e_failure)
    {
        printf("Error : copying remainig image data is failed\n");
        return e_failure;
    }

    printf("Encoded successfully....\n");
    
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);
    return e_success;
}