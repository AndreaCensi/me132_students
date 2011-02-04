/* 
 * This is a driver for the Bumblebee2 cameras. It incorporates 
 * files from the Point Grey Research website and is based largely
 * in part on their sample code. 
 */

#include "bb2.h"

// default constructor
BumbleBee::BumbleBee()
{
  // initialize some variables
  minDisparity = 0;
  maxDisparity = 240;
  scale = 2; //scale rectified and disparity image
  
  // by default, use 
  bumblebeeId = 6021014;
  color = false;
}

// loaded constructor
BumbleBee::BumbleBee(int bbId)
{
  // initialize some variables
  minDisparity = 0;
  maxDisparity = 240;
  scale = 2; //scale rectified and disparity image
  bumblebeeId = bbId;
  color = false;
}

// loaded constructor
BumbleBee::BumbleBee(int bbId, int downscale)
{
  // initialize some variables
  minDisparity = 0;
  maxDisparity = 240;
  scale = downscale;
  bumblebeeId = bbId;
  color = false;
}

// loaded constructor
BumbleBee::BumbleBee(int bbId, int downscale, bool enable_color)
{
  // initialize some variables
  minDisparity = 0;
  maxDisparity = 240;
  scale = downscale;
  bumblebeeId = bbId;
  color = enable_color;
}

// default destructor
BumbleBee::~BumbleBee()
{
  return;
}

// initialize stereocamera
int BumbleBee::init(float shutter)
{
  uint32_t nCameras;
  dc1394camera_t** cameras=NULL;

  err = dc1394_find_cameras( &cameras, &nCameras );
  
  if ( err != DC1394_SUCCESS ) 
    {
      fprintf( stderr, "Unable to look for cameras\n\n"
	       "Please check \n"
	       "  - if the kernel modules `ieee1394',`raw1394' and `ohci1394' "
	       "are loaded \n"
	       "  - if you have read/write access to /dev/raw1394\n\n");
      return(-1);
    }
   
   //  get the camera nodes and describe them as we find them
   if ( nCameras < 1 ) 
   {
     fprintf( stderr, "No cameras found!\n");
     return(-1);
   }
   
   unsigned int n;
   int serialNumber;
   for ( n=0; n < nCameras; n++ )
     { 
       serialNumber = (int)(cameras[n]->euid_64);
       if(bumblebeeId==serialNumber && isStereoCamera(cameras[n]))
	 {
	   camera = cameras[n];

	   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	   //Cleanup the ISO channels and freeup locked bandwidth
	   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	   err = dc1394_cleanup_iso_channels_and_bandwidth(camera);

	   printf( "Using this camera: %d\n",serialNumber);
	   break;
	 }       
     }

   if ( n == nCameras )
     {
       printf("n: %d  nCameras: %d\n", n, nCameras);
       printf("Camera %d not found. Abort.\n", bumblebeeId );
       return (-1);
     }
   
   // free the other cameras
   for ( unsigned int i = 0; i < nCameras; i++ )
     {
       if ( i != n )
	 dc1394_free_camera( cameras[i] );
     }
  
   // free the cameras object
   free(cameras);

   // try to initialize
   for(int i=0; i<5; i++)
     {
       if(init_try(shutter)<0)
	 {
	   dc1394_capture_stop( camera );
	   dc1394_video_set_transmission( camera , DC1394_OFF );
	 }
       else
	 break;
     }
   
   // load the configuration file for stereo processing
   sprintf(calibrationfile,"%d.cal",bumblebeeId);
   tri_err = triclopsGetDefaultContextFromFile( &triclops, calibrationfile);
   if ( tri_err != TriclopsErrorOk )
   {
      fprintf( stderr, "Can't get context from camera\n" );
      this->cleanup(camera);
      return (-1);
   }
  
   // make sure we are in subpixel mode
   triclopsSetSubpixelInterpolation( triclops, 1 );
   
   // Set output resolution of rectified image and disparity image-- could be smaller, just be sure to preserve 640x480 ratio;
   // NOTE: this only sets the resolution of the disparity image and the rectified image
   tri_err = triclopsSetResolution( triclops, stereoCamera.nRows/scale, stereoCamera.nCols/scale);
   if ( tri_err != TriclopsErrorOk )
     {
       fprintf( stderr, "triclopsSetResolution failed!\n" );
       triclopsDestroyContext( triclops );
       this->cleanup(camera);
       return (-1);
     }
   
   // grab focal length
   tri_err = triclopsGetFocalLength( triclops, &focalLength);
   if ( tri_err != TriclopsErrorOk )
     {
       fprintf( stderr, "triclopsGetFocalLength failed!\n" );
       triclopsDestroyContext( triclops );
       this->cleanup(camera);
       return (-1);
     }

   // grab image center
   tri_err = triclopsGetImageCenter( triclops, &imageCenterRow, &imageCenterCol);
   if ( tri_err != TriclopsErrorOk )
     {
       fprintf( stderr, "triclopsGetImageCenter failed!\n" );
       triclopsDestroyContext( triclops );
       this->cleanup(camera);
       return (-1);
     }

   // Set disparity values
   tri_err = triclopsSetDisparity( triclops, minDisparity, maxDisparity);
   if ( tri_err != TriclopsErrorOk )
     {
       fprintf( stderr, "triclopsSetResolution failed!\n" );
       triclopsDestroyContext( triclops );
       this->cleanup(camera);
       return (-1);
     }

   // specify buffer size and allocate memory
   nBufferSize = stereoCamera.nRows * stereoCamera.nCols * stereoCamera.nBytesPerPixel;
   pucDeInterlacedBuffer = new unsigned char[nBufferSize];

   if(stereoCamera.bColor)
     {
       pucRGBBuffer 	= new unsigned char[ 3 * nBufferSize ];
       pucGreenBuffer 	= new unsigned char[ nBufferSize ];
       pucRedGreenBlue  = new unsigned char[ 3 * nBufferSize ];
       pucRightRGB	= NULL;
       pucLeftRGB	= NULL;
       pucCenterRGB	= NULL;
     }
   else
     {
       pucRightMono = NULL;
       pucLeftMono = NULL;
       pucCenterMono = NULL;       
     }

   // calculate the horizontal field of view (rectified image only)
   float h = stereoCamera.nRows/scale;
   float w = stereoCamera.nCols/scale;
   this->hfov = 2*atan2(w, 2*this->focalLength );
   this->vfov = 2*atan2(h, 2*this->focalLength );
   
   // grab stereo baseline
   tri_err = triclopsGetBaseline( triclops, &baseline);
   if ( tri_err != TriclopsErrorOk )
     {
       fprintf( stderr, "triclopsGetStereoBaseline failed!\n" );
       triclopsDestroyContext( triclops );
       this->cleanup(camera);
       return (-1);
     }

   printf("hfov: %f [deg]   vfov: %f [deg]  baseline: %f\n",
	  this->hfov*180/M_PI, this->vfov*180/M_PI, this->baseline);

   // do a quick capture
   while(this->capture()<0)
     continue;


   return 0;   
}

