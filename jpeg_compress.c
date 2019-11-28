#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "jpeglib.h"

int main(int argc, char *argv[]) {
    char rdbuf[64];
    if(argc != 2) {
        printf("USAGE %s [ppm of pgm]\n", argv[0]);
        return 1;
    }
    int fd = open(argv[1], O_RDONLY);
    int bytes_per_pixel, color_space;
    memset(rdbuf, 0x00, sizeof(rdbuf));
    read(fd, rdbuf, 2);
    if(strncmp(rdbuf, "P6", 2) == 0) {
        printf("head: '%s'\n", rdbuf);
        bytes_per_pixel = 3;
        color_space = JCS_RGB; //RGB
    }
    else if(strncmp(rdbuf, "P5", 2) == 0) {
        printf("head: '%s'\n", rdbuf);
        bytes_per_pixel = 1;
        color_space = JCS_GRAYSCALE; //GrayScale
    }

    memset(rdbuf, 0x00, sizeof(rdbuf));
    int i; 
    int break_flag = 0;
    for(i=0; i < 10; i++) {
        while(break_flag == 0) {
            read(fd, &rdbuf[i], 1);
            if(rdbuf[i] >= 48 && rdbuf[i] <= 57) {
                break_flag = 1;
                i++;
                break;
            }
        }
        read(fd, &rdbuf[i], 1);
        if(rdbuf[i] < 48 || rdbuf[i] > 57)
            break;
    }
    int width = atoi(rdbuf);

    memset(rdbuf, 0x00, sizeof(rdbuf));
    for(i=0; i < 10; i++) {
        while(break_flag == 0) {
            read(fd, &rdbuf[i], 1);
            if(rdbuf[i] >= 48 && rdbuf[i] <= 57) {
                break_flag = 1;
                i++;
                break;
            }
        }
        read(fd, &rdbuf[i], 1);
        if(rdbuf[i] < 48 || rdbuf[i] > 57)
            break;
    }
    int height = atoi(rdbuf);

    memset(rdbuf, 0x00, sizeof(rdbuf));
    for(i=0; i < 10; i++) {
        while(break_flag == 0) {
            read(fd, &rdbuf[i], 1);
            if(rdbuf[i] >= 48 && rdbuf[i] <= 57) {
                break_flag = 1;
                i++;
                break;
            }
        }
        read(fd, &rdbuf[i], 1);
        if(rdbuf[i] < 48 || rdbuf[i] > 57)
            break;
    }
    int color = atoi(rdbuf);

    printf("width:%d, height:%d color:%d\n", width, height, color);

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *outfilePoint = fopen("output.jpg", "w");

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest( &cinfo, outfilePoint );

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = bytes_per_pixel;
    cinfo.in_color_space = color_space;
    printf(" image_width:%d\n image_height:%d\n input_components:%d\n", cinfo.image_width, cinfo.image_height, cinfo.input_components);

    jpeg_set_defaults(&cinfo);

    cinfo.dct_method=1;
    cinfo.write_JFIF_header = TRUE;
    cinfo.density_unit= 1;
    cinfo.X_density = 300;
    cinfo.Y_density = 300;
    //jpeg_set_quality(&cinfo, 100, TRUE);

    jpeg_start_compress(&cinfo, TRUE);  //located in '../scan/libjpeg-turbo-1.1.0/src/jcapistd.c'


    int row_idx = 0;
    int rdbytes = 0;
    int total_rdbytes = 0;
    JSAMPROW row_pointer[1];
#if 1
    int bufsize = cinfo.image_width * cinfo.input_components;
    unsigned char *buf = malloc(bufsize);
    memset(buf, 0x00, bufsize);
    while ((rdbytes = read(fd, buf, bufsize)) > 0) {
        row_pointer[0] = buf;
        jpeg_write_scanlines(&cinfo, row_pointer, 1); //located in 'jcapistd.c'
        memset(buf, 0x00, bufsize);
        total_rdbytes += rdbytes;
        //printf("cinfo.next_scanline: %d\n",cinfo.next_scanline);
        if(cinfo.next_scanline >= cinfo.image_height+1)
            break;
    }
#else
    int bufsize = cinfo.image_height * cinfo.image_width * cinfo.input_components;
    unsigned char *buf = malloc(bufsize);
    if(buf != NULL)
        printf("malloc success (%d)\n", bufsize);
    memset(buf, 0x00, bufsize);
    rdbytes = read(fd, buf, bufsize);
    while( cinfo.next_scanline < cinfo.image_height )
    {
        row_pointer[0] = &buf[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
        jpeg_write_scanlines( &cinfo, row_pointer, 1 );
        total_rdbytes += rdbytes;
        printf("cinfo.next_scanline: %d\n",cinfo.next_scanline);
    }
#endif
    printf("total_rdbytes:%d\n", total_rdbytes);
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress( &cinfo );

    free(buf);
    close(fd);
    fclose(outfilePoint);
    
    return 0;
}
