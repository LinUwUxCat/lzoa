# lzox
Is a lightweight LZO compression and decompression tool.
It only supports lzo 1x_1, meaning only `lzo1x_1_compress` and `lzo1x_decompress` are used.\
LZO1X_1 is the fastest algorithm of all LZOXX_X algorithms - see [here](https://github.com/nemequ/lzo/blob/master/doc/LZO.TXT#L74).\
Note - this program is NOT compatible with [lzop](http://www.lzop.org/)! While lzop's header files are more complex, this one only contains a minimal header with the 4 characters "LZOX" and the uncompressed file size on 8 bytes, thus taking only 12 bytes before the compressed data.
