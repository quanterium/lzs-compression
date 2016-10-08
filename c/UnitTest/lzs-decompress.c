/*****************************************************************************
 *
 * \file
 *
 * \brief Decompression of a file
 *
 ****************************************************************************/


/*****************************************************************************
 * Includes
 ****************************************************************************/

#include "lzs-decompression.h"

#include <stdio.h>
#include <string.h>         /* For memset() */

#include <fcntl.h>
#include <unistd.h>

/*****************************************************************************
 * Functions
 ****************************************************************************/

int main(int argc, char **argv)
{
	int in_fd;
	int out_fd;
	int read_len;
	uint8_t in_buffer[16];
    uint8_t out_buffer[16];
    uint8_t history_buffer[MAX_HISTORY_SIZE];
    DecompressParameters_t  decompress_params;
    size_t  out_length;

    if (argc < 3)
    {
    	printf("Too few arguments\n");
    	exit(1);
    }
    in_fd = open(argv[1], O_RDONLY);
    if (in_fd < 0)
    {
    	perror("argv[1]");
    	exit(2);
    }
    out_fd = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if (out_fd < 0)
    {
    	perror("argv[2]");
    	exit(3);
    }

    // Initialise
    decompress_params.historyPtr = history_buffer;
    decompress_params.historyBufferSize = sizeof(history_buffer);
    decompress_init(&decompress_params);

    // Decompress bounded by input buffer size
    decompress_params.inPtr = in_buffer;
    decompress_params.inLength = 0;
    decompress_params.outPtr = out_buffer;
    decompress_params.outLength = sizeof(out_buffer);
    while (1)
    {
    	if (decompress_params.inLength == 0)
    	{
    		read_len = read(in_fd, in_buffer, sizeof(in_buffer));
    		if (read_len > 0)
    		{
        		decompress_params.inPtr = in_buffer;
        		decompress_params.inLength = read_len;
    		}
    	}
        if (
                (decompress_params.inLength == 0) &&
                ((decompress_params.status & D_STATUS_INPUT_STARVED) != 0)
           )
        {
            break;
        }

        out_length = decompress_incremental(&decompress_params);
        if (out_length)
        {
        	write(out_fd, decompress_params.outPtr - out_length, out_length);
        	decompress_params.outPtr = out_buffer;
        	decompress_params.outLength = sizeof(out_buffer);
        }
        if ((decompress_params.status & ~(D_STATUS_INPUT_STARVED | D_STATUS_INPUT_FINISHED)) != 0)
        {
            printf("Exit with status %02X\n", decompress_params.status);
        }
    }

    return 0;
}
