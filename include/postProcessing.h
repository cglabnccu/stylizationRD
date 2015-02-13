#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class PP{
public:
	PP(Size);
	void	motionIllu(Mat &src,Mat &flowfield,Mat &dis);
	void	dirTexture(Mat &src,Mat &flowfield,Mat &dis); 
	void	dirTexture_Polar(Mat &src, Mat &flowfield, Mat &dis);
	void	adaThresholding(Mat &src,Mat &mask,Mat &dis);
	void	LIC(Mat &flowfield, Mat &dis);
	void	Thresholding(Mat &src, Mat &dis);
	void	Colormapping(Mat &src, Mat &mask, Mat &oriImg, Mat &dis);
	void	ReadTexture(string);

	Mat		texture;
	float	alpha;
	float	beta;
	bool	TextureLoaded;

private:
	void	SeedGradient(Mat &src, Mat &Gradient);
};