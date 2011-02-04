/* 
 * Stereo image binary large object (blob) header file
 * - used by the bb2.cc/h driver for logging/playback stereo data 
 * - largely based on DGC code from Caltech
*/


#ifndef STEREO_IMAGE_BLOB_H
#define STEREO_IMAGE_BLOB_H

#ifdef __cplusplus
extern "C"
{
#endif
  
#include <stdint.h>
  
  // blob version number
#define STEREO_IMAGE_BLOB_VERSION 0x04
  
  // maximum image dimensions
#define STEREO_IMAGE_BLOB_MAX_COLS 1024
  
  // maximum image dimensions
#define STEREO_IMAGE_BLOB_MAX_ROWS 768
  
  // Contains a rectified left/right stereo image pair and (optionally)
  // the left disparity image.  Disparity values are scaled up and
  // stored as 16-bit integers.
  typedef struct _StereoImageBlob
  {    
    // Unique frame id (increased monotonically)
    int32_t frameId;
    
    // Image timestamp
    uint64_t timestamp;
    
    // Reserved for future use; must be all zero.
    uint32_t reserved[16];
    
    // Image dimensions 
    int32_t cols, rows;
    
    // Image row width
    int rowinc;
    
    // Number of channels in rectified image (1 = grayscale, 3 = RGB)
    int32_t channels;
    
    // Extra padding to align with page boundaries
    uint8_t padding[408];

    // Input buffer for the red channel 
    uint8_t left_buffer[STEREO_IMAGE_BLOB_MAX_COLS * STEREO_IMAGE_BLOB_MAX_ROWS];
    
    // Input buffer for the green channel
    uint8_t right_buffer[STEREO_IMAGE_BLOB_MAX_COLS * STEREO_IMAGE_BLOB_MAX_ROWS];
    
    float shutter;
    float gain;
    
  } __attribute__((packed)) StereoImageBlob;
  
  
  
#ifdef __cplusplus
}
#endif

#endif
