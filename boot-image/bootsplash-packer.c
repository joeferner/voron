/*
 * Kernel based bootsplash.
 *
 * (Splash file packer tool)
 *
 * Authors:
 * Max Staudt <mstaudt@suse.de>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <endian.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bootsplash_file.h"


static void print_help(char *progname)
{
	printf("Usage: %s [OPTIONS] outfile\n", progname);
	printf("\n"
	       "Options, executed in order given:\n"
	       "  -h, --help                   Print this help message\n"
	       "\n"
	       "  --bg_red <u8>                Background color (red part)\n"
	       "  --bg_green <u8>              Background color (green part)\n"
	       "  --bg_blue <u8>               Background color (blue part)\n"
	       "  --bg_reserved <u8>           (do not use)\n"
	       "  --frame_ms <u16>             Minimum milliseconds between animation steps\n"
	       "\n"
	       "  --picture                    Start describing the next picture\n"
	       "  --pic_width <u16>            Picture width in pixels\n"
	       "  --pic_height <u16>           Picture height in pixels\n"
	       "  --pic_position <u8>             Coarse picture placement:\n"
	       "                                  0x00 - Top left\n"
	       "                                  0x01 - Top\n"
	       "                                  0x02 - Top right\n"
	       "                                  0x03 - Right\n"
	       "                                  0x04 - Bottom right\n"
	       "                                  0x05 - Bottom\n"
	       "                                  0x06 - Bottom left\n"
	       "                                  0x07 - Left\n"
	       "\n"
	       "                                Flags:\n"
	       "                                 0x10 - Calculate offset from corner towards center,\n"
	       "                                         rather than from center towards corner\n"
	       "  --pic_position_offset <u16>  Distance from base position in pixels\n"
	       "  --pic_anim_type <u8>         Animation type:\n"
	       "                                 0 - None\n"
	       "                                 1 - Forward loop\n"
	       "  --pic_anim_loop <u8>         Loop point for animation\n"
	       "\n"
	       "  --blob <filename>            Include next data stream\n"
	       "  --blob_type <u16>            Type of data\n"
	       "  --blob_picture_id <u8>       Picture to associate this blob with, starting at 0\n"
	       "                                 (default: number of last --picture)\n"
	       "\n");
	printf("This tool will write %s files.\n\n",
#if __BYTE_ORDER == __BIG_ENDIAN
	       "Big Endian (BE)");
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	       "Little Endian (LE)");
#else
#error
#endif
}


struct blob_entry {
	struct blob_entry *next;

	char *fn;

	struct splash_blob_header header;
};


static void dump_file_header(struct splash_file_header *h)
{
	printf(" --- File header ---\n");
	printf("\n");
	printf("  version:     %5u\n", h->version);
	printf("\n");
	printf("  bg_red:      %5u\n", h->bg_red);
	printf("  bg_green:    %5u\n", h->bg_green);
	printf("  bg_blue:     %5u\n", h->bg_blue);
	printf("  bg_reserved: %5u\n", h->bg_reserved);
	printf("\n");
	printf("  num_blobs:   %5u\n", h->num_blobs);
	printf("  num_pics:    %5u\n", h->num_pics);
	printf("\n");
	printf("  frame_ms:    %5u\n", h->frame_ms);
	printf("\n");
}

static void dump_pic_header(struct splash_pic_header *ph)
{
	printf(" --- Picture header ---\n");
	printf("\n");
	printf("  width:           %5u\n", ph->width);
	printf("  height:          %5u\n", ph->height);
	printf("\n");
	printf("  num_blobs:       %5u\n", ph->num_blobs);
	printf("\n");
	printf("  position:        %0x3x\n", ph->position);
	printf("  position_offset: %5u\n", ph->position_offset);
	printf("\n");
	printf("  anim_type:       %5u\n", ph->anim_type);
	printf("  anim_loop:       %5u\n", ph->anim_loop);
	printf("\n");
}

static void dump_blob(struct blob_entry *b)
{
	printf(" --- Blob header ---\n");
	printf("\n");
	printf("  length:     %7u\n", b->header.length);
	printf("  type:       %7u\n", b->header.type);
	printf("\n");
	printf("  picture_id: %7u\n", b->header.picture_id);
	printf("\n");
}


#define OPT_MAX(var, max) \
	do { \
		if ((var) > max) { \
			fprintf(stderr, "--%s: Invalid value\n", \
			long_options[option_index].name); \
			break; \
		} \
	} while (0)

static struct option long_options[] = {
	{"help", 0, 0, 'h'},
	{"bg_red", 1, 0, 10001},
	{"bg_green", 1, 0, 10002},
	{"bg_blue", 1, 0, 10003},
	{"bg_reserved", 1, 0, 10004},
	{"frame_ms", 1, 0, 10005},
	{"picture", 0, 0, 20000},
	{"pic_width", 1, 0, 20001},
	{"pic_height", 1, 0, 20002},
	{"pic_position", 1, 0, 20003},
	{"pic_position_offset", 1, 0, 20004},
	{"pic_anim_type", 1, 0, 20005},
	{"pic_anim_loop", 1, 0, 20006},
	{"blob", 1, 0, 30000},
	{"blob_type", 1, 0, 30001},
	{"blob_picture_id", 1, 0, 30002},
	{NULL, 0, NULL, 0}
};


int main(int argc, char **argv)
{
	FILE *of;
	char *ofn;
	int c;
	int option_index = 0;

	unsigned long ul;
	struct splash_file_header fh = {};
	struct splash_pic_header ph[255];
	struct blob_entry *blob_first = NULL;
	struct blob_entry *blob_last = NULL;
	struct blob_entry *blob_cur = NULL;

	if (argc < 2) {
		print_help(argv[0]);
		return EXIT_FAILURE;
	}


	/* Parse and and execute user commands */
	while ((c = getopt_long(argc, argv, "h",
			  long_options, &option_index)) != -1) {
		switch (c) {
		case 10001:	/* bg_red */
			ul = strtoul(optarg, NULL, 0);
			OPT_MAX(ul, 255);
			fh.bg_red = ul;
			break;
		case 10002:	/* bg_green */
			ul = strtoul(optarg, NULL, 0);
			OPT_MAX(ul, 255);
			fh.bg_green = ul;
			break;
		case 10003:	/* bg_blue */
			ul = strtoul(optarg, NULL, 0);
			OPT_MAX(ul, 255);
			fh.bg_blue = ul;
			break;
		case 10004:	/* bg_reserved */
			ul = strtoul(optarg, NULL, 0);
			OPT_MAX(ul, 255);
			fh.bg_reserved = ul;
			break;
		case 10005:	/* frame_ms */
			ul = strtoul(optarg, NULL, 0);
			OPT_MAX(ul, 65535);
			fh.frame_ms = ul;
			break;


		case 20000:	/* picture */
			if (fh.num_pics >= 255) {
				fprintf(stderr, "--%s: Picture array full\n",
					long_options[option_index].name);
				break;
			}

			fh.num_pics++;
			break;

		case 20001:	/* pic_width */
			ul = strtoul(optarg, NULL, 0);
			OPT_MAX(ul, 65535);
			ph[fh.num_pics - 1].width = ul;
			break;

		case 20002:	/* pic_height */
			ul = strtoul(optarg, NULL, 0);
			OPT_MAX(ul, 65535);
			ph[fh.num_pics - 1].height = ul;
			break;

		case 20003:	/* pic_position */
			ul = strtoul(optarg, NULL, 0);
			OPT_MAX(ul, 255);
			ph[fh.num_pics - 1].position = ul;
			break;

		case 20004:	/* pic_position_offset */
			ul = strtoul(optarg, NULL, 0);
			OPT_MAX(ul, 255);
			ph[fh.num_pics - 1].position_offset = ul;
			break;

		case 20005:	/* pic_anim_type */
			ul = strtoul(optarg, NULL, 0);
			OPT_MAX(ul, 255);
			ph[fh.num_pics - 1].anim_type = ul;
			break;

		case 20006:	/* pic_anim_loop */
			ul = strtoul(optarg, NULL, 0);
			OPT_MAX(ul, 255);
			ph[fh.num_pics - 1].anim_loop = ul;
			break;


		case 30000:	/* blob */
			if (fh.num_blobs >= 65535) {
				fprintf(stderr, "--%s: Blob array full\n",
					long_options[option_index].name);
				break;
			}

			blob_cur = calloc(1, sizeof(struct blob_entry));
			if (!blob_cur) {
				fprintf(stderr, "--%s: Out of memory\n",
					long_options[option_index].name);
				break;
			}

			blob_cur->fn = optarg;
			if (fh.num_pics)
				blob_cur->header.picture_id = fh.num_pics - 1;

			if (!blob_first)
				blob_first = blob_cur;
			if (blob_last)
				blob_last->next = blob_cur;
			blob_last = blob_cur;
			fh.num_blobs++;
			break;

		case 30001:	/* blob_type */
			if (!blob_cur) {
				fprintf(stderr, "--%s: No blob selected\n",
					long_options[option_index].name);
				break;
			}

			ul = strtoul(optarg, NULL, 0);
			OPT_MAX(ul, 255);
			blob_cur->header.type = ul;
			break;

		case 30002:	/* blob_picture_id */
			if (!blob_cur) {
				fprintf(stderr, "--%s: No blob selected\n",
					long_options[option_index].name);
				break;
			}

			ul = strtoul(optarg, NULL, 0);
			OPT_MAX(ul, 255);
			blob_cur->header.picture_id = ul;
			break;



		case 'h':
		case '?':
		default:
			print_help(argv[0]);
			goto EXIT;
		} /* switch (c) */
	} /* while ((c = getopt_long(...)) != -1) */

	/* Consume and drop lone arguments */
	while (optind < argc) {
		ofn = argv[optind];
		optind++;
	}


	/* Read file lengths */
	for (blob_cur = blob_first; blob_cur; blob_cur = blob_cur->next) {
		FILE *f;
		long pos;
		int i;

		if (!blob_cur->fn)
			continue;

		f = fopen(blob_cur->fn, "rb");
		if (!f)
			goto ERR_FILE_LEN;

		if (fseek(f, 0, SEEK_END))
			goto ERR_FILE_LEN;

		pos = ftell(f);
		if (pos < 0 || pos > (1 << 30))
			goto ERR_FILE_LEN;

		blob_cur->header.length = pos;

		fclose(f);
		continue;

ERR_FILE_LEN:
		fprintf(stderr, "Error getting file length (or too long): %s\n",
			blob_cur->fn);
		if (f)
			fclose(f);
		continue;
	}


	/* Set magic headers */