// initialize sequence
int BumbleBee::init_try(float shutter)
{
  // query information about this stereo camera
  err = queryStereoCamera( camera, &stereoCamera );
  if ( err != DC1394_SUCCESS )
    {
      fprintf( stderr, "Cannot query all information from camera\n" );     
      return(-1);
    }
  
  // set the capture mode
  printf( "Setting stereo video capture mode\n" );
  err = setStereoVideoCapture( &stereoCamera );
  if ( err != DC1394_SUCCESS )
    {
      fprintf( stderr, "Could not set up video capture mode\n" );
      return(-1);
    }
  
  //set gain to auto
  dc1394_feature_set_mode( camera, DC1394_FEATURE_GAIN, DC1394_FEATURE_MODE_AUTO );
  
  // set shutter to manual  [minShut: 0.000003  maxShut: 0.020847]
  err = dc1394_feature_set_power( camera, DC1394_FEATURE_SHUTTER, DC1394_ON );
  dc1394_feature_set_mode( camera, DC1394_FEATURE_SHUTTER, DC1394_FEATURE_MODE_MANUAL );
  err = dc1394_feature_set_absolute_control( camera, DC1394_FEATURE_SHUTTER, DC1394_ON );
  err = dc1394_feature_set_absolute_value( camera, DC1394_FEATURE_SHUTTER, shutter);
    
  float s, g;
  err = dc1394_feature_get_absolute_value(camera, DC1394_FEATURE_SHUTTER, &s);
  err = dc1394_feature_get_absolute_value(camera, DC1394_FEATURE_GAIN, &g);
  printf("using this gain: %f    this shutter: %f\n", g, s);
  

  // have the camera start sending us data
  printf( "Start transmission\n" );
  err = startTransmission( &stereoCamera );
  if ( err != DC1394_SUCCESS ) 
   {
     fprintf( stderr, "Unable to start camera iso transmission\n" );
     return(-1);
   }

  return 0;
}

  
// capture images from camera
int BumbleBee::capture()
{
  int ret;
  fflush(stdout);
  // get the images from the capture buffer and do all required processing
  // note: produces a TriclopsInput that can be used for stereo processing
  if(stereoCamera.bColor)
    {
      grabColorImages_RGB( &stereoCamera,
			   DC1394_BAYER_METHOD_NEAREST,
			   pucDeInterlacedBuffer,
			   pucRGBBuffer,
			   pucRedGreenBlue,
			   &pucRightRGB,
			   &pucLeftRGB,
			   &pucCenterRGB,
			   &input,
			   &imagetimestamp);      
    }
  else
    {
      grabMonoImages( &stereoCamera,
		      pucDeInterlacedBuffer,
		      &pucRightMono,
		      &pucLeftMono,
		      &pucCenterMono,
		      &input,
		      &imagetimestamp);
    }

  // grab grayscale rectified images
  tri_err = triclopsRectify( triclops, &input );
  if ( tri_err != TriclopsErrorOk )
    {
      //fprintf( stderr, "triclopsRectify failed!\n" );      
      return (-1);
    }
  
  if(stereoCamera.bColor && color)
    {
      // grab color rectified images (right image only now)
      memcpy(&input_right, &input, sizeof(TriclopsInput));
      input_right.u.rgb.red   = pucRedGreenBlue;
      input_right.u.rgb.green = pucRedGreenBlue + 2 * stereoCamera.nRows * stereoCamera.nCols;
      input_right.u.rgb.blue  = pucRedGreenBlue + 4 * stereoCamera.nRows * stereoCamera.nCols;
      tri_err = triclopsRectifyColorImage(triclops,
					  TriCam_REFERENCE,
					  &input_right,
					  &tri_color_image_right);
            
      memcpy(&input_left, &input, sizeof(TriclopsInput));
      input_left.u.rgb.red   = pucRedGreenBlue + 1 * stereoCamera.nRows * stereoCamera.nCols;
      input_left.u.rgb.green = pucRedGreenBlue + 3 * stereoCamera.nRows * stereoCamera.nCols;
      input_left.u.rgb.blue  = pucRedGreenBlue + 5 * stereoCamera.nRows * stereoCamera.nCols;
      tri_err = triclopsRectifyColorImage(triclops,
					  TriCam_LEFT,
					  &input_left,
					  &tri_color_image_left);      
    }


  // now do stereo
  tri_err = triclopsStereo( triclops );
  if ( tri_err != TriclopsErrorOk )
    {
      //fprintf( stderr, "triclopsStereo failed!\n" );
      return (-1);
    }
  
   // grab the rectified and disparity images
   triclopsGetImage( triclops, TriImg_RECTIFIED, TriCam_RIGHT, &tri_image_right );
   triclopsGetImage( triclops, TriImg_RECTIFIED, TriCam_LEFT, &tri_image_left );
   triclopsGetImage16( triclops, TriImg16_DISPARITY, TriCam_REFERENCE, &tri_image16 );

  return 0;
}

