/*
 * This program grabs the left and right image (fast) from the bumblebee 
 * camera and displays it using opencv. There is no stereo processing 
 * done here which is why it is particularly fast. SIFT features are also
 * extracted and plotted for the right camera only.
 */

// include some standard header files
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// now include the opencv header files
#include <opencv/cv.h>
#include <opencv/highgui.h>

// finally, include the bumblebee header file
#include "bb2.h"

// include the SIFT header files
#include <sift/sift.h>
#include <sift/imgfeatures.h>
#include <sift/utils.h>
#include <sift/kdtree.h>
#include <sift/xform.h>

// this is the beginning of the "main" program
int main(int argc, char** argv)
{
  // what's the ID of the camera?
  int ID = 5020066;

  if(argc>1)
    ID = atoi(argv[1]);
  else
  {
    fprintf(stderr, "missing argument. Need model ID. Abort. \n");
    return -1;
  }

  // let's create the bumblebee object with some default parameters
  // - the 2 is for stereo downscaling (1 means full image stereo, 2
  //   means half image)
  // - the true is a boolean indicating we're using a color camera
  int scale = 2;
  bool color = true;
  BumbleBee bb(ID,scale,color); 
  
  // first let's initialize the bumblebee but stop the program if the
  // initialization fails 
  if(bb.init()<0)
    return(-1);  
  
  // what's the width and height of the camera? -- remember to scale it
  int width = bb.getImageWidth()/scale;
  int height = bb.getImageHeight()/scale; 
  
  // now let's create an opencv image container to hold the left &
  // right image
  IplImage *left = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);
  IplImage *right = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);

  // let's create a buffer to hold the disparity image
  unsigned short disparity_buffer[width*height];
  
  // let's create two windows to display the left and right images
  cvNamedWindow("Left",1);
  cvNamedWindow("Right",1);
  
  // now let's enter a while loop to continually capture from the
  // camera and display the image
  while(true)
  {
    // first thing we do is call the bumblebee to capture the images
    // and exit if the capture fails
    if(bb.capture()<0)
    {
      printf("capture failed!\n");
      break;
    }

    // if here, then the capture succeeded so let's grab the left and
    // right rectified images and place them into the opencv containers
    // we setup earlier
    bb.getRectifiedColorBuffer((unsigned char*)left->imageData, BB_LEFT);
    bb.getRectifiedColorBuffer((unsigned char*)right->imageData, BB_RIGHT);

    // lets grab the disparity buffer too along with the row size
    int rowinc;
    bb.getDisparityImage(disparity_buffer, &rowinc);
    
    // extract SIFT features for the small right image only, plot them,
    // then calculate the 3d point to the first feature then delete them
    // to avoid memory leak
    struct feature* current_features = NULL;
    int num_current_features = sift_features(right, &current_features);
    printf("detected %d features ... \n", num_current_features);
    for(int i=0; i<num_current_features; i++)
    {
      cvCircle( right,
                cvPoint( (int)current_features[i].img_pt.x,
                         (int)current_features[i].img_pt.y),  // feature point
                1,
                CV_RGB(255,0,0),
                2,
                8,
                0
                );

      // calculate distance to first feature and display it's XYZ in right
      // camera ref frame
      if(i==0)
      {
        int row, col;
        unsigned short disp;
        float x, y, z;
        row = (int)current_features[i].img_pt.y;
        col = (int)current_features[i].img_pt.x;
        disp = disparity_buffer[row*width + col];
        bb.disparityToXYZ(row, col, disp, &x, &y, &z);
        printf("feature 0 is at %f, %f, %f\n", x, y, z);
      }
    }
    free(current_features);
    
    // now let's display these captured images in the display windows we setup
    // earlier
    cvShowImage("Left", left);
    cvShowImage("Right", right);

    // now let's handle if a key was pressed, waiting up to 2 ms for it
    // if no key is pressed, then pressed_key < 0; otherwise, we exit on a
    // pressed key
    int pressed_key = cvWaitKey(3); 
    if(pressed_key>0)
    {
      break;      
    }
  }
  
  // cleanup, close, and finish the bumblebee camera
  bb.fini();

  // let's release those image containers we setup earlier
  cvReleaseImage(&right);   
  cvReleaseImage(&left);

  // finally, let's destroy those windows we setup to display the images
  cvDestroyWindow("Left");
  cvDestroyWindow("Right");

  return 0;
}

