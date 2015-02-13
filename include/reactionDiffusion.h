#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <amp.h>
#include <amp_math.h>

using namespace cv;
using namespace std;
using namespace concurrency;
using namespace concurrency::fast_math;

class PixelPattern{
public:
	PixelPattern();
	PixelPattern(int GrayScale, float F, float k, int l);
	int GrayScale;
	float F;
	float k; 
	int l;
};

class RD{
public:
	RD(Size);
	void Init(Size);
	void ReadSrc(string);
	void ReadFlow(string);
	void ETF(string);
	void ReadControlImg(string);
	void UpdateControlMask();
	void FastGrayScott();// Eq.6
	void FastGrayScott(float min_degree, float max_degree);// Eq.7
	void GrayScottModel();// generate GS-Model
	vector<PixelPattern> segmentation;
	Mat *c_A;        // Current element of A
	Mat *p_A;        // Previous element of A
	Mat *c_B;
	Mat *p_B;
	Mat Flowfield;
	Mat RotationMat;
	Mat Original_img;
	Mat Mask;
	Mat Mask_s;
	Mat Mask_control;
	Mat Mask_control_F;
	Mat Mask_control_k;
	Mat Mask_control_l;
	Mat Diffusion_A;
	Mat Diffusion_B;
	Mat Gradient_A; // Gradient vetor of A
	Mat Gradient_B; // Gradient vetor of B
	Mat Addition_A; // Addition A
	Mat Addition_B; // Addition B
	Mat alpha_A;    // alpha of A
	Mat alpha_B;    // alpha of B
	Mat A1;         // Element of A
	Mat A2;	           
	Mat B1;         // Element of B
	Mat B2;	           
	float s;        // Pattern size
	float v;        // Weight of advection term (speed)
	int l;          // Paramater of alpha, controls the edge number of the generated polygon pattern
	float f;        // Paramater of GS model
	float k;        // Paramater of GS model
	int theta0;     // Rotation angle
	float addA;     // Coefficient(weight) of A
	float addB;     // Coefficient(weight) of B
	bool SrcLoaded;
	bool FlowLoaded;
	bool ETFLoaded;
	bool ControlImgLoad;
};