// capture fast and save left, right, and input images w/o online stereo
int BumbleBee::captureBlob(StereoImageBlob* blob)
{
  // get the images from the capture buffer and do all required processing
  // note: produces a TriclopsInput that can be used for stereo processing
  int ret;
  if(stereoCamera.bColor)
    {
      ret = grabColorImages( &stereoCamera,
			     DC1394_BAYER_METHOD_NEAREST,
			     pucDeInterlacedBuffer,
			     pucRGBBuffer,
			     pucGreenBuffer,
			     &pucRightRGB,
			     &pucLeftRGB,
			     &pucCenterRGB,
			     &input,
			     &imagetimestamp);

      if(ret<0)
	return -1;

      // will this work?
      memcpy(&blob->left_buffer, pucLeftRGB, sizeof(unsigned char)*stereoCamera.nRows*stereoCamera.nCols*3);
      memcpy(&blob->right_buffer, pucRightRGB, sizeof(unsigned char)*stereoCamera.nRows*stereoCamera.nCols*3);      
    }
  else
    {
      ret = grabMonoImages( &stereoCamera,
			    pucDeInterlacedBuffer,
			    &pucRightMono,
			    &pucLeftMono,
			    &pucCenterMono,
			    &input,
			    &imagetimestamp);
      
      if(ret<0)
	return -1;
      
      memcpy(&blob->left_buffer, pucLeftMono, sizeof(unsigned char)*stereoCamera.nRows*stereoCamera.nCols);
      memcpy(&blob->right_buffer, pucRightMono, sizeof(unsigned char)*stereoCamera.nRows*stereoCamera.nCols);      
    }

  blob->timestamp = imagetimestamp;
  blob->cols = stereoCamera.nCols;
  blob->rows = stereoCamera.nRows;      
  blob->frameId++;
  this->getShutter(&blob->shutter);
  this->getGain(&blob->gain);
  
  return 0;
}

