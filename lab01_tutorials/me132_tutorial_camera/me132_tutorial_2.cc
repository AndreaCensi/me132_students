/*
 * This program loads a reference image and test image and 
 * performs SIFT feature extraction on both. Features are
 * matched between to the two images and drawn using
 * OpenCV's draw commands.
 */

// include some standard header files
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// now include the opencv header files
#include <opencv/cv.h>
#include <opencv/highgui.h>

// include the SIFT header files
#include <sift/sift.h>
#include <sift/imgfeatures.h>
#include <sift/utils.h>
#include <sift/kdtree.h>
#include <sift/xform.h>

/* the maximum number of keypoint NN candidates to check during BBF search */
#define KDTREE_BBF_MAX_NN_CHKS     210
#define NN_SQ_DIST_RATIO_THR       0.30

// this is the beginning of the "main" program
int main(int argc, char** argv)
{
  // let's load the reference image into an opencv image container
  IplImage *reference_img = cvLoadImage("reference.png", CV_LOAD_IMAGE_GRAYSCALE);
  IplImage *test_img = cvLoadImage("test.png", CV_LOAD_IMAGE_GRAYSCALE);

  // extract features from the reference image and call these "database features"
  struct feature* database_features = NULL;
  fprintf(stderr, "extracting features from the reference image ... \n");
  int num_database_features = sift_features(reference_img, &database_features);

  // extract features from the test image and call these "current features"
  struct feature* current_features = NULL;
  fprintf(stderr, "extracting features from the test image ... \n");
  int num_current_features = sift_features(test_img, &current_features);

  // now build a kd-tree to hold the database features
  struct kd_node* kd_tree = NULL;
  kd_tree = kdtree_build(database_features, num_database_features);



  
  // create a display image that will be tiled with the reference image
  // in the top left and the test image in the bottom right and black everywhere else
  int display_width  = reference_img->width + test_img->width;
  int display_height = reference_img->height + test_img->height;
  IplImage *display = cvCreateImage(cvSize(display_width, display_height), IPL_DEPTH_8U, 3);
  cvZero(display);
  
  // let's create a window to display the loaded image
  cvNamedWindow("Display",1);

  // copy the reference image into the top left corner of the display image
  CvRect rect;
  rect.x = 0;
  rect.y = 0;
  rect.width = reference_img->width;
  rect.height = reference_img->height;
  cvSetImageROI(display, rect);
  cvCvtColor(reference_img, display, CV_GRAY2RGB);
  cvResetImageROI(display);

  // copy the test image into the bottom right corner of the display image
  rect.x = reference_img->width;
  rect.y = reference_img->height;
  rect.width = test_img->width;
  rect.height = test_img->height;
  cvSetImageROI(display, rect);
  cvCvtColor(test_img, display, CV_GRAY2RGB);
  cvResetImageROI(display);

  
  // draw the database features
  int i;
  for(i=0; i<num_database_features; i++)
  {
    cvCircle( display,
              cvPoint((int)database_features[i].img_pt.x, (int)database_features[i].img_pt.y),  // feature point
              1,           // radius of the circle
              CV_RGB(255,0,0),  // red circle
              2,                // thickness of circle 
              8,                // line type
              0                 // shift
              );
  }

  // draw the current features
  for(i=0; i<num_current_features; i++)
  {
    cvCircle( display,
              cvPoint( (int)current_features[i].img_pt.x + reference_img->width,
                       (int)current_features[i].img_pt.y + reference_img->height),  // feature point
              1,           // radius of the circle
              CV_RGB(0,255,255),  // red circle
              2,                // thickness of circle 
              8,                // line type
              0                 // shift
              );
  }

  // find the matches
  struct feature curr_feat;
  struct feature database_feat;
  struct feature** nbrs;
  double d0, d1;
  int k;
  for(i=0; i<num_current_features; i++)
  {
    curr_feat = current_features[i];
    k = kdtree_bbf_knn(  kd_tree,       // kd tree
                         &curr_feat,     // feature to match
                         2,             // number of interest neighbors to find
                         &nbrs,         // nearest neighbor features
                         KDTREE_BBF_MAX_NN_CHKS
                         );

    if( k == 2 )
    {
      d0 = descr_dist_sq( &curr_feat, nbrs[0] );
      d1 = descr_dist_sq( &curr_feat, nbrs[1] );
      if( d0 < d1 * NN_SQ_DIST_RATIO_THR )
      {
        // this database feature matches the current feature
        // curr_feat <==> database_feat
        database_feat = *nbrs[0];

        // draw the match as a line
        cvLine( display,
                cvPoint((int)database_feat.img_pt.x, (int)database_feat.img_pt.y), // database point from ref img
                cvPoint((int)curr_feat.img_pt.x + reference_img->width, (int)curr_feat.img_pt.y + reference_img->height), // current point from test img
                CV_RGB(0,255,0),
                1,   // thickness
                8,   // line type
                0    // shift
                );
      }      
    }
  }

  
  // now let's enter a while loop to continually show the image but exit
  // if a key is pressed
  while(true)
  {    
    // show the image
    cvShowImage("Display", display);

    // now let's handle if a key was pressed, waiting up to 2 ms for it
    // if no key is pressed, then pressed_key < 0; otherwise, we exit on a
    // pressed key
    int pressed_key = cvWaitKey(2);
    if(pressed_key > 0)
      break;
  }

  // free the display image we created
  cvReleaseImage(&display);
  
  // let destroy the window we setup to display the image
  cvDestroyWindow("Display");

  return 0;
}

