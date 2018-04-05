#include <opencv2/opencv.hpp>


using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
    if(argc!=4){
        cout<<"Usage: ../seg input_image initialization_file output_mask"<<endl;
        return -1;
    }
    
    // Load the input image
    // the image should be a 3 channel image by default but we will double check that in teh seam_carving
    Mat in_image;
    in_image = imread(argv[1]/*, CV_LOAD_IMAGE_COLOR*/);
   
    if(!in_image.data)
    {
        cout<<"Could not load input image!!!"<<endl;
        return -1;
    }

    if(in_image.channels()!=3){
        cout<<"Image does not have 3 channels!!! "<<in_image.depth()<<endl;
        return -1;
    }
    
    // the output image
    Mat out_image = in_image.clone();
    
    ifstream f(argv[2]);
    if(!f){
        cout<<"Could not load initial mask file!!!"<<endl;
        return -1;
    }
    
    int width = in_image.cols;
    int height = in_image.rows;
    
    int n;
    f>>n;
    
    // get the initil pixels
    for(int i=0;i<n;++i){
        int x, y, t;
        f>>x>>y>>t;
        
        if(x<0 || x>=width || y<0 || y>=height){
            cout<<"I valid pixel mask!"<<endl;
            return -1;
        }
        
        
        Vec3b pixel;
        pixel[0] = 0;
        pixel[1] = 0;
        pixel[2] = 0;
        
        if(t==1){
            pixel[2] = 255;
        } else {
            pixel[0] = 255;
        }
        
        out_image.at<Vec3b>(y, x) = pixel;
    }
    
    // write it on disk
    imwrite( argv[3], out_image);
    
    // also display them both
    
    namedWindow( "Original image", WINDOW_AUTOSIZE );
    namedWindow( "Show Marked Pixels", WINDOW_AUTOSIZE );
    imshow( "Original image", in_image );
    imshow( "Show Marked Pixels", out_image );
    waitKey(0);
    return 0;
}
