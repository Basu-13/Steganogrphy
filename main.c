/*Name :- Basavaraj pagad
submission date :- 25/02/2026
description :- steganography is the technique of hiding secret information inside another
file like image, audio, video so that the presence of the message is not noticeable. it 
ensures secure communication by hiding data in a cover file without visibility.*/
#include <stdio.h>
#include <string.h>
#include "encode.h" 
#include "decode.h"
#include "types.h"
 
OperationType check_operation_type(char *argv[])
{
    //for encode
    if(strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    //for decode
    else if(strcmp(argv[1], "-d")==0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}

int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        printf("usage\n");
        printf("./a.out -e <src.bmp> <secret.txt> [stego.bmp]\n");
        printf("./a.out -d <stego.bmp> [output]\n");
        return 0;
    }

    //res = check_operation_type(argv);
    OperationType res = check_operation_type(argv);

    switch(res)
    {
        //res -> e_encode -> read_and_validate_encodde_arg(); -> do_encoding();
        
        case e_encode:
        {
            EncodeInfo encInfo;

            if(argc <= 3)
            {
                printf("Error : Invalid arguments\n");
                return 0;
            }
            
            if(read_and_validate_encode_args(argv, &encInfo) == e_success)
            {
                do_encoding(&encInfo);
            }
            else
            {
                printf("Invalid encode arguments\n");
            }

            break;
        }

        //res -> e_decode -> read_and_validate_decode_arg();  -> do_decoding();

        case e_decode:
        {
            DecodeInfo decInfo;
            
            if(read_and_validate_decode_args(argv, &decInfo) == e_success)
            {
                do_decoding(&decInfo);
            }
            else
            {
                printf("Invalid encode arguments\n");
            }
            break;
        }

        //res -> e_usupported -> error -> terminate

        default:
            printf("Unsupported operation use -e for encode and -d for decode.\n");
            break;
    }
    return 0;
}
