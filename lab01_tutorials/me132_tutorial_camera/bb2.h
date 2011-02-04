/* 
 * This is a driver for the Bumblebee2 cameras. It incorporates 
 * files from the Point Grey Research website and is based largely
 * in part on their sample code. 
 */

#ifndef _BUMBLEBEE2_HH_
#define _BUMBLEBEE2_HH_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <dc1394/conversions.h>
#include <dc1394/control.h>
#include <dc1394/utils.h>
#include <dc1394/register.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>


//=============================================================================
// PGR Includes
//=============================================================================
#include <pgrlibdcstereo/pgr_conversions.h>
#include <pgrlibdcstereo/pgr_registers.h>
#include <pgrlibdcstereo/pgr_stereocam.h>
#include <math.h>

#include "StereoImageBlob.h"

enum CameraType{
  BB_REFERENCE = 0,
  BB_LEFT,
  BB_RIGHT,
};



// grab color image
int grabColorImages(
		    PGRStereoCamera_t* 	stereoCamera, 
		    dc1394bayer_method_t bayerMethod,
		    unsigned char* 	pucDeInterleaved,
		    unsigned char* 	pucRGB,
		    unsigned char* 	pucGreen,
		    unsigned char** 	ppucRightRGB,
		    unsigned char** 	ppucLeftRGB,
		    unsigned char** 	ppucCenterRGB,
		    TriclopsInput*  	pTriclopsInput,
		    uint64_t*           timestamp
		    );

// grab color image
int grabMonoImages(
		   PGRStereoCamera_t* 	stereoCamera, 
		   unsigned char* 	pucDeInterleaved,
		   unsigned char** 	ppucRightMono8,
		   unsigned char** 	ppucLeftMono8,
		   unsigned char** 	ppucCenterMono8,
		   TriclopsInput*  	pTriclopsInput,
		   uint64_t*            timestamp
		   );

// grab color image
int grabColorImages_RGB(
			PGRStereoCamera_t* 	stereoCamera, 
			dc1394bayer_method_t bayerMethod,
			unsigned char* 	pucDeInterleaved,
			unsigned char* 	pucRGB,
			unsigned char* 	pucRedGreenBlue,
			unsigned char** ppucRightRGB,
			unsigned char** ppucLeftRGB,
			unsigned char** ppucCenterRGB,
			TriclopsInput*	pTriclopsInput,
			uint64_t*       timestamp
			);

class BumbleBee
{
 public:
  // default constructor
  BumbleBee();

  // constructor with specified camera ID
  BumbleBee(int bbId);

  // constructor with specified camera ID and scale
  BumbleBee(int bbId, int downscale);

  // constructor with specified camera ID and scale and color enabled
  BumbleBee(int bbId, int downscale, bool enable_color);

  // default destructor
  ~BumbleBee();

  // initialize stereocamera
  int init(float shutter=0.0105);

  // initialize sequence
  int init_try(float shutter);

  // capture images from camera and do stereo processing
  int capture();

  // capture fast and save left, right raw images only
  int captureBlob(StereoImageBlob* blob);

  // capture the left and right images only
  int captureImageOnly();

  // capture raw image only
  int captureRawImageOnly();

  // return stereoimage width
  unsigned int getImageWidth();
  
  // return stereoimage Height
  unsigned int getImageHeight();

  // return rectified image width
  int getRectifiedWidth();
  
  // return rectified image height
  int getRectifiedHeight();

  // return the pixel value of the rectified image
  uint8_t getRectifiedPixel(int i, int j, CameraType type);

  // return the pixel value of the rectified image
  int getRectifiedColorPixel(int i, int j, CameraType type, uint8_t* red, uint8_t* green, uint8_t* blue);

  // return disparity image width
  unsigned int getDisparityWidth();
  
  // return disparity image height
  unsigned int getDisparityHeight();

  // return input width
  unsigned int getInputWidth();
  
  // return input Height
  unsigned int getInputHeight();

  // Get the left image from the stereocamera
  int getLeftImage(unsigned char* imgBuffer);

