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
static char args_doc[] = "FILENAME";
static struct argp_option options[] = {
    {"compress", 'c', 0, 0, "compress to a lzo file"},
    {"decompress", 'd', 0, 0, "decompress a lzo file"},
    {"output", 'o', "FILE", 0, "output to FILE."},
    {"parts", 'p', "INT", 0, "Currently does nothing"}, //"split the output into X parts. They will be named using the -o option with .X extension added."},//TODO
    {"verbose", 'v', 0,0,"details the process."},
    { 0 }
};
int8_t tasktype = 0;
int8_t verbose = 0;
int parts = 0;
char* outfilename;
char* infilename;
//key : key for the option (e.g. c, d, o, p)
//arg : its value (e.g. cat -c bar -> key=c arg=bar)
//https://www.gnu.org/software/libc/manual/html_node/Argp-Parser-Functions.html
error_t argp_parser (int key, char *arg, struct argp_state *state){
    switch (key){
        case 'c':
            tasktype = 1;
            break;
        case 'd':
            tasktype = 2;
            break;
        case 'o':
            outfilename = arg;
            break;
        case 'p':
            parts = atoi(arg);
            break;
        case 'v':
            verbose = 1;
            break;
        case ARGP_KEY_NO_ARGS:
            argp_usage(state);
        case ARGP_KEY_ARG:
            infilename = arg;
            state->next = state->argc;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}


static struct argp argp = { options, argp_parser, args_doc, doc, 0, 0, 0 };

int main(int argc, char *argv[]){
    int r; //result

    if (lzo_init() != LZO_E_OK) {
        printf("Error - lzo_init() failed. This typically indicates a compiler bug. Try compiling again.\n");
        return 2; //1 is a bug in the code, here it's a compiler bug. I don't know what to return, so have a 2.
    }
    argp_parse(&argp, argc, argv, 0, 0, 0); //parse args
    //start handling them args
    switch (tasktype){
        case 0:
            printf("You must choose to either compress or decompress!\n");
            return 0;
        case 1:
            if (verbose) printf("Task type : compressing\n");
            break;
        case 2:
            if (verbose) printf("Task type : decompressing\n");
            break;
        default:
            return 0; //this should never happen
    }
    fflush(stdout);
    if (infilename == NULL) return 0; //this should never happen
    if (outfilename == NULL){
        char* extension = tasktype==1?"lzo":"out";
        if (verbose) printf("Outfile name not precised. Defaulting to %s.%s\n", infilename, extension);
        fflush(stdout);
        outfilename = (char *) malloc(strlen(infilename) + 5);
        sprintf(outfilename, "%s.%s", infilename, extension);
    }

    /**************/
    /*   Infile   */
    /**************/
    if (verbose) printf("Opening %s...\n", infilename);
    FILE *in = fopen(infilename, "r"); //open the input file
    if (in == NULL) {
        printf("Error - Could not open input file.\n");
        return 1;
    }
    if (verbose) printf("Done.\n");
    fflush(stdout);

    fseek(in, 0, SEEK_END);     //
    lzo_uint inlen = ftell(in); // Get file size
    fseek(in, 0, SEEK_SET);     //
    if (verbose) printf("File %s has size of %lu\n", infilename, inlen);
    if (verbose) printf("Allocating buffer\n");
    unsigned char __LZO_MMODEL *inbuf = (unsigned char *)malloc(inlen);

    /***************/
    /*   Outfile   */
    /***************/
    if (verbose) printf("opening %s\n", outfilename);
    lzo_uint outlen;
    FILE *out;
    out = fopen(outfilename, "w");
    fflush(stdout);


    /***************/
    /* Compression */
    /***************/
    if (tasktype == 1){
        if (verbose) printf("Starting compression\n");
        fread(inbuf, 1, inlen, in);
        fclose(in);
        unsigned char __LZO_MMODEL outbuf [ inlen + inlen / 16 + 67 ]; //shouldn't this be 64?
        r = lzo1x_1_compress(inbuf,inlen,outbuf,&outlen,wrkmem);

        if (r != LZO_E_OK){
            printf("Error - Compression failed! Please open an issue on github about this. Error code : COM%d\n", r);
            return 1; //no clue
        }
        if (outlen >= inlen){
            if (verbose) printf("Note - Data is not compressible.\n");
        }
        if (verbose) printf("Compression successful.\n");
        fwrite("LZOA", 1, 4, out);
        fwrite(&inlen, sizeof(lzo_uint), 1, out);
        fwrite(outbuf, 1, outlen, out);
        if (verbose) printf("Wrote to %s.\n", outfilename);
    /**************/
    /* Decompress */
    /**************/
    } else { //there is 3 possible task types and we already yeet the possibilities of having 0 when we manage the args so we don't have to compare to 2 again here
        if (verbose) printf("Startin decompression\n");
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
            printf("Error - Decompression failed! Please check the integrity of your lzo file. Error code : DEC%d\n", r);
            return 1;
        }
        if (verbose) printf("Decompression successful.\n");
        fwrite(outbuf, 1, outlen, out);
        if (verbose) printf("Wrote to %s\n", outfilename);
    }
    free(inbuf);
    fclose(out);
    return 0;
}