// capture the left and right images only
int BumbleBee::captureImageOnly()
{
  // get the images from the capture buffer and do all required processing
  // note: produces a TriclopsInput that can be used for stereo processing
  if(stereoCamera.bColor)
    {
      grabColorImages_RGB( &stereoCamera,
			   DC1394_BAYER_METHOD_NEAREST,
			   pucDeInterlacedBuffer,
			   pucRGBBuffer,
			   pucRedGreenBlue,
			   &pucRightRGB,
			   &pucLeftRGB,
			   &pucCenterRGB,
			   &input,
			   &imagetimestamp);
    }
  else
    {
      grabMonoImages( &stereoCamera,
		      pucDeInterlacedBuffer,
		      &pucRightMono,
		      &pucLeftMono,
		      &pucCenterMono,
		      &input,
		      &imagetimestamp);
    }
  
  // pre-process
  tri_err = triclopsSetLowpass( triclops, 1);
  tri_err = triclopsPreprocess( triclops, &input);
  if ( tri_err != TriclopsErrorOk )
    {
      fprintf( stderr, "triclopsPreprocess failed!\n" );
      triclopsDestroyContext( triclops );
      this->cleanup(camera);
      return (-1);
    }

  // grab grayscale rectified images
  tri_err = triclopsRectify( triclops, &input );
  if ( tri_err != TriclopsErrorOk )
    {
      fprintf( stderr, "triclopsRectify failed!\n" );
      triclopsDestroyContext( triclops );
      this->cleanup(camera);
      return (-1);
    }

  if(stereoCamera.bColor && color)
    {
      // grab color rectified image -- right image only now
      memcpy(&input_right, &input, sizeof(TriclopsInput));
      input_right.u.rgb.red   = pucRedGreenBlue;
      input_right.u.rgb.green = pucRedGreenBlue + 2 * stereoCamera.nRows * stereoCamera.nCols;
      input_right.u.rgb.blue  = pucRedGreenBlue + 4 * stereoCamera.nRows * stereoCamera.nCols;
      tri_err = triclopsRectifyColorImage(triclops,
					  TriCam_REFERENCE,
					  &input_right,
					  &tri_color_image_right);
      
      memcpy(&input_left, &input, sizeof(TriclopsInput));
      input_left.u.rgb.red   = pucRedGreenBlue + 1 * stereoCamera.nRows * stereoCamera.nCols;
      input_left.u.rgb.green = pucRedGreenBlue + 3 * stereoCamera.nRows * stereoCamera.nCols;
      input_left.u.rgb.blue  = pucRedGreenBlue + 5 * stereoCamera.nRows * stereoCamera.nCols;
      tri_err = triclopsRectifyColorImage(triclops,
					  TriCam_LEFT,
					  &input_left,
					  &tri_color_image_left);      
    }

  // grab the rectified image
  triclopsGetImage( triclops, TriImg_RECTIFIED, TriCam_RIGHT, &tri_image_right );
  triclopsGetImage( triclops, TriImg_RECTIFIED, TriCam_LEFT, &tri_image_left );

  return 0;
}

// capture the left and right images only
int BumbleBee::captureRawImageOnly()
{
  // get the images from the capture buffer and do all required processing
  // note: produces a TriclopsInput that can be used for stereo processing
  if(stereoCamera.bColor)
    {
      grabColorImages_RGB( &stereoCamera,
			   DC1394_BAYER_METHOD_NEAREST,
			   pucDeInterlacedBuffer,
			   pucRGBBuffer,
			   pucRedGreenBlue,
			   &pucRightRGB,
			   &pucLeftRGB,
			   &pucCenterRGB,
			   &input,
			   &imagetimestamp);
    }
  else
    {
      grabMonoImages( &stereoCamera,
		      pucDeInterlacedBuffer,
		      &pucRightMono,
		      &pucLeftMono,
		      &pucCenterMono,
		      &input,
		      &imagetimestamp);
    }

  return 0;
}

// return stereoimage width
unsigned int BumbleBee::getImageWidth()
{
  return stereoCamera.nCols;
}

// return stereoimage Height
unsigned int BumbleBee::getImageHeight()
{
  return stereoCamera.nRows;
}

