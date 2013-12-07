#ifndef SETTINGS_H
#define SETTINGS_H

// Enable or disable streams
#define SETUP_STREAM_RGB 0x1
#define SETUP_STREAM_IR  0x2
#define SETUP_STREAMS (SETUP_STREAM_RGB | SETUP_STREAM_IR) // (SETUP_STREAM_RGB) or (SETUP_STREAM_IR)

// Lengths of frame buffers
#define SETUP_BUFFER_LENGTH_G2P 4
#define SETUP_BUFFER_LENGTH_P2C 4
#define SETUP_BUFFER_LENGTH_C2S 4

// How many milliseconds between frames
#define SETUP_STREAM_INTERVAL 1000

// How busy to poll channels (microseconds)
#define SETUP_POLL_DELAY 10000

// Images; RGB is a raw Bayer image (RGGB)
#define SETUP_IMAGE_WIDTH_RGB 640
#define SETUP_IMAGE_HEIGHT_RGB 480
#define SETUP_IMAGE_SIZE_RAW_RGB (SETUP_IMAGE_WIDTH_RGB * SETUP_IMAGE_HEIGHT_RGB)
#define SETUP_IMAGE_SIZE_RGB (SETUP_IMAGE_WIDTH_RGB * SETUP_IMAGE_HEIGHT_RGB * 3)

// ..and IR is packed 10 bit pixels but here only 8 of those bits are used
#define SETUP_IMAGE_WIDTH_IR 640
#define SETUP_IMAGE_HEIGHT_IR 488
#define SETUP_IMAGE_SIZE_RAW_IR ((SETUP_IMAGE_WIDTH_IR * SETUP_IMAGE_HEIGHT_IR * 10) / 8)
#define SETUP_IMAGE_SIZE_IR (SETUP_IMAGE_WIDTH_IR * SETUP_IMAGE_HEIGHT_IR)

// Server
#define SETUP_SERVER_ADDRESS "0.0.0.0"
#define SETUP_SERVER_PORT 8000
#define SETUP_SERVER_THREADS 8
#define SETUP_SERVER_PATH_RGB "/rgb"
#define SETUP_SERVER_PATH_IR "/ir"

#endif // SETTINGS_H