#if __BYTE_ORDER == __BIG_ENDIAN
	memcpy(&fh.id[0], BOOTSPLASH_MAGIC_BE, 16);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	memcpy(&fh.id[0], BOOTSPLASH_MAGIC_LE, 16);
#else
#error
#endif
	fh.version = BOOTSPLASH_VERSION;

	/* Set blob counts */
	for (blob_cur = blob_first; blob_cur; blob_cur = blob_cur->next) {
		if (blob_cur->header.picture_id < fh.num_pics)
			ph[blob_cur->header.picture_id].num_blobs++;
	}


	/* Dump structs */
	dump_file_header(&fh);

	for (ul = 0; ul < fh.num_pics; ul++)
		dump_pic_header(&ph[ul]);

	for (blob_cur = blob_first; blob_cur; blob_cur = blob_cur->next)
		dump_blob(blob_cur);


	/* Write to file */
	printf("Writing splash to file: %s\n", ofn);
	of = fopen(ofn, "wb");
	if (!of)
		goto ERR_WRITING;

	if (fwrite(&fh, sizeof(struct splash_file_header), 1, of) != 1)
		goto ERR_WRITING;

	for (ul = 0; ul < fh.num_pics; ul++) {
		if (fwrite(&ph[ul], sizeof(struct splash_pic_header), 1, of)
		    != 1)
			goto ERR_WRITING;
	}

	blob_cur = blob_first;
	while (blob_cur) {
		struct blob_entry *blob_old = blob_cur;
		FILE *f;
		char *buf[256];
		uint32_t left;

		if (fwrite(&blob_cur->header,
			   sizeof(struct splash_blob_header), 1, of) != 1)
			goto ERR_WRITING;

		if (!blob_cur->header.length || !blob_cur->fn)
			continue;

		f = fopen(blob_cur->fn, "rb");
		if (!f)
			goto ERR_FILE_COPY;

		left = blob_cur->header.length;
		while (left >= sizeof(buf)) {
			if (fread(buf, sizeof(buf), 1, f) != 1)
				goto ERR_FILE_COPY;
			if (fwrite(buf, sizeof(buf), 1, of) != 1)
				goto ERR_FILE_COPY;
			left -= sizeof(buf);
		}
		if (left) {
			if (fread(buf, left, 1, f) != 1)
				goto ERR_FILE_COPY;
			if (fwrite(buf, left, 1, of) != 1)
				goto ERR_FILE_COPY;
		}

		/* Pad data stream to 16 bytes */
		if (left % 16) {
			if (fwrite("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
					16 - (left % 16), 1, of) != 1)
				goto ERR_FILE_COPY;
		}

		fclose(f);
		blob_cur = blob_cur->next;
		free(blob_old);
		continue;

ERR_FILE_COPY:
		if (f)
			fclose(f);
		goto ERR_WRITING;
	}

	fclose(of);

EXIT:
	return EXIT_SUCCESS;


ERR_WRITING:
	fprintf(stderr, "Error writing splash.\n");
	fprintf(stderr, "The output file is probably corrupt.\n");
	if (of)
		fclose(of);

	while (blob_cur) {
		struct blob_entry *blob_old = blob_cur;

		blob_cur = blob_cur->next;
		free(blob_old);
	}

	return EXIT_FAILURE;
}