// return rectified image width
int BumbleBee::getRectifiedWidth()
{
  int width;
  width = (int)tri_image_right.ncols;

  return width;
}

// return rectified image height
int BumbleBee::getRectifiedHeight()
{
  int height;
  height = (int)tri_image_right.nrows;

  return height;
}

// return the pixel value of the rectified image
uint8_t BumbleBee::getRectifiedPixel(int i, int j, CameraType type)
{
  if(type==BB_REFERENCE || type==BB_RIGHT)    
    return( (uint8_t)tri_image_right.data[ i * tri_image_right.rowinc + j]);    
  else    
    return( (uint8_t)tri_image_left.data[ i * tri_image_left.rowinc + j]);   
}

// return the pixel value of the rectified image
int BumbleBee::getRectifiedColorPixel(int i, int j, CameraType type, uint8_t* red, uint8_t* green, uint8_t* blue)
{
  if(!stereoCamera.bColor)
    return -1;

  if(!color)
    return -1;

  if(type==BB_LEFT)
    {
      *red = tri_color_image_left.red[i*tri_color_image_left.ncols + j];
      *green = tri_color_image_left.green[i*tri_color_image_left.ncols + j];
      *blue = tri_color_image_left.blue[i*tri_color_image_left.ncols + j];	
    }
  else
    {
      *red = tri_color_image_right.red[i*tri_color_image_right.ncols + j];
      *green = tri_color_image_right.green[i*tri_color_image_right.ncols + j];
      *blue = tri_color_image_right.blue[i*tri_color_image_right.ncols + j];	
    }

  return 0;
}


// return disparity image width
unsigned int BumbleBee::getDisparityWidth()
{
  return tri_image16.ncols;
}

// return disparity image height
unsigned int BumbleBee::getDisparityHeight()
{
  return tri_image16.nrows;
}

// return input width
unsigned int BumbleBee::getInputWidth()
{
  return input.ncols;
}

// return input Height
unsigned int BumbleBee::getInputHeight()
{
  return input.nrows;
}

// Get the left image from the stereocamera
int BumbleBee::getLeftImage(unsigned char* imgBuffer)
{
  if(stereoCamera.bColor)
    memcpy(imgBuffer, pucLeftRGB, sizeof(pucLeftRGB[0]) * stereoCamera.nCols * stereoCamera.nRows * 3);
  else
    memcpy(imgBuffer, pucLeftMono, sizeof(pucLeftMono[0]) * stereoCamera.nCols * stereoCamera.nRows);

  return 0;
}

// Get the right image from the stereocamera
int BumbleBee::getRightImage(unsigned char* imgBuffer)
{
  if(stereoCamera.bColor)
    memcpy(imgBuffer, pucRightRGB, sizeof(pucRightRGB[0]) * stereoCamera.nCols * stereoCamera.nRows * 3);
  else
    memcpy(imgBuffer, pucRightMono, sizeof(pucRightMono[0]) * stereoCamera.nCols * stereoCamera.nRows);
  
  return 0;
}

// Get the rectified image
int BumbleBee::getRectifiedImage(unsigned char* imgBuffer, CameraType type)
{
  if(type==BB_REFERENCE || type==BB_RIGHT)   
    memcpy(imgBuffer, tri_image_right.data, sizeof(unsigned char) * tri_image_right.ncols * tri_image_right.nrows);    
  else         
    memcpy(imgBuffer, tri_image_left.data, sizeof(unsigned char) * tri_image_left.ncols * tri_image_left.nrows);    
  return 0;
}

// Get the rectified color image -- this will be 4 channels
int BumbleBee::getRectifiedColorBuffer(unsigned char* imgBuffer, CameraType type)
{  
  if(!stereoCamera.bColor)
    return -1;

  if(!color)
    return -1;

  if(type==BB_RIGHT || type==BB_REFERENCE)
    {
      for(int i=0; i<tri_color_image_right.nrows; i++)
        for(int j=0; j<tri_color_image_right.ncols; j++)
        {
          imgBuffer[i*tri_color_image_right.ncols*3 + j*3 + 0] = tri_color_image_right.red[i*tri_color_image_right.ncols + j];
          imgBuffer[i*tri_color_image_right.ncols*3 + j*3 + 1] = tri_color_image_right.green[i*tri_color_image_right.ncols + j];
          imgBuffer[i*tri_color_image_right.ncols*3 + j*3 + 2] = tri_color_image_right.blue[i*tri_color_image_right.ncols + j];
        }
    }
  else
    {
      for(int i=0; i<tri_color_image_left.nrows; i++)
        for(int j=0; j<tri_color_image_left.ncols; j++)
        {
          imgBuffer[i*tri_color_image_left.ncols*3 + j*3 + 0] = tri_color_image_left.red[i*tri_color_image_left.ncols + j];
          imgBuffer[i*tri_color_image_left.ncols*3 + j*3 + 1] = tri_color_image_left.green[i*tri_color_image_left.ncols + j];
          imgBuffer[i*tri_color_image_left.ncols*3 + j*3 + 2] = tri_color_image_left.blue[i*tri_color_image_left.ncols + j];
        }
    }
      

  return 0;
}


