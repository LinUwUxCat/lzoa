/*  main.c
    this file contains the main function of lzox
    it uses the lzo library - for more info, please see http://www.oberhumer.com/opensource/lzo/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minilzo.h"

#define HEAP_ALLOC(var,size) \
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

int main(int argc, char *argv[]){

    int r; //result

    if (lzo_init() != LZO_E_OK) {
        printf("Error - lzo_init() failed. This typically indicates a compiler bug. Try compiling again.\n");
        return 2; //1 is a bug in the code, here it's a compiler bug. I don't know what to return, so have a 2.
    }
    
    if (argc < 3 || argv[1][0] != '-' || (argv[1][1] != 'c' && argv[1][1] != 'd' && argv[1][1] != 'p' && argv[1][1] != 'x')) {
        printf("lzox 0.0 - by LinuxCat\nUsing miniLZO library by Markus Franz Xaver Johannes Oberhumer\nUsage: %s <OPTION> <input file> [output file]\n\nOptions : \n\t-c\tCompress input file to output file\n\t-d\tDecompress input file to output file\n\nIf no output is specified, it will be FILE_NAME.lzo if compress, and stdout if decompress.", argv[0]); //congrats, you read to the end of this line! now go listen to this https://www.youtube.com/watch?v=9R80DUsixGg
        return 0; //actually i have no fucking idea what i'm supposed to return. Here's a 0
    }
    
    FILE *in = fopen(argv[2], "r"); //open the input file
    if (in == NULL) {
        printf("Error - Could not open input file.\n");
        return 1;
    }

    fseek(in, 0, SEEK_END);
    lzo_uint inlen = ftell(in);
    fseek(in, 0, SEEK_SET);

    //unsigned char __LZO_MMODEL out [ inlen + inlen / 16 + 64 + 3 ]; //from the miniLZO example -> we provide additional space in case data isn't compressible
                                                                      //TODO : Replace by a file.
    unsigned char __LZO_MMODEL *inbuf = (unsigned char *)malloc(inlen);
    fread(inbuf, 1, inlen, in);
    fclose(in);
    

    /****************/
    /*    Outfile   */
    /****************/
    lzo_uint outlen;
    FILE *out;
    if (argc < 4 || argv[3][0] == '-'){
        if (argv[1][1] == 'd'){
            out = stdout;
        } else {
            out = fopen(strcat(argv[2], ".lzo"), "w"); //the evkitpro incident
        }
    } else {
        out = fopen(argv[3], "w");
    }


    /***************/
    /* Compression */
    /***************/
    if (argv[1][1] == 'c'){
        unsigned char __LZO_MMODEL outbuf [ inlen + inlen / 16 + 64 + 3 ];
        r = lzo1x_1_compress(inbuf,inlen,outbuf,&outlen,wrkmem);

        if (r != LZO_E_OK){
            printf("Error - Compression failed! Please open an issue on github about this. Error code : COM-%d", r);
            //remove file
            return 1; //no clue
        }
        if (outlen >= inlen){
            printf("Note - Data is not compressible.");
        }
        fwrite("LZOX", 1, 4, out);
        fwrite(&inlen, sizeof(lzo_uint), 1, out);
        fwrite(outbuf, 1, outlen, out);
    /**************/
    /* Decompress */
    /**************/
    } else {
        r = lzo1x_decompress(inbuf,inlen,NULL,&outlen,NULL);
        if (r != LZO_E_OK){
            printf("Error - Decompression failed! Please open an issue on github about this. Error code : DEC-%d", r);
            return 1;
        }
    }

    free(inbuf);
   
    fclose(out);
    return 0;
    
}