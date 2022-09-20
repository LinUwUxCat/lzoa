/*  main.c
    this file contains the main function of lzox
    it uses the lzo library - for more info, please see http://www.oberhumer.com/opensource/lzo/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minilzo.h"

#define HEAP_ALLOC(var,size) lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]
static HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

#include <argp.h>
const char *argp_program_version = "lzoa 0.2";
const char *argp_program_bug_address = "linuxcat@linuxcat.tech";
static char doc[] = "LZOA is a command-line tool to compress and decompress LZO files using the miniLZO library by Markus Franz Xaver Johannes Oberhumer";
static char args_doc[] = "[FILENAME]...";
static struct argp_option options[] = {
    {"compress", 'c', 0, 0, "compress to a lzo file"},
    {"decompress", 'd', 0, 0, "decompress a lzo file"},
    {"output", 'o', "FILE", 0, "output to FILE."},
    {"parts", 'p', "NUMBER", OPTION_ARG_OPTIONAL, "split the output into X parts. They will be named using the -o option with .X extension added."},
    { 0 }
};

//key : key for the option (e.g. c, d, o, p)
//arg : its value (e.g. cat -c bar -> key=c arg=bar)
//https://www.gnu.org/software/libc/manual/html_node/Argp-Parser-Functions.html
error_t argp_parser (int key, char *arg, struct argp_state *state){
    switch (key){
        //this is where i do shit
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, argp_parser, args_doc, doc, 0, 0, 0 }

int main(int argc, char *argv[]){

    int r; //result

    if (lzo_init() != LZO_E_OK) {
        printf("Error - lzo_init() failed. This typically indicates a compiler bug. Try compiling again.\n");
        return 2; //1 is a bug in the code, here it's a compiler bug. I don't know what to return, so have a 2.
    }
    
    /*if (argc == 1 || argv[1][0] != '-' || (argc < 3 && argv[1][1]=='c') || (argv[1][1] == 'd' && argc < 4)) {
        printf("lzoa 0.1 - by LinuxCat\nUsing miniLZO library by Markus Franz Xaver Johannes Oberhumer\nUsage: %s <OPTION> <input file> [output file]\n\nOptions : \n\t-c\tCompress input file to output file\n\t-d\tDecompress input file to output file\n\nIf no output is specified, it will be FILE_NAME.lzo if -c is used.\n", argv[0]); //congrats, you read to the end of this line! now go listen to this https://www.youtube.com/watch?v=9R80DUsixGg
        return 0; //actually i have no fucking idea what i'm supposed to return. Here's a 0
    }*/
    
    argp_parse(argp, argc, argv, )

    /**************/
    /*   Infile   */
    /**************/
    FILE *in = fopen(argv[2], "r"); //open the input file
    if (in == NULL) {
        printf("Error - Could not open input file.\n");
        return 1;
    }

    fseek(in, 0, SEEK_END);
    lzo_uint inlen = ftell(in);
    fseek(in, 0, SEEK_SET);

    unsigned char __LZO_MMODEL *inbuf = (unsigned char *)malloc(inlen);

    /***************/
    /*   Outfile   */
    /***************/
    lzo_uint outlen;
    FILE *out;
    if (argc < 4 || argv[3][1] == 'c'){
        out = fopen(strcat(argv[2], ".lzo"), "w"); //the evkitpro incident
    } else {
        out = fopen(argv[3], "w");
    }


    /***************/
    /* Compression */
    /***************/
    if (argv[1][1] == 'c'){
        fread(inbuf, 1, inlen, in);
        fclose(in);
        unsigned char __LZO_MMODEL outbuf [ inlen + inlen / 16 + 67 ];
        r = lzo1x_1_compress(inbuf,inlen,outbuf,&outlen,wrkmem);

        if (r != LZO_E_OK){
            printf("Error - Compression failed! Please open an issue on github about this. Error code : COM%d\n", r);
            return 1; //no clue
        }
        if (outlen >= inlen){
            printf("Note - Data is not compressible.");
        }
        fwrite("LZOA", 1, 4, out);
        fwrite(&inlen, sizeof(lzo_uint), 1, out);
        fwrite(outbuf, 1, outlen, out);
    /**************/
    /* Decompress */
    /**************/
    } else {
        fread(&outlen, sizeof(char), 4, in); //reusing variables woooo
        if (outlen != 0x414f5a4c){ //=LZOA
            printf("Error - Invalid header.\n");
            return 1;
        }
        fread(&outlen, sizeof(lzo_uint), 1, in); //read the length of the uncompressed data
        unsigned char __LZO_MMODEL outbuf [ outlen ];
        fflush(stdout);
        fread(inbuf, 1, inlen, in);
        fclose(in);
        r = lzo1x_decompress(inbuf,inlen - sizeof(char)*4 - sizeof(lzo_uint),outbuf,&outlen,NULL); //dammit why do i need an outlen here
        if (r != LZO_E_OK){
            printf("Error - Decompression failed! Please open an issue on github about this. Error code : DEC%d\n", r);
            return 1;
        }
        fwrite(outbuf, 1, outlen, out);
    }
    free(inbuf);
    fclose(out);
    return 0;
}