// Get the disparity image
int BumbleBee::getDisparityImage(unsigned short* img16Buffer, int* rowinc)
{
  memcpy(img16Buffer, tri_image16.data, sizeof(unsigned short) * tri_image16.ncols * tri_image16.nrows);
  *rowinc = tri_image16.rowinc;
  return 0;
}

// Set disparity range (max can be as high as 1024 but then there's this offset issue;
// -- just keep the max below 240
// * function call must be made AFTER init()
int BumbleBee::setDisparity(int minDisp, int maxDisp)
{
  // Set disparity values
  tri_err = triclopsSetDisparity( triclops, minDisp, maxDisp);
  if ( tri_err != TriclopsErrorOk )
    {
      fprintf( stderr, "triclopsSetDisparity failed!\n" );
      triclopsDestroyContext( triclops );
      this->cleanup(camera);
      return (-1);
    }
  
  minDisparity = minDisp;
  maxDisparity = maxDisp;

  return 0;
}

// Convert the (i,j) image 16-bit disparity value to an XYZ value
int BumbleBee::disparityToXYZ(int i, int j, unsigned short disparity, float* x, float* y, float* z)
{
  tri_err = triclopsRCD16ToXYZ(triclops, i, j, disparity, x, y, z);
  if ( tri_err != TriclopsErrorOk )
    {
      fprintf( stderr, "triclopsRCD16ToXYZ failed!\n" );
      triclopsDestroyContext( triclops );
      this->cleanup(camera);
      return (-1);
    }

  return 0;
}

// Convert the (i,j) image float disparity value to an XYZ value
int BumbleBee::disparityToXYZ_f(float i, float j, float disparity, float* x, float* y, float* z)
{
  tri_err = triclopsRCDFloatToXYZ(triclops, i, j, disparity, x, y, z);
  if ( tri_err != TriclopsErrorOk )
    {
      fprintf( stderr, "triclopsRCD16ToXYZ failed!\n" );
      triclopsDestroyContext( triclops );
      this->cleanup(camera);
      return (-1);
    }

  return 0;
}

// Get focal length
float BumbleBee::getFocalLength()
{
  return(this->focalLength);
}

// Get image center
int BumbleBee::getImageCenter(float* centerRow, float* centerCol)
{
  *centerRow = this->imageCenterRow;
  *centerCol = this->imageCenterCol;

  return 0;
}

// Get field of view
void BumbleBee::getFieldOfView(float *fov_horizontal, float *fov_vertical)
{
  *fov_horizontal = this->hfov;
  *fov_vertical = this->vfov;
}

// Get baseline
void BumbleBee::getBaseline(float *stereo_baseline)
{
  *stereo_baseline = this->baseline;
}

// Get shutter
void BumbleBee::getShutter(float *shutter)
{
  float val;
  err = dc1394_feature_get_absolute_value(camera, DC1394_FEATURE_SHUTTER, &val);
  
  *shutter = val;

  return;
}

// get gain
void BumbleBee::getGain(float *gain)
{
  float val;
  err = dc1394_feature_get_absolute_value(camera, DC1394_FEATURE_GAIN, &val);
  
  *gain = val;

  return;
}



// Cleanup camera pointers and close
int BumbleBee::cleanup(dc1394camera_t* cam)
{
  dc1394_capture_stop( cam );
  dc1394_video_set_transmission( cam , DC1394_OFF );
  dc1394_free_camera( cam );
  return 0;
}

// finish up and quit
int BumbleBee::fini()
{
  //  Stop data transmission
  if ( dc1394_video_set_transmission( stereoCamera.camera, DC1394_OFF ) != DC1394_SUCCESS ) 
    {
      fprintf( stderr, "Couldn't stop the camera?\n" );
    }
  
  delete[] pucDeInterlacedBuffer;

  if(stereoCamera.bColor)
    {
      delete[] pucRGBBuffer;
      delete[] pucGreenBuffer;
      delete[] pucRedGreenBlue;
    }

  this->cleanup(camera);
  return 0;
}