  // Get the right image from the stereocamera
  int getRightImage(unsigned char* imgBuffer);

  // Get the rectified image
  int getRectifiedImage(unsigned char* imgBuffer, CameraType type);

  // Get the color rectified image (right image only now)
  int getRectifiedColorBuffer(unsigned char* imgBuffer, CameraType type=BB_RIGHT);
  
  // Get the disparity image; each pixel is 16bits
  int getDisparityImage(unsigned short* img16Buffer, int* rowinc);
  
  // Set disparity range (max can be as high as 1024 but then there's this offset issue);
  // -- just keep the max below 240
  // -- rule of thumb: if closer objects aren't valid, try increasing max disparity value
  // -- rule of thumb: if image is too dark, try reducing max disparity value
  int setDisparity(int minDisp, int maxDisp);

  // Convert the (i,j) image 16-bit disparity value to an XYZ value
  int disparityToXYZ(int i, int j, unsigned short disparity, float* x, float* y, float* z);

  // Convert the (i,j) image float disparity value to an XYZ value
  int disparityToXYZ_f(float i, float j, float disparity, float* x, float* y, float* z);

  // Get focal length
  float getFocalLength();

  // Get image center
  int getImageCenter(float* centerRow, float* centerCol);

  // Get field of view
  void getFieldOfView(float *fov_horizontal, float *fov_vertical);

  // Get baseline
  void getBaseline(float *stereo_baseline);

  // Get shutter
  void getShutter(float *shutter);

  // get gain
  void getGain(float *gain);

  // Cleanup camera pointers and close
  int cleanup(dc1394camera_t* cam);

  // finish up and quit
  int fini();

  // check whether we have color camera or not
  bool isColor();

  // get the downscale value
  int getScale();

 private:

  // dc1394 camera object
  dc1394camera_t* camera;

  // dc1394 error object
  dc1394error_t err;

  // Triclops error object for stereo
  TriclopsError tri_err;
  
  // Triclops context object for stereo
  TriclopsContext triclops;

  // Triclops image for right rectified image
  TriclopsImage tri_image_right;
  
// Triclops image for left rectified image
  TriclopsImage tri_image_left;

  // Triclops image for disparity info (this is 16bits / pixel)
  TriclopsImage16 tri_image16;

  // Triclops packed color image
  TriclopsColorImage tri_color_image_right;

  // Triclops packed color image
  TriclopsColorImage tri_color_image_left;
  
  // Triclops input
  TriclopsInput input;

  // Triclops input
  TriclopsInput input_right;
  
// Triclops input
  TriclopsInput input_left;

  // PGR stereocam object
  PGRStereoCamera_t stereoCamera;

  // buffer to hold image
  unsigned int nBufferSize;
  
  // buffer to hold de-interleaved images
  unsigned char* pucDeInterlacedBuffer;
  
  // buffer for mono camera
  unsigned char* pucRightMono;
  unsigned char* pucLeftMono;
  unsigned char* pucCenterMono;

  // buffer for color camera
  unsigned char* pucRGBBuffer;
  unsigned char* pucGreenBuffer;
  unsigned char* pucRedGreenBlue;
  unsigned char* pucRightRGB;
  unsigned char* pucLeftRGB;
  unsigned char* pucCenterRGB;

  // Bumblebee2 unique ID
  int bumblebeeId;
  
  // min Disparity value (default 0)
  int minDisparity;

  // max Disparity value (default 240)
  int maxDisparity;

  // Calibration file name
  char calibrationfile[30];
  
  // focal length (for rectified image only)
  float focalLength;

  // image center (row)
  float imageCenterRow;
  
  // image center (col)
  float imageCenterCol;

  // horizontal field of view (for rectified image only)
  float hfov;

  // vertical field of view (for rectified image only)
  float vfov;

  // stereo baseline
  float baseline;

  // down scale by which to reduce the output image by; default is 2
  // NOTE: this only scales the rectified image and te disparity image!!! not the left/right image;
  int scale;

  // enable color capture of rectified images
  bool color;

  // timestamp
  uint64_t imagetimestamp;

};

#endif