// check whether we have color camera or not
bool BumbleBee::isColor()
{
  if(stereoCamera.bColor)
    return true;
  else
    return false;
}

// get the downscale value
int BumbleBee::getScale()
{
  return this->scale;
}





// -------------------------------
// not class member functions
// -------------------------------


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
		    uint64_t*            timestamp
		    )
{  
  dc1394video_frame_t* frame;
  unsigned char* pucGrabBuffer = NULL;
  
  while( dc1394_capture_dequeue( stereoCamera->camera,
				 DC1394_CAPTURE_POLICY_POLL,
				 &frame ) == DC1394_SUCCESS )
    {
      pucGrabBuffer = frame->image;
      *timestamp = frame->timestamp;
      // return buffer for use
      dc1394_capture_enqueue( stereoCamera->camera, frame );
    }
  
  if(pucGrabBuffer==NULL)
    {
      fprintf( stderr, "extractImagesColor - cannot dequeue image!\n" );
      return -1;
    }
  
  if ( stereoCamera->nBytesPerPixel == 2 )
    {
      // de-interlace the 16 bit data into 2 bayer tile pattern images
      dc1394_deinterlace_stereo( pucGrabBuffer,
				 pucDeInterleaved,
				 stereoCamera->nCols,
				 2*stereoCamera->nRows );
      // extract color from the bayer tile image
      // note: this will alias colors on the top and bottom rows
      dc1394_bayer_decoding_8bit( pucDeInterleaved,
				  pucRGB,
				  stereoCamera->nCols,
				  2*stereoCamera->nRows,
				  stereoCamera->bayerTile,
				  bayerMethod );
      // now deinterlace the RGB Buffer to extract the green channel
      // The green channel is a quick and dirty approximation to the mono
      // equivalent of the image and can be used for stereo processing
      dc1394_deinterlace_green( pucRGB,
				pucGreen,
				stereoCamera->nCols,
				6*stereoCamera->nRows );
      *ppucRightRGB 	= pucRGB;
      *ppucLeftRGB 	= pucRGB + 3 * stereoCamera->nRows * stereoCamera->nCols;
      *ppucCenterRGB	= *ppucLeftRGB;
    }
  else
    {
      dc1394_deinterlace_rgb( pucGrabBuffer,
			      pucDeInterleaved,
			      stereoCamera->nCols,
			      3*stereoCamera->nRows );
      // extract color from the bayer tile image
      // note: this will alias colors on the top and bottom rows
      dc1394_bayer_decoding_8bit( pucDeInterleaved,
				  pucRGB,
				  stereoCamera->nCols,
				  3*stereoCamera->nRows,
				  stereoCamera->bayerTile,
				  bayerMethod );
      // now deinterlace the RGB Buffer
      dc1394_deinterlace_green( pucRGB,
				pucGreen,
				stereoCamera->nCols,
				9*stereoCamera->nRows );
      // NOTE: this code needs to be double checked.
      // Currently 3-bytes-per-pixel is not activatable in this example
      *ppucRightRGB 	= pucRGB;
      *ppucCenterRGB 	= pucRGB + 3 * stereoCamera->nRows * stereoCamera->nCols;
      *ppucLeftRGB 	= pucRGB + 6 * stereoCamera->nRows * stereoCamera->nCols;
    }
  
  pTriclopsInput->inputType 	= TriInp_RGB;
  pTriclopsInput->nrows	= stereoCamera->nRows;
  pTriclopsInput->ncols	= stereoCamera->nCols;
  pTriclopsInput->rowinc	= stereoCamera->nCols;
  pTriclopsInput->u.rgb.red   	= pucGreen; 
  pTriclopsInput->u.rgb.green 	= pucGreen + stereoCamera->nRows * stereoCamera->nCols;
  pTriclopsInput->u.rgb.blue  	= pTriclopsInput->u.rgb.green;
  
  return 0;
}

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
			TriclopsInput*  	pTriclopsInput,
			uint64_t*            timestamp
			)
{
  dc1394video_frame_t* frame;
  unsigned char* pucGrabBuffer = NULL;
  
  while(dc1394_capture_dequeue( stereoCamera->camera,
				DC1394_CAPTURE_POLICY_POLL,
				&frame ) == DC1394_SUCCESS )
    {
      pucGrabBuffer = frame->image;
      *timestamp = frame->timestamp;
      // return buffer for use
      dc1394_capture_enqueue( stereoCamera->camera, frame );
    }
  
  if(pucGrabBuffer==NULL)
    return 0;
  
    
  
  // de-interlace the 16 bit data into 2 bayer tile pattern images
  dc1394_deinterlace_stereo( pucGrabBuffer,
			     pucDeInterleaved,
			     stereoCamera->nCols,
			     2*stereoCamera->nRows );
  // extract color from the bayer tile image
  // note: this will alias colors on the top and bottom rows
  dc1394_bayer_decoding_8bit( pucDeInterleaved,
			      pucRGB,
			      stereoCamera->nCols,
			      2*stereoCamera->nRows,
			      stereoCamera->bayerTile,
			      bayerMethod );
  // now deinterlace the RGB Buffer to extract the green channel
  // The green channel is a quick and dirty approximation to the mono
  // equivalent of the image and can be used for stereo processing
  dc1394_deinterlace_rgb( pucRGB,
			  pucRedGreenBlue,
			  stereoCamera->nCols,
			  6*stereoCamera->nRows);
  
  *ppucRightRGB	 = pucRGB;
  *ppucLeftRGB 	 = pucRGB + 3 * stereoCamera->nRows * stereoCamera->nCols;
  *ppucCenterRGB = *ppucLeftRGB;  

  pTriclopsInput->inputType 	= TriInp_RGB;
  pTriclopsInput->nrows 	= stereoCamera->nRows;
  pTriclopsInput->ncols	        = stereoCamera->nCols;
  pTriclopsInput->rowinc	= stereoCamera->nCols;
  pTriclopsInput->u.rgb.red     = pucRedGreenBlue + 2 * stereoCamera->nRows * stereoCamera->nCols; // right green
  pTriclopsInput->u.rgb.green   = pucRedGreenBlue + 3 * stereoCamera->nRows * stereoCamera->nCols; // left green
  pTriclopsInput->u.rgb.blue  	= pTriclopsInput->u.rgb.green;
   
  return 0;
}

// grab color image
int grabMonoImages(
		   PGRStereoCamera_t* 	stereoCamera, 
		   unsigned char* 	pucDeInterleaved,
		   unsigned char** 	ppucRightMono8,
		   unsigned char** 	ppucLeftMono8,
		   unsigned char** 	ppucCenterMono8,
		   TriclopsInput*  	pTriclopsInput,
		   uint64_t*            timestamp 
		   )
{
   dc1394video_frame_t* frame;
   unsigned char* pucGrabBuffer = NULL;

   while(dc1394_capture_dequeue( stereoCamera->camera,
				 DC1394_CAPTURE_POLICY_POLL,
				 &frame ) == DC1394_SUCCESS )
     {
       pucGrabBuffer = frame->image;
      *timestamp = frame->timestamp;
       // return buffer for use
       dc1394_capture_enqueue( stereoCamera->camera, frame );
     }
   
   if(pucGrabBuffer==NULL)
     return -1;

   //unsigned char* pucGrabBuffer = frame->image;   
   unsigned char* right;
   unsigned char* left;
   unsigned char* center;
   if ( stereoCamera->nBytesPerPixel == 2 )
     {
       // de-interlace the 16 bit data into 2 mono images
       dc1394_deinterlace_stereo( pucGrabBuffer,
				  pucDeInterleaved,
				  stereoCamera->nCols,
				  2*stereoCamera->nRows );
       right = pucDeInterleaved;
       left  = pucDeInterleaved + stereoCamera->nRows * stereoCamera->nCols;
       center= left;
     }
   else
     {
       dc1394_deinterlace_rgb( pucGrabBuffer,
			       pucDeInterleaved,
			       stereoCamera->nCols,
			       3*stereoCamera->nRows );
       
       // NOTE: this code needs to be double checked.
       // Currently 3-bytes-per-pixel is not activatable in this example
       right 	= pucDeInterleaved;
       center  	= pucDeInterleaved + stereoCamera->nRows * stereoCamera->nCols;
       left	= pucDeInterleaved + 2 * stereoCamera->nRows * stereoCamera->nCols;
     }
   
   *ppucRightMono8 	= right;
   *ppucLeftMono8 	= left;
   *ppucCenterMono8 	= center;
   pTriclopsInput->inputType 	= TriInp_RGB;
   pTriclopsInput->nrows	= stereoCamera->nRows;
   pTriclopsInput->ncols	= stereoCamera->nCols;
   pTriclopsInput->rowinc	= stereoCamera->nCols;
   pTriclopsInput->u.rgb.red   = right;
   pTriclopsInput->u.rgb.green = left;
   pTriclopsInput->u.rgb.blue  = left;

   return 0;
}
