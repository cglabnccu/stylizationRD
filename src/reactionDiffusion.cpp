﻿#include "reactionDiffusion.h"
#include <time.h>
# define M_PI 3.14159265358979323846

PixelPattern::PixelPattern()
{
	this->GrayScale = 0;
	this->F = 0;
	this->k = 0;
	this->l = 0;
	this->size = 0;
	this->sd = 0;
	this->CAF = false;
	this->dmax = 0;
	this->dmin = 0;
	this->theta0 = 0;
	this->regionPixelSize = 0;
}
PixelPattern::PixelPattern(int GrayScale, float F, float k, int l, float size, int dmin, int dmax)
{
	this->GrayScale = GrayScale;
	this->F = F;
	this->k = k;
	this->l = l;
	this->size = size;
	this->CAF = false;
	this->dmax = dmax;
	this->dmin = dmin;
	this->theta0 = 0;
	this->regionPixelSize = 0;
	this->sd = 0;
}

RD::RD()
{
	Size s = Size(256, 256);
	RotationMat = Mat::zeros(s, CV_32F);
	Flowfield = Mat::zeros(s, CV_32FC3);
	gvf = Mat::zeros(s, CV_32FC3);
	Mask = Mat::zeros(s, CV_32F);
	Mask_s = Mat::zeros(s, CV_32F);
	Mask_control = Mat::zeros(s, CV_32F);
	Mask_control_F = Mat::zeros(s, CV_32F);
	Mask_control_k = Mat::zeros(s, CV_32F);
	Mask_control_l = Mat::zeros(s, CV_32F);
	Mask_control_dmin = Mat::zeros(s, CV_32F);
	Mask_control_dmax = Mat::zeros(s, CV_32F);
	Mask_control_size = Mat::zeros(s, CV_32F);
	Mask_control_sd = Mat::zeros(s, CV_32F);
	Mask_control_theta0 = Mat::zeros(s, CV_32F);
	Gradient_A = Mat::zeros(s, CV_32FC3);
	Gradient_B = Mat::zeros(s, CV_32FC3);
	Diffusion_A = Mat::zeros(s, CV_32F);
	Diffusion_B = Mat::zeros(s, CV_32F);
	Addition_A = Mat::zeros(s, CV_32F);
	Addition_B = Mat::zeros(s, CV_32F);
	alpha_A = Mat::zeros(s, CV_32F);
	alpha_B = Mat::zeros(s, CV_32F);
	A1 = Mat::ones(s, CV_32F);
	A2 = Mat::ones(s, CV_32F);
	B1 = Mat::zeros(s, CV_32F);
	B2 = Mat::zeros(s, CV_32F);
	c_A = &A1;
	p_A = &A2;
	c_B = &B1;
	p_B = &B2;

	this->s = 0.6;
	this->sd = 0.3;
	this->v = 0.0;
	this->l = 1;
	this->f = 0.0375;
	this->k = 0.05775;
	this->theta0 = 0;
	this->addA = 0.5;
	this->addB = 0.5;

	SrcLoaded = false;
	FlowLoaded = false;
	ETFLoaded = false;
	ControlImgLoad = false;

	innerAMPloopsize = 4;
	UpdateSizeMask();
	UpdatekMask();
	isFlowGuide = false;
}


RD::RD(Size s)
{
	RotationMat = Mat::zeros(s, CV_32F);
	Flowfield = Mat::zeros(s, CV_32FC3);
	gvf = Mat::zeros(s, CV_32FC3);
	Mask = Mat::zeros(s, CV_32F);
	Mask_s = Mat::zeros(s, CV_32F);
	Mask_control = Mat::zeros(s, CV_32F);
	Mask_control_F = Mat::zeros(s, CV_32F);
	Mask_control_k = Mat::zeros(s, CV_32F);
	Mask_control_l = Mat::zeros(s, CV_32F);
	Mask_control_dmin = Mat::zeros(s, CV_32F);
	Mask_control_dmax = Mat::zeros(s, CV_32F);
	Mask_control_size = Mat::zeros(s, CV_32F);
	Mask_control_sd = Mat::zeros(s, CV_32F);
	Mask_control_theta0 = Mat::zeros(s, CV_32F);
	Gradient_A = Mat::zeros(s, CV_32FC3);
	Gradient_B = Mat::zeros(s, CV_32FC3);
	Diffusion_A = Mat::zeros(s, CV_32F);
	Diffusion_B = Mat::zeros(s, CV_32F);
	Addition_A = Mat::zeros(s, CV_32F);
	Addition_B = Mat::zeros(s, CV_32F);
	alpha_A = Mat::zeros(s, CV_32F);
	alpha_B = Mat::zeros(s, CV_32F);
	A1 = Mat::ones(s, CV_32F);
	A2 = Mat::ones(s, CV_32F);
	B1 = Mat::zeros(s, CV_32F);
	B2 = Mat::zeros(s, CV_32F);
	c_A = &A1;
	p_A = &A2;
	c_B = &B1;
	p_B = &B2;

	this->s = 0.6;
	this->sd = 0.3;
	this->v = 0.0;
	this->l = 1;
	this->f = 0.0375;
	this->k = 0.05775;
	this->theta0 = 0;
	this->addA = 0.5;
	this->addB = 0.5;

	SrcLoaded = false;
	FlowLoaded = false;
	ETFLoaded = false;
	ControlImgLoad = false;

	innerAMPloopsize = 4;
	UpdateSizeMask();
	UpdatekMask();
	isFlowGuide = false;
}

void RD::Init(Size s)
{
	RotationMat = Mat::zeros(s, CV_32F);
	resize(Flowfield, Flowfield, Mask.size(), 0, 0, CV_INTER_LINEAR);
	Gradient_A = Mat::zeros(s, CV_32FC3);
	Gradient_B = Mat::zeros(s, CV_32FC3);
	Diffusion_A = Mat::zeros(s, CV_32F);
	Diffusion_B = Mat::zeros(s, CV_32F);
	Addition_A = Mat::zeros(s, CV_32F);
	Addition_B = Mat::zeros(s, CV_32F);
	alpha_A = Mat::zeros(s, CV_32F);
	alpha_B = Mat::zeros(s, CV_32F);
	A1 = Mat::ones(s, CV_32F);
	A2 = Mat::ones(s, CV_32F);
	B1 = Mat::zeros(s, CV_32F);
	B2 = Mat::zeros(s, CV_32F);
	c_A = &A1;
	p_A = &A2;
	c_B = &B1;
	p_B = &B2;

	innerAMPloopsize = 4;
	UpdateSizeMask();
	isFlowGuide = false;
}

void RD::operator=(const RD &in)
{
	resize(Flowfield, Flowfield, Mask.size(), 0, 0, CV_INTER_LINEAR);
	RotationMat = in.RotationMat.clone();
	Gradient_A = in.Gradient_A.clone();
	Gradient_B = in.Gradient_B.clone();
	Diffusion_A = in.Diffusion_A.clone();
	Diffusion_B = in.Diffusion_B.clone();
	Addition_A = in.Addition_A.clone();
	Addition_B = in.Addition_B.clone();
	alpha_A = in.alpha_A.clone();
	alpha_B = in.alpha_B.clone();
	A1 = in.A1.clone();
	A2 = in.A2.clone();
	B1 = in.B1.clone();
	B2 = in.B2.clone();
	c_A = &A1;
	p_A = &A2;
	c_B = &B1;
	p_B = &B2;
	isFlowGuide = in.isFlowGuide;

	UpdateSizeMask();

}

void RD::SaveRD(string filepath) 
{
	cv::FileStorage storage(filepath, cv::FileStorage::WRITE);
	storage << "RotationMat" << RotationMat;
	storage << "Gradient_A" << Gradient_A;
	storage << "Gradient_B" << Gradient_B;
	storage << "Diffusion_A" << Diffusion_A;
	storage << "Diffusion_B" << Diffusion_B;
	storage << "Addition_A" << Addition_A;
	storage << "Addition_B" << Addition_B;
	storage << "alpha_A" << alpha_A;
	storage << "alpha_B" << alpha_B;
	storage << "A1" << A1;
	storage << "A2" << A2;
	storage << "B1" << B1;
	storage << "B2" << B2;
	storage.release();

}

void RD::ReadRD(string filepath)
{
	cv::FileStorage storage(filepath, cv::FileStorage::READ);
	storage["RotationMat"] >> RotationMat;
	storage["Gradient_A"] >> Gradient_A;
	storage["Gradient_B"] >> Gradient_B;
	storage["Diffusion_A"] >> Diffusion_A;
	storage["Diffusion_B"] >> Diffusion_B;
	storage["Addition_A"] >> Addition_A;
	storage["Addition_B"] >> Addition_B;
	storage["alpha_A"] >> alpha_A;
	storage["alpha_B"] >> alpha_B;
	storage["A1"] >> A1;
	storage["A2"] >> A2;
	storage["B1"] >> B1;
	storage["B2"] >> B2;
	storage.release();

}


void RD::ReadSrc(string file)
{
	Original_img = imread(file, CV_LOAD_IMAGE_COLOR);
	//imshow("mjk,", Original_img);

	Mask = imread(file, 0);
	Init(Mask.size());
	Mask.convertTo(Mask, CV_32FC1);
	normalize(Mask, Mask, 0.0, 1.0, NORM_MINMAX, CV_32FC1);
	/// Generate grad_x and grad_y
	Mat grad_x, grad_y;
	/// Gradient X
	Sobel(Mask, grad_x, CV_32F, 1, 0, 3, 1, 0, 1);
	/// Gradient Y
	Sobel(Mask, grad_y, CV_32F, 0, 1, 3, 1, 0, 1);
	/// Total Gradient (approximate)
	addWeighted(abs(grad_x), 0.5, abs(grad_y), 0.5, 0, Mask_s);
	normalize(Mask_s, Mask_s, 0.0, 1.0, NORM_MINMAX);
	GaussianBlur(Mask, Mask, Size(9, 9), 0, 0);

	resize(Mask_control, Mask_control, Mask.size(), 0, 0, CV_INTER_LINEAR);

	//imshow("M",Mask);
	//imshow("MS",Mask_s);
	SrcLoaded = true;
	resize(Mask_control, Mask_control, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_F, Mask_control_F, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_k, Mask_control_k, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_l, Mask_control_l, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_size, Mask_control_size, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_sd, Mask_control_sd, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_dmin, Mask_control_dmin, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_dmax, Mask_control_dmax, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_theta0, Mask_control_theta0, Mask.size(), 0, 0, CV_INTER_LINEAR);
}

void RD::ReadFlow(string file)
{
	FILE *stream = fopen(file.c_str(), "rb");
	if (!stream)
	{
		//std::cout<<"ERROR!! Can't read "<<featurefile<<'\n';
		return;
	}

	int vf_w, vf_h;
	int sint = sizeof(int);

	fread((void *)&(vf_w), sint, 1, stream);
	fread((void *)&(vf_h), sint, 1, stream);

	int sfloat = sizeof(float);
	float *data = new float[vf_w*vf_h * 2];
	fread((void *)(data), sfloat, vf_w*vf_h * 2, stream);

	resize(Flowfield, Flowfield, Size(vf_w, vf_h), 0, 0, CV_INTER_LINEAR);
	//for (int i =0; i<vf_w; i++){
	//	for (int j = 0; j<vf_h; j++){
	//		Flowfield.at<Vec3f>(i,j) = Vec3f(data[(i*vf_h+j)*2+1],data[(i*vf_h+j)*2],0.0);
	//	}
	//}

	for (int j = 0; j < vf_h; j++)
	{
		for (int i = 0; i < vf_w; i++)
		{
			int index = j*vf_w + i;
			float dx = data[index * 2 + 1];
			float dy = data[index * 2];
			Flowfield.at<Vec3f>(vf_h - j - 1, i) = Vec3f(dx, -dy, 0.0); //x,y swap??

		}
	}
	resize(Flowfield, Flowfield, Mask.size(), 0, 0, CV_INTER_LINEAR);

	FlowLoaded = true;
	ETFLoaded = false;
}

//Generate ETF of input image as flowfield
void RD::ETF(string file)
{
	Mat src = imread(file, 1);
	Mat src_n;
	Mat grad;
	normalize(src, src_n, 0.0, 1.0, NORM_MINMAX, CV_32FC3);
	GaussianBlur(src_n, src_n, Size(91, 91), 0, 0);

	/// Generate grad_x and grad_y
	Mat grad_x, grad_y;
	/// Gradient X
	Sobel(src_n, grad_x, CV_32F, 1, 0, 3, 1, 0, 1);
	/// Gradient Y
	Sobel(src_n, grad_y, CV_32F, 0, 1, 3, 1, 0, 1);
	//GaussianBlur( grad_x, grad_x, Size( 51, 51 ), 0, 0 );
	//GaussianBlur( grad_y, grad_y, Size( 51, 51 ), 0, 0 );
	//imshow("x", grad_x);
	//imshow("y", grad_y);

	Flowfield = Mat::zeros(src.size(), CV_32FC3);
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			Vec3f u = grad_x.at<cv::Vec3f>(i, j) / 255.0; //-255~255
			Vec3f v = grad_y.at<cv::Vec3f>(i, j) / 255.0;

			float x = u.dot(u);
			float y = v.dot(v);
			float z = v.dot(u);
			float temp = y*y - 2.0*x*y + x*x + 4.0*z*z;
			float lambda1 = 0;
			lambda1 = 0.5 * (y + x + sqrt(temp));
			Flowfield.at<cv::Vec3f>(i, j) = normalize(Vec3f(z, x - lambda1, 0.0));
			//dis.at<cv::Vec3f>(i,j) = normalize( Vec3f(lambda1-x,z,0.0) );

			if (Flowfield.at<cv::Vec3f>(i, j) == Vec3f(0.0, 0.0, 0.0))
			{
				Flowfield.at<cv::Vec3f>(i, j) = Vec3f(0.0, 1.0, 0.0);
			}
		}
	}

	resize(Flowfield, Flowfield, Mask.size(), 0, 0, CV_INTER_LINEAR);

	ETFLoaded = true;
	FlowLoaded = false;
}

void RD::GVF()
{
	Mat tmp_n = Mat::zeros(Size(256, 256), CV_32FC3);

	//Addition_B is CV_32F ,but we need CV_32FC3 to store vec3f
	//create 3 same CV_32F channel and merge to create CV_32FC3 Mat
	vector<Mat> channels;
	Mat c = Mat::zeros(tmp_n.size(), CV_32F);
	Addition_B.convertTo(c, CV_32F, 255);
	channels.push_back(c);
	channels.push_back(c);
	channels.push_back(c);
	merge(channels, tmp_n);

	normalize(tmp_n, tmp_n, 0.0, 1.0, NORM_MINMAX, CV_32FC3);
	GaussianBlur(tmp_n, tmp_n, Size(91, 91), 0, 0);

	/// Generate grad_x and grad_y
	Mat dX, dY;
	Sobel(tmp_n, dX, CV_32F, 1, 0, 3, 1, 0, 1);
	Sobel(tmp_n, dY, CV_32F, 0, 1, 3, 1, 0, 1);

	gvf = Mat::zeros(tmp_n.size(), CV_32FC3);
	for (int i = 0; i < tmp_n.rows; i++)
	{
		for (int j = 0; j < tmp_n.cols; j++)
		{
			Vec3f u = dX.at<cv::Vec3f>(i, j) / 255.0; //-255~255
			Vec3f v = dY.at<cv::Vec3f>(i, j) / 255.0;

			float x = u.dot(u);
			float y = v.dot(v);
			float z = v.dot(u);
			float temp = y*y - 2.0*x*y + x*x + 4.0*z*z;
			float lambda1 = 0;
			lambda1 = 0.5 * (y + x + sqrt(temp));
			gvf.at<cv::Vec3f>(i, j) = normalize(Vec3f(z,x - lambda1, 0.0));

			if (gvf.at<cv::Vec3f>(i, j) == Vec3f(0.0, 0.0, 0.0))
			{
				gvf.at<cv::Vec3f>(i, j) = Vec3f(0.0, 1.0, 0.0);
			}
		}
	}

	for (int i = 0; i < gvf.rows; i++)
	{
		for (int j = 0; j < gvf.cols; j++)
		{
			Vec3f v = gvf.at<cv::Vec3f>(i, j);
			gvf.at<cv::Vec3f>(i, j) = Vec3f(-v[1], v[0], 0.0);
		}
	}
	resize(gvf, gvf, Mask.size(), 0, 0, CV_INTER_LINEAR);
	Flowfield = gvf.clone();

}

void RD::RotateFlow(float theta)
{
	theta = theta / 180.0 * M_PI;
	for (int i = 0; i < Flowfield.rows; i++)
	{
		for (int j = 0; j < Flowfield.cols; j++)
		{
			Vec3f v = Flowfield.at<cv::Vec3f>(i, j);
			// x' = x*cos(Theta) - y*sin(Theta)
			// y' = y*cos(Theta) + x*sin(Theta)
			float rx = v[0] * cos(theta) - v[1] * sin(theta);
			float ry = v[1] * cos(theta) + v[0] * sin(theta);
			Flowfield.at<cv::Vec3f>(i, j) = Vec3f(rx, ry, 0.0);
		}
	}

}

void RD::ReadControlImg(string file)
{
	Mask_control = imread(file, 0);

	resize(Mask_control, Mask_control, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_F, Mask_control_F, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_k, Mask_control_k, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_l, Mask_control_l, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_size, Mask_control_size, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_dmin, Mask_control_dmin, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_dmax, Mask_control_dmax, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_theta0, Mask_control_theta0, Mask.size(), 0, 0, CV_INTER_LINEAR);
	ControlImgLoad = true;
	//imshow("loil", Mask_control);

	const int Tolerate = 5;
	segmentation.clear();
	for (int i = 0; i < Mask_control.rows; i++)
	{
		for (int j = 0; j < Mask_control.cols; j++)
		{
			int n = Mask_control.at<uchar>(i, j);
			PixelPattern pixel(n, 0.0375, 0.0655, 1, 0.3, 0, 0);
			if (segmentation.empty())
			{
				segmentation.push_back(pixel);
			}
			else
			{
				bool haved = false;
				for (int u = 0; u < segmentation.size(); u++)
				{
					if ((segmentation[u].GrayScale >= pixel.GrayScale - Tolerate) &&
						(segmentation[u].GrayScale <= pixel.GrayScale + Tolerate))
					{
						haved = true;
						segmentation[u].regionPixelSize++;
						break;
					}
				}
				if (!haved)
					segmentation.push_back(pixel);
			}
		}
	}
	// discard region that too small
	int Img_size = Mask_control.rows * Mask_control.cols;
	float thresholding = 0.01; // the region that smaller than 0.01 * Img size  will be discard
	for (int i = 0; i < segmentation.size(); i++)
	{
		if (segmentation[i].regionPixelSize < Img_size*0.01)
		{
			segmentation.erase(segmentation.begin() + i);
			i--;
		}
	}
	UpdateControlMask();
}

void RD::ReadSizeControlImg(string file)
{
	Mat sizeImg;
	sizeImg = imread(file, 0);

	resize(sizeImg, sizeImg, Mask.size(), 0, 0, CV_INTER_LINEAR);
	resize(Mask_control_size, Mask_control_size, Mask.size(), 0, 0, CV_INTER_LINEAR);

	for (int i = 0; i < Mask_control_size.rows; i++)
	{
		for (int j = 0; j < Mask_control_size.cols; j++)
		{
			int n = sizeImg.at<uchar>(i, j);
			Mask_control_size.at<float>(i, j) = max((float)n / 255.0 , 0.1);
		}
	}

}

void RD::Gradient(Point start, Point end, string type, string target)
{
	Mat tmp = Mat::zeros(Mask.size(), CV_32F);

	float guideLine_length = sqrt((double)(pow(start.x - end.x, 2) + (double)pow(start.y - end.y, 2)));
	if (type == "Circular" || type == "Inverse Circular")
	{
		// Circular gradient startPoint ----> endPoint
		// Pattern Size:         Big     ---->  Small 
		for (int i = 0; i < Mask_control.rows; i++)
		{
			for (int j = 0; j < Mask_control.cols; j++)
			{
				double distace_to_Central = sqrt((double)(pow(start.x - j, 2) + (double)pow(start.y - i, 2)));
				if (type == "Circular")
				{
					if (distace_to_Central > guideLine_length) 
					{
						if(target == "size") tmp.at<float>(i, j) = 1.0;//Smallest s = 0.0
						else if (target == "k") tmp.at<float>(i, j) = 0.066;// white
					} 
					else 
					{ 
						if (target == "size") tmp.at<float>(i, j) = abs(distace_to_Central) / guideLine_length;
						else if (target == "k") tmp.at<float>(i, j) = 0.056 + (float)(abs(distace_to_Central) / guideLine_length) / 100.0;
					}
				}
				else //Inverse Circular
				{
					if (distace_to_Central > guideLine_length)
					{ 
						if (target == "size") tmp.at<float>(i, j) = 0.1;//Biggest s = 0.9
						else if (target == "k") tmp.at<float>(i, j) = 0.056;
					} 
					else
					{ 
						if (target == "size") tmp.at<float>(i, j) = 1 - abs(distace_to_Central) / guideLine_length + 0.1;
						else if (target == "k") tmp.at<float>(i, j) = 0.066 - (float)(abs(distace_to_Central) / (guideLine_length*100.0));
					}
				}
			}
		}

	}
	else if (type == "Linear")
	{
		bool isVertical = false, isUp_to_Down = false;
		float guide_line_m, prependicular_m;

		//opencv Point (0, 0) is upperleft
		if (start.y > end.y) { isUp_to_Down = false; }
		else if (start.y < end.y) { isUp_to_Down = true; }
		else { prependicular_m = 0; }

		if (start.x == end.x) { isVertical = true; }

		if (!isVertical && start.x != end.x)
		{
			guide_line_m = (float)((Mask_control.rows - end.y) - (Mask_control.rows - start.y)) / (float)(end.x - start.x);
			prependicular_m = -1.0 / guide_line_m;
		}

		// Two Line Cut the drawpanel to three region
		// line1: start.y - Y = m(start.x - X)
		// line2: end.y - Y = m(end.x - X)
		for (int i = 0; i < Mask_control.rows; i++)
		{
			for (int j = 0; j < Mask_control.cols; j++)
			{
				double distace_to_startLine = distance_to_line(Point(start.x - 1, Mask_control.rows - start.y - prependicular_m),
					Point(start.x + 1, Mask_control.rows - start.y + prependicular_m),
					Point(j, Mask_control.rows - i));
				double distace_to_endLine = distance_to_line(Point(end.x - 1, Mask_control.rows - end.y - prependicular_m),
					Point(end.x + 1, Mask_control.rows - end.y + prependicular_m),
					Point(j, Mask_control.rows - i));
				if (isUp_to_Down)
				{
					if (distace_to_startLine < 0) 
					{
						if(target == "size") tmp.at<float>(i, j) = 0.1;  //Biggest s = 0.9
						else if (target == "k") tmp.at<float>(i, j) = 0.056;
					}
					else if (distace_to_endLine > 0) 
					{
						if (target == "size") tmp.at<float>(i, j) = 1.0; //Smallest s = 0.0
						else if (target == "k") tmp.at<float>(i, j) = 0.066;
					} 
					else 
					{ 
						if (target == "size") tmp.at<float>(i, j) = min(1, 1 - abs(distace_to_endLine) / guideLine_length + 0.1);
						else if (target == "k") tmp.at<float>(i, j) = 0.066 - (abs(distace_to_endLine) / guideLine_length / 100.0);
					}
				}
				else
				{
					//if (distace_to_startLine > 0) { Mask_control_size.at<float>(i, j) = 0.1; }
					//else if (distace_to_endLine < 0) { Mask_control_size.at<float>(i, j) = 1.0; }
					//else { Mask_control_size.at<float>(i, j) = min(1, 1 - abs(distace_to_endLine) / guideLine_length + 0.1); }
					if (distace_to_startLine > 0)
					{
						if (target == "size") tmp.at<float>(i, j) = 0.1;  //Biggest s = 0.9
						else if (target == "k") tmp.at<float>(i, j) = 0.056;
					}
					else if (distace_to_endLine < 0)
					{
						if (target == "size") tmp.at<float>(i, j) = 1.0; //Smallest s = 0.0
						else if (target == "k") tmp.at<float>(i, j) = 0.066;
					}
					else
					{
						if (target == "size") tmp.at<float>(i, j) = min(1, 1 - abs(distace_to_endLine) / guideLine_length + 0.1);
						else if (target == "k") tmp.at<float>(i, j) = 0.066 - (abs(distace_to_endLine) / guideLine_length / 100.0);
					}
				}
			}
		}
	}
	if (target == "size") Mask_control_size = tmp.clone();
	else if (target == "k") Mask_control_k = tmp.clone();
}

double RD::distance_to_line(Point line_start, Point line_end, Point point)
{
	double normalLength = _hypot(line_end.x - line_start.x, line_end.y - line_start.y);
	double distance = (double)((point.x - line_start.x) * (line_end.y - line_start.y) - (point.y - line_start.y) * (line_end.x - line_start.x)) / normalLength;
	return distance;
}

void RD::UpdateSizeMask()
{
	for (int i = 0; i < Mask_control.rows; i++)
	{
		for (int j = 0; j < Mask_control.cols; j++)
		{
			Mask_control_size.at<float>(i, j) = 1 - s;
		}
	}
}

void RD::UpdatekMask()
{
	for (int i = 0; i < Mask_control.rows; i++)
	{
		for (int j = 0; j < Mask_control.cols; j++)
		{
			Mask_control_k.at<float>(i, j) = k;
		}
	}
}

void RD::UpdateControlMask()
{
	const int Tolerate = 5;
	for (int i = 0; i < Mask_control.rows; i++)
	{
		for (int j = 0; j < Mask_control.cols; j++)
		{
			int n = Mask_control.at<uchar>(i, j);
			for (int num = 0; num < segmentation.size(); num++)
			{
				if ((n + Tolerate >= segmentation[num].GrayScale) &&
					(n - Tolerate <= segmentation[num].GrayScale))
				{
					Mask_control_F.at<float>(i, j) = segmentation[num].F;
					Mask_control_k.at<float>(i, j) = segmentation[num].k;
					Mask_control_l.at<float>(i, j) = segmentation[num].l;
					Mask_control_size.at<float>(i, j) = 1 - segmentation[num].size;
					Mask_control_sd.at<float>(i, j) =0.7+ segmentation[num].sd;
					Mask_control_dmin.at<float>(i, j) = (float)segmentation[num].dmin / 180.0 * M_PI;
					Mask_control_dmax.at<float>(i, j) = (float)segmentation[num].dmax / 180.0 * M_PI;
					Mask_control_theta0.at<float>(i, j) = (float)segmentation[num].theta0 / 180.0 * M_PI;
				}
			}
		}
	}
}

void RD::DrawHistogram(Mat &A, Mat &B){

	Mat srcA, srcB;
	A.convertTo(srcA, CV_8UC1, 255);
	B.convertTo(srcB, CV_8UC1, 255);

	/// Establish the number of bins
	int histSize = 256;

	/// Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 };
	const float* histRange = { range };
	Mat histA;
	Mat histB;
	bool uniform = true; bool accumulate = false;
	/// Compute the histograms:
	calcHist(&srcA, 1, 0, Mat(), histA, 1, &histSize, &histRange, uniform, accumulate);
	calcHist(&srcB, 1, 0, Mat(), histB, 1, &histSize, &histRange, uniform, accumulate);

	// Draw the histograms for B, G and R
	int hist_w = 512; int hist_h = 400;
	int bin_w = cvRound((double)hist_w / histSize);

	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));


	/// Normalize the result to [ 0, histImage.rows ]
	normalize(histA, histA, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	normalize(histB, histB, 0, histImage.rows, NORM_MINMAX, -1, Mat());

	for (int i = 1; i < histSize; i++)
	{
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(histA.at<float>(i - 1))),
			Point(bin_w*(i), hist_h - cvRound(histA.at<float>(i))),
			Scalar(0, 0, 255), 2, 8, 0);
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(histB.at<float>(i - 1))),
			Point(bin_w*(i), hist_h - cvRound(histB.at<float>(i))),
			Scalar(0, 255, 0), 2, 8, 0);
	}
	/// Display
	namedWindow("Histogram of A and B", CV_WINDOW_AUTOSIZE);
	imshow("Histogram of A and B", histImage);
}

// add checkboard mask outside regionindex
void RD::DisplaySeg(Mat &dis, int regionindex)
{
	const int Tolerate = 5;
	int regionGray = segmentation[regionindex].GrayScale;

	for (int i = 0; i < Mask_control.rows; i++)
	{
		for (int j = 0; j < Mask_control.cols; j++)
		{
			int Gray = Mask_control.at<uchar>(i, j);
			if ((Gray >= regionGray - Tolerate) && (Gray <= regionGray + Tolerate));
			else
			{
				int k = (j / 3) % 2;
				int l = (i / 3) % 2;
				if ((k == 0 && l == 0) || (k == 1 && l == 1))
					dis.at<float>(i, j) = 0.5;
				else
					dis.at<float>(i, j) = 1.0;
			}
		}
	}
}

// for Eq.6(Anisotropic) and Eq.7(custom AnisotropicFunction)
int RD::FastGrayScott(float min_degree, float max_degree, bool isCAF, bool segmentOn)
{
	clock_t	Start_Time = clock();
	
	int nRows = c_A->rows;
	int nCols = c_A->cols;
	bool ControlImgLoad = this->ControlImgLoad;

	array_view< float, 1 > p_flowfield(nRows*nCols * 3, (float*)Flowfield.data);

	array_view< float, 1 > alpha_A(nRows*nCols, (float*)alpha_A.data);
	//array_view< float, 1 > alpha_B( nRows*nCols, (float*)alpha_B.data );
	array_view< float, 1 > c_A(nRows*nCols, (float*)c_A->data);
	array_view< float, 1 > c_B(nRows*nCols, (float*)c_B->data);
	array_view< float, 1 > p_A(nRows*nCols, (float*)p_A->data);
	array_view< float, 1 > p_B(nRows*nCols, (float*)p_B->data);
	array_view< const float, 1 > a_A(nRows*nCols, (float*)Addition_A.data);
	array_view< const float, 1 > a_B(nRows*nCols, (float*)Addition_B.data);
	array_view< const float, 1 > m(nRows*nCols, (float*)Mask.data);
	array_view< const float, 1 > m_s(nRows*nCols, (float*)Mask_s.data);


	//Mat tmp_Mask_control;
	//normalize(Mask_control, tmp_Mask_control, 0.0, 1.0, NORM_MINMAX, CV_8U);
	array_view<const float, 1> m_control(nRows*nCols, (float*)Mask_control.data); // control img
	array_view<const float, 1> m_control_F(nRows*nCols, (float*)Mask_control_F.data); // control img - F
	array_view<const float, 1> m_control_k(nRows*nCols, (float*)Mask_control_k.data); // control img - k
	array_view<const float, 1> m_control_l(nRows*nCols, (float*)Mask_control_l.data); // control img - l
	array_view<const float, 1> m_control_size(nRows*nCols, (float*)Mask_control_size.data); // control img - size
	array_view<const float, 1> m_control_sd(nRows*nCols, (float*)Mask_control_sd.data); // control img - sd
	array_view<const float, 1> m_control_dmin(nRows*nCols, (float*)Mask_control_dmin.data); // control img - dmin
	array_view<const float, 1> m_control_dmax(nRows*nCols, (float*)Mask_control_dmax.data); // control img - dmax
	array_view<const float, 1> m_control_theta0(nRows*nCols, (float*)Mask_control_theta0.data); // control img - dmax



	int l = this->l;
	float theta0 = (float)this->theta0 / 180.0 * M_PI;
	min_degree = min_degree / 180.0 *M_PI;
	max_degree = max_degree / 180.0 *M_PI;


	// Gray-Scott models' paramaters
	float f = this->f;
	float k = this->k;

	// Coefficient(weight)
	float addA = this->addA;
	float addB = this->addB;

	// Scaling factors: Sd, Sr  (Refer to Section 3.2.1 in Paper)
	float sd = 0.7 + this->sd;			  //speed of diffusion
	float sr = 1.0 - this->s; //speed of reaction

	// Kernel size: h, w
	int kh = 3;
	int kw = 3;

	//--------------------------------------------- inter start
	int t = 0;
	//const int innerAMPloopsize = 32;
	while (t < innerAMPloopsize)
	{
		t++;

		//--------------------------------------------
		parallel_for_each(alpha_A.extent,
			[=](index<1> idx) restrict(amp)
		{
			index<1> x = idx / nCols;
			index<1> y = idx % nCols;

			//Gradient
			float sx = 0;
			float sy = 0;
			for (int i = -1; i <= 1; i++)
			{
				index<1> j1 = ((x + 1 + nRows) % nRows)*nCols + (y + i + nCols) % nCols;
				index<1> j2 = ((x - 1 + nRows) % nRows)*nCols + (y + i + nCols) % nCols;
				index<1> j3 = ((x + i + nRows) % nRows)*nCols + (y + 1 + nCols) % nCols;
				index<1> j4 = ((x + i + nRows) % nRows)*nCols + (y - 1 + nCols) % nCols;
				sx += c_A[j1] - c_A[j2];
				sy += c_A[j3] - c_A[j4];
			}

			//anisotropic function
			index<1> ix3 = idx * 3;

			// a cross b ?
			float axb = sqrt(sx*sx + sy*sy) * sqrt(p_flowfield[ix3] * p_flowfield[ix3] + p_flowfield[ix3 + 1] * p_flowfield[ix3 + 1]);
			float cos_theta = 0;
			float cross_product = 0;
			if (axb == 0)
			{
				cos_theta = 0;
				cross_product = 0;
			}
			else
			{
				cos_theta = (sx*p_flowfield[ix3] + sy*p_flowfield[ix3 + 1]) / axb;
				cross_product = (sx*p_flowfield[ix3 + 1] - sy*p_flowfield[ix3]) / axb;
			}

			if (cos_theta >= 1.0)
			{
				cos_theta = 1.0;
			}
			else if (cos_theta <= -1.0)
			{
				cos_theta = -1.0;
			}

			float theta = acos(cos_theta);
			if (cross_product < 0)
			{
				theta = 2 * M_PI - theta;
			}

			if (ControlImgLoad && segmentOn)
			{
				// modify anisotropic function with segmentation On
				if (m_control_dmin[idx] <= m_control_dmax[idx])
				{
					if (theta >= m_control_dmin[idx] && theta <= m_control_dmax[idx])
					{
						float temp = 1.0*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
					else
					{
						float temp = 0.5*(1 + cos(m_control_l[idx] * (theta + m_control_theta0[idx])))*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
				}
				else
				{
					if ((theta >= m_control_dmin[idx] && theta <= 2 * M_PI) || (theta <= m_control_dmax[idx] && theta > 0))
					{
						float temp = 1.0*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
					else
					{
						float temp = 0.5*(1 + cos(m_control_l[idx] * (theta + m_control_theta0[idx])))*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
				}
			}
			else
			{
				// modify anisotropic function with segmentation Off
				if (min_degree <= max_degree)
				{
					if (theta >= min_degree && theta <= max_degree)
					{
						float temp = 1.0*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
					else
					{
						float temp = 0.5*(1 + cos(l*(theta + theta0)))*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
				}
				else
				{
					if ((theta >= min_degree && theta <= 2 * M_PI) || (theta <= max_degree&& theta > 0))
					{
						float temp = 1.0*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
					else
					{
						float temp = 0.5*(1 + cos(l*(theta + theta0)))*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
				}
			}

		}
		);

		//---------------------------------------------  
		parallel_for_each(p_A.extent,
			[=](index<1> idx) restrict(amp)
		{
			//anisotropic diffusion
			index<1> x = idx / nCols;
			index<1> y = idx % nCols;
			float da = 0;
			float db = 0;
			for (int q = -kh / 2; q <= kh / 2; q++)
			{
				for (int p = -kw / 2; p <= kw / 2; p++)
				{
					//if (x+q<0 || x+q>=nRows || y+p<0 || y+p>=nCols)
					//	continue;
					index<1> j = ((x + q + nRows) % nRows)*nCols + (y + p + nCols) % nCols;
					da += 0.5*(alpha_A[j] + alpha_A[idx])*(c_A[j] - c_A[idx]);
					db += 0.5*(alpha_A[j] + alpha_A[idx])*(c_B[j] - c_B[idx]);
				}
			}
			da /= (kw*kh - 1);
			db /= (kw*kh - 1);

			//reaction diffusion
			float a = c_A[idx];
			float b = c_B[idx];
			float DA = sd*1.0*da;
			float DB = sd*0.5*db;
			float AA = addA*a_A[idx];
			float AB = addB*a_B[idx];
			float RA;
			float RB;

			// assign diﬀerent parameters to each region
			if (ControlImgLoad && segmentOn)
			{
				RA = m_control_size[idx] * (-a*b*b + m_control_F[idx] * (1 - a));
				RB = m_control_size[idx] * (a*b*b - (m_control_k[idx] + m_control_F[idx])*b);
				DA = m_control_sd[idx]*1.0*da;
				DB = m_control_sd[idx]*0.5*db;
			}
			else
			{
				RA = m_control_size[idx]*(-a*b*b + f*(1 - a));
				RB = m_control_size[idx]*(a*b*b - (m_control_k[idx] + f)*b);
			}
			p_A[idx] = max(min(a + (double)(DA + RA), 1.0), 0.0);
			p_B[idx] = max(min(b + (double)(DB + RB + 0.04*(AB - AA)), 1.0), 0.0);
		}
		);

		//--------------------------------------------- SWAP-------------------------------------------------------

		//slower than parallel_for_each()
		//p_A.copy_to(c_A);
		//p_B.copy_to(c_B);
		parallel_for_each(
			c_A.extent,
			[=](index<1> idx) restrict(amp)
		{
			c_A[idx] = p_A[idx];
			c_B[idx] = p_B[idx];
		}
		);
		
	} //while (t < innerAMPloopsize)
	//--------------------------------------------- inter finish

	clock_t	End_Time = clock();
	clock_t Elapsed_Time = End_Time - Start_Time;
	if (Elapsed_Time < (1000 / 60)) {
		innerAMPloopsize *= 1.5;
		if (innerAMPloopsize > 1024) innerAMPloopsize = 1024;
	}
	else {
		if (Elapsed_Time > (1000 / 30)) {
			innerAMPloopsize *= 0.75;
			if (innerAMPloopsize < 4) innerAMPloopsize = 2;
		}
		
	}

	return innerAMPloopsize;
}

// for Eq.6(Anisotropic) and Eq.7(custom AnisotropicFunction)
int RD::FlowGuideRD(float min_degree, float max_degree, bool isCAF, bool segmentOn)
{
	clock_t	Start_Time = clock();

	int nRows = c_A->rows;
	int nCols = c_A->cols;
	bool ControlImgLoad = this->ControlImgLoad;

	array_view< float, 1 > p_flowfield(nRows*nCols * 3, (float*)Flowfield.data);

	array_view< float, 1 > alpha_A(nRows*nCols, (float*)alpha_A.data);
	//array_view< float, 1 > alpha_B( nRows*nCols, (float*)alpha_B.data );
	array_view< float, 1 > c_A(nRows*nCols, (float*)c_A->data);
	array_view< float, 1 > c_B(nRows*nCols, (float*)c_B->data);
	array_view< float, 1 > p_A(nRows*nCols, (float*)p_A->data);
	array_view< float, 1 > p_B(nRows*nCols, (float*)p_B->data);
	array_view< const float, 1 > a_A(nRows*nCols, (float*)Addition_A.data);
	array_view< const float, 1 > a_B(nRows*nCols, (float*)Addition_B.data);
	array_view< const float, 1 > m(nRows*nCols, (float*)Mask.data);
	array_view< const float, 1 > m_s(nRows*nCols, (float*)Mask_s.data);


	//Mat tmp_Mask_control;
	//normalize(Mask_control, tmp_Mask_control, 0.0, 1.0, NORM_MINMAX, CV_8U);
	array_view<const float, 1> m_control(nRows*nCols, (float*)Mask_control.data); // control img
	array_view<const float, 1> m_control_F(nRows*nCols, (float*)Mask_control_F.data); // control img - F
	array_view<const float, 1> m_control_k(nRows*nCols, (float*)Mask_control_k.data); // control img - k
	array_view<const float, 1> m_control_l(nRows*nCols, (float*)Mask_control_l.data); // control img - l
	array_view<const float, 1> m_control_size(nRows*nCols, (float*)Mask_control_size.data); // control img - size
	array_view<const float, 1> m_control_sd(nRows*nCols, (float*)Mask_control_sd.data); // control img - sd
	array_view<const float, 1> m_control_dmin(nRows*nCols, (float*)Mask_control_dmin.data); // control img - dmin
	array_view<const float, 1> m_control_dmax(nRows*nCols, (float*)Mask_control_dmax.data); // control img - dmax
	array_view<const float, 1> m_control_theta0(nRows*nCols, (float*)Mask_control_theta0.data); // control img - dmax



	int l = this->l;
	float theta0 = (float)this->theta0 / 180.0 * M_PI;
	min_degree = min_degree / 180.0 *M_PI;
	max_degree = max_degree / 180.0 *M_PI;


	float v = this->v;

	// Gray-Scott models' paramaters
	float f = this->f;
	float k = this->k;

	// Coefficient(weight)
	float addA = this->addA;
	float addB = this->addB;

	// Scaling factors: Sd, Sr  (Refer to Section 3.2.1 in Paper)
	float sd = 0.7 + this->sd;			  //speed of diffusion
	float sr = 1.0 - this->s; //speed of reaction

	// Kernel size: h, w
	int kh = 3;
	int kw = 3;

	//--------------------------------------------- inter start
	int t = 0;
	//const int innerAMPloopsize = 32;
	while (t < innerAMPloopsize)
	{
		t++;

		//--------------------------------------------
		parallel_for_each(alpha_A.extent,
			[=](index<1> idx) restrict(amp)
		{
			index<1> x = idx / nCols;
			index<1> y = idx % nCols;

			//Gradient
			float sx = 0;
			float sy = 0;
			for (int i = -1; i <= 1; i++)
			{
				index<1> j1 = ((x + 1 + nRows) % nRows)*nCols + (y + i + nCols) % nCols;
				index<1> j2 = ((x - 1 + nRows) % nRows)*nCols + (y + i + nCols) % nCols;
				index<1> j3 = ((x + i + nRows) % nRows)*nCols + (y + 1 + nCols) % nCols;
				index<1> j4 = ((x + i + nRows) % nRows)*nCols + (y - 1 + nCols) % nCols;
				sx += c_A[j1] - c_A[j2];
				sy += c_A[j3] - c_A[j4];
			}

			//anisotropic function
			index<1> ix3 = idx * 3;

			// a cross b ?
			float axb = sqrt(sx*sx + sy*sy) * sqrt(p_flowfield[ix3] * p_flowfield[ix3] + p_flowfield[ix3 + 1] * p_flowfield[ix3 + 1]);
			float cos_theta = 0;
			float cross_product = 0;
			if (axb == 0)
			{
				cos_theta = 0;
				cross_product = 0;
			}
			else
			{
				cos_theta = (sx*p_flowfield[ix3] + sy*p_flowfield[ix3 + 1]) / axb;
				cross_product = (sx*p_flowfield[ix3 + 1] - sy*p_flowfield[ix3]) / axb;
			}

			if (cos_theta >= 1.0)
			{
				cos_theta = 1.0;
			}
			else if (cos_theta <= -1.0)
			{
				cos_theta = -1.0;
			}

			float theta = acos(cos_theta);
			if (cross_product < 0)
			{
				theta = 2 * M_PI - theta;
			}

			if (ControlImgLoad && segmentOn)
			{
				// modify anisotropic function with segmentation On
				if (m_control_dmin[idx] <= m_control_dmax[idx])
				{
					if (theta >= m_control_dmin[idx] && theta <= m_control_dmax[idx])
					{
						float temp = 1.0*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
					else
					{
						float temp = 0.5*(1 + cos(m_control_l[idx] * (theta + m_control_theta0[idx])))*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
				}
				else
				{
					if ((theta >= m_control_dmin[idx] && theta <= 2 * M_PI) || (theta <= m_control_dmax[idx] && theta > 0))
					{
						float temp = 1.0*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
					else
					{
						float temp = 0.5*(1 + cos(m_control_l[idx] * (theta + m_control_theta0[idx])))*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
				}
			}
			else
			{
				// modify anisotropic function with segmentation Off
				if (min_degree <= max_degree)
				{
					if (theta >= min_degree && theta <= max_degree)
					{
						float temp = 1.0*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
					else
					{
						float temp = 0.5*(1 + cos(l*(theta + theta0)))*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
				}
				else
				{
					if ((theta >= min_degree && theta <= 2 * M_PI) || (theta <= max_degree&& theta > 0))
					{
						float temp = 1.0*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
					else
					{
						float temp = 0.5*(1 + cos(l*(theta + theta0)))*0.9 + 0.1;
						float temp2 = 1 / temp*0.1;
						alpha_A[idx] = temp2;
					}
				}
			}

		}
		);

		//---------------------------------------------  
		parallel_for_each(p_A.extent,
			[=](index<1> idx) restrict(amp)
		{
			//anisotropic diffusion
			index<1> x = idx / nCols;
			index<1> y = idx % nCols;
			float da = 0;
			float db = 0;
			//for (int q = -kh / 2; q <= kh / 2; q++)
			//{
			//	for (int p = -kw / 2; p <= kw / 2; p++)
			//	{
			//		//if (x+q<0 || x+q>=nRows || y+p<0 || y+p>=nCols)
			//		//	continue;
			//		index<1> j = ((x + q + nRows) % nRows)*nCols + (y + p + nCols) % nCols;
			//		da += 0.5*(alpha_A[j] + alpha_A[idx])*(c_A[j] - c_A[idx]);
			//		db += 0.5*(alpha_A[j] + alpha_A[idx])*(c_B[j] - c_B[idx]);
			//	}
			//}
			//da /= (kw*kh - 1);
			//db /= (kw*kh - 1);

			//flowbase diffusion
			int kh = 5 + (1 - m[idx]) * 6;
			float sh = 0.0;
			float sw = 0.0;
			for (int q = 0; q < kh; q++)
			{
				index<1> j = ((x + int(sh) + nRows) % nRows)*nCols + (y + int(sw) + nCols) % nCols;
				index<1> jx3 = j * 3;
				float fx;
				float afx;
				float fy;
				float afy;

				float th = sh;
				float tw = sw;
				for (int p = 0; p < kw; p++)
				{
					j = ((x + int(th) + nRows) % nRows)*nCols + (y + int(tw) + nCols) % nCols;
					jx3 = j * 3;
					fx = p_flowfield[jx3];
					afx = fx < 0 ? -fx : fx;
					fy = -p_flowfield[jx3 + 1];
					afy = fy < 0 ? -fy : fy;
					th += (afy / (afx + afy))*afy / (fy);
					tw += (afx / (afx + afy))*afx / (fx);
					da += 0.5*(alpha_A[j] + alpha_A[idx])*(c_A[j] - c_A[idx]);
					db += 0.5*(alpha_A[j] + alpha_A[idx])*(c_B[j] - c_B[idx]);
				}
				th = sh;
				tw = sw;
				for (int p = 0; p < kw; p++)
				{
					j = ((x + int(th) + nRows) % nRows)*nCols + (y + int(tw) + nCols) % nCols;
					jx3 = j * 3;
					fx = -p_flowfield[jx3];
					afx = fx < 0 ? -fx : fx;
					fy = p_flowfield[jx3 + 1];
					afy = fy < 0 ? -fy : fy;
					th += (afy / (afx + afy))*afy / (fy);
					tw += (afx / (afx + afy))*afx / (fx);
					da += 0.5*(alpha_A[j] + alpha_A[idx])*(c_A[j] - c_A[idx]);
					db += 0.5*(alpha_A[j] + alpha_A[idx])*(c_B[j] - c_B[idx]);
				}
				jx3 = j * 3;
				fx = p_flowfield[jx3];
				afx = fx < 0 ? -fx : fx;
				fy = p_flowfield[jx3 + 1];
				afy = fy < 0 ? -fy : fy;
				sh += (afx / (afx + afy))*afx / (fx);
				sw += (afy / (afx + afy))*afy / (fy);
				j = ((x + int(sh) + nRows) % nRows)*nCols + (y + int(sw) + nCols) % nCols;
				da += 0.5*(alpha_A[j] + alpha_A[idx])*(c_A[j] - c_A[idx]);
				db += 0.5*(alpha_A[j] + alpha_A[idx])*(c_B[j] - c_B[idx]);
			}

			sh = 0.0;
			sw = 0.0;
			for (int q = 0; q < kh; q++)
			{
				index<1> j = ((x + int(sh) + nRows) % nRows)*nCols + (y + int(sw) + nCols) % nCols;
				index<1> jx3 = j * 3;
				float fx;
				float afx;
				float fy;
				float afy;
				float th = sh;
				float tw = sw;
				for (int p = 0; p < kw; p++)
				{
					j = ((x + int(th) + nRows) % nRows)*nCols + (y + int(tw) + nCols) % nCols;
					jx3 = j * 3;
					fx = p_flowfield[jx3];
					afx = fx < 0 ? -fx : fx;
					fy = -p_flowfield[jx3 + 1];
					afy = fy < 0 ? -fy : fy;
					th += (afy / (afx + afy))*afy / (fy);
					tw += (afx / (afx + afy))*afx / (fx);
					da += 0.5*(alpha_A[j] + alpha_A[idx])*(c_A[j] - c_A[idx]);
					db += 0.5*(alpha_A[j] + alpha_A[idx])*(c_B[j] - c_B[idx]);
				}
				th = sh;
				tw = sw;
				for (int p = 0; p < kw; p++)
				{
					j = ((x + int(th) + nRows) % nRows)*nCols + (y + int(tw) + nCols) % nCols;
					jx3 = j * 3;
					fx = -p_flowfield[jx3];
					afx = fx < 0 ? -fx : fx;
					fy = p_flowfield[jx3 + 1];
					afy = fy < 0 ? -fy : fy;
					th += (afy / (afx + afy))*afy / (fy);
					tw += (afx / (afx + afy))*afx / (fx);
					da += 0.5*(alpha_A[j] + alpha_A[idx])*(c_A[j] - c_A[idx]);
					db += 0.5*(alpha_A[j] + alpha_A[idx])*(c_B[j] - c_B[idx]);
				}
				jx3 = j * 3;
				fx = -p_flowfield[jx3];
				afx = fx < 0 ? -fx : fx;
				fy = -p_flowfield[jx3 + 1];
				afy = fy < 0 ? -fy : fy;
				sh += (afx / (afx + afy))*afx / (fx);
				sw += (afy / (afx + afy))*afy / (fy);
				j = ((x + int(sh) + nRows) % nRows)*nCols + (y + int(sw) + nCols) % nCols;
				da += 0.5*(alpha_A[j] + alpha_A[idx])*(c_A[j] - c_A[idx]);
				db += 0.5*(alpha_A[j] + alpha_A[idx])*(c_B[j] - c_B[idx]);
			}
			da /= (2 * (kh - 1) * 2 * (kw + 1) - 1);
			db /= (2 * (kh - 1) * 2 * (kw + 1) - 1);


			float M = m[idx];
			float MS = m_s[idx];
			if (M > v) { M = 1.0; }

			else { M = 0.0; }


			if (MS > 0.1) { MS = 1.0; }

			else { MS = 0.0; }






			//reaction diffusion
			float a = c_A[idx];
			float b = c_B[idx];
			float DA = sd*1.0*da;
			float DB = sd*0.5*db;
			float AA = addA*a_A[idx];
			float AB = addB*a_B[idx];
			float RA;
			float RB;

			// assign diﬀerent parameters to each region
			if (ControlImgLoad && segmentOn)
			{
				RA = m_control_size[idx] * (-a*b*b + m_control_F[idx] * (1 - a));
				RB = m_control_size[idx] * (a*b*b - (m_control_k[idx] + m_control_F[idx])*b);
				DA = m_control_sd[idx] * 1.0*da;
				DB = m_control_sd[idx] * 0.5*db;
			}
			else
			{
				RA = m_control_size[idx] * (-a*b*b + f*(1 - a));
				RB = m_control_size[idx] * (a*b*b - (m_control_k[idx] + f)*b);
			}
			p_A[idx] = max(min(a + (double)(DA + RA), 1.0), 0.0);
			p_B[idx] = max(min(b + (double)(DB + RB + 0.04*(AB - AA)), 1.0), 0.0);
		}
		);

		//--------------------------------------------- SWAP-------------------------------------------------------

		//slower than parallel_for_each()
		//p_A.copy_to(c_A);
		//p_B.copy_to(c_B);
		parallel_for_each(
			c_A.extent,
			[=](index<1> idx) restrict(amp)
		{
			c_A[idx] = p_A[idx];
			c_B[idx] = p_B[idx];
		}
		);

	} //while (t < innerAMPloopsize)
	//--------------------------------------------- inter finish

	clock_t	End_Time = clock();
	clock_t Elapsed_Time = End_Time - Start_Time;
	if (Elapsed_Time < (1000 / 60))
	{
		innerAMPloopsize *= 1.5;
		if (innerAMPloopsize > 1024) innerAMPloopsize = 1024;
	}
	else
	{
		if (Elapsed_Time > (1000 / 30))
		{
			innerAMPloopsize *= 0.75;
			if (innerAMPloopsize < 4) innerAMPloopsize = 2;
		}

	}

	return innerAMPloopsize;
}

//// for Eq.7
//void RD::FastGrayScott(float min_degree, float max_degree, bool segmentOn){
//	int nRows = c_A->rows;
//	int nCols = c_A->cols;
//	array_view< float, 1 > p_flowfield(nRows*nCols * 3, (float*)Flowfield.data);
//
//	array_view< float, 1 > alpha_A(nRows*nCols, (float*)alpha_A.data);
//	//array_view< float, 1 > alpha_B( nRows*nCols, (float*)alpha_B.data );
//	array_view< float, 1 > c_A(nRows*nCols, (float*)c_A->data);
//	array_view< float, 1 > c_B(nRows*nCols, (float*)c_B->data);
//	array_view< float, 1 > p_A(nRows*nCols, (float*)p_A->data);
//	array_view< float, 1 > p_B(nRows*nCols, (float*)p_B->data);
//	array_view< float, 1 > a_A(nRows*nCols, (float*)Addition_A.data);
//	array_view< float, 1 > a_B(nRows*nCols, (float*)Addition_B.data);
//	array_view< float, 1 > m(nRows*nCols, (float*)Mask.data);
//	array_view< float, 1 > m_s(nRows*nCols, (float*)Mask_s.data);
//
//	array_view<float, 1> m_control(nRows*nCols, (float*)Mask_control.data); // control img
//
//	int l = this->l;
//	float theta0 = (float)this->theta0 / 180.0 * M_PI;
//	min_degree = min_degree / 180.0 *M_PI;
//	max_degree = max_degree / 180.0 *M_PI;
//
//	// Gray-Scott models' paramaters
//	float f = this->f;
//	float k = this->k;
//
//	// Coefficient(weight)
//	float addA = this->addA;
//	float addB = this->addB;
//
//	// Scaling factors: Sd, Sr  (Refer to Section 3.2.1 in Paper)
//	float sd = 1.0;			  //speed of diffusion
//	float sr = 1.0 - this->s; //speed of reaction
//
//	int kh = 3;
//	int kw = 3;
//	//inter start
//	int t = 0;
//	const int innerAMPloopsize = 4;
//	while (t < innerAMPloopsize){
//		t++;
//		parallel_for_each(alpha_A.extent,
//			[=](index<1> idx) restrict(amp)
//		{
//			index<1> x = idx / nCols;
//			index<1> y = idx%nCols;
//			//Gradient
//			float sx = 0;
//			float sy = 0;
//			for (int i = -1; i <= 1; i++){
//				index<1> j1 = ((x + 1 + nRows) % nRows)*nCols + (y + i + nCols) % nCols;
//				index<1> j2 = ((x - 1 + nRows) % nRows)*nCols + (y + i + nCols) % nCols;
//				index<1> j3 = ((x + i + nRows) % nRows)*nCols + (y + 1 + nCols) % nCols;
//				index<1> j4 = ((x + i + nRows) % nRows)*nCols + (y - 1 + nCols) % nCols;
//				sx += c_A[j1] - c_A[j2];
//				sy += c_A[j3] - c_A[j4];
//			}
//
//			//anisotropic function
//			index<1> ix3 = idx * 3;
//
//			// a cross b ?
//			float axb = sqrt(sx*sx + sy*sy) * sqrt(p_flowfield[ix3] * p_flowfield[ix3] + p_flowfield[ix3 + 1] * p_flowfield[ix3 + 1]);
//			float cos_theta = 0;
//			float cross_product = 0;
//			if (axb == 0){
//				cos_theta = 0;
//				cross_product = 0;
//			}
//			else{
//				cos_theta = (sx*p_flowfield[ix3] + sy*p_flowfield[ix3 + 1]) / axb;
//				cross_product = (sx*p_flowfield[ix3 + 1] - sy*p_flowfield[ix3]) / axb;
//			}
//			if (cos_theta >= 1.0){
//				cos_theta = 1.0;
//			}
//			else if (cos_theta <= -1.0){
//				cos_theta = -1.0;
//			}
//			float theta = acos(cos_theta);
//			if (cross_product < 0){
//				theta = 2 * M_PI - theta;
//			}
//
//			// modify anisotropic function
//			if (min_degree <= max_degree){ 
//				if (theta  >= min_degree && theta <= max_degree){
//					float temp = 1.0*0.9 + 0.1;
//					float temp2 = 1 / temp*0.1;
//					alpha_A[idx] = temp2;
//				}
//				else{
//					float temp = 0.5*(1 + cos(l*(theta + theta0)))*0.9 + 0.1;
//					float temp2 = 1 / temp*0.1;
//					alpha_A[idx] = temp2;
//				}
//			}
//			else{
//				if ((theta >= min_degree && theta <= 2 * M_PI) || (theta <= max_degree&& theta > 0)){
//					float temp = 1.0*0.9 + 0.1;
//					float temp2 = 1 / temp*0.1;
//					alpha_A[idx] = temp2;
//				}
//				else{
//					float temp = 0.5*(1 + cos(l*(theta + theta0)))*0.9 + 0.1;
//					float temp2 = 1 / temp*0.1;
//					alpha_A[idx] = temp2;
//				}
//			}
//		}
//		);
//
//		parallel_for_each(p_A.extent,
//			[=](index<1> idx) restrict(amp)
//		{
//			//anisotropic diffusion
//			index<1> x = idx / nCols;
//			index<1> y = idx % nCols;
//			float da = 0;
//			float db = 0;
//			for (int q = -kh / 2; q <= kh / 2; q++){
//				for (int p = -kw / 2; p <= kw / 2; p++){
//					//if (x+q<0 || x+q>=nRows || y+p<0 || y+p>=nCols)
//					//	continue;
//					index<1> j = ((x + q + nRows) % nRows)*nCols + (y + p + nCols) % nCols;
//					da += 0.5*(alpha_A[j] + alpha_A[idx])*(c_A[j] - c_A[idx]);
//					db += 0.5*(alpha_A[j] + alpha_A[idx])*(c_B[j] - c_B[idx]);
//				}
//			}
//			da /= (kw*kh - 1);
//			db /= (kw*kh - 1);
//
//			//reaction diffusion
//			float a = c_A[idx];
//			float b = c_B[idx];
//			float DA = sd*1.0*da;
//			float DB = sd*0.5*db;
//			float RA;
//			float RB;
//			float AA = addA*a_A[idx];
//			float AB = addB*a_B[idx];
//			if (m_control[idx] == 0.0){  // RD work on black area
//				RA = sr*(-a*b*b + f*(1 - a));
//				RB = sr*(a*b*b - (k + f)*b);
//			}
//			else{// else didn't work (on white area)
//				float f = 0.7;
//				float k = 0.7;
//				RA = sr*(-a*b*b + f*(1 - a));
//				RB = sr*(a*b*b - (k + f)*b);
//			}
//			p_A[idx] = max(min(a + (double)(DA + RA), 1.0), 0.0);
//			p_B[idx] = max(min(b + (double)(DB + RB + 0.04*(AB - AA)), 1.0), 0.0);
//		}
//		);
//		//--------------------------------------------- SWAP-------------------------------------------------------
//
//		//slower than parallel_for_each()
//		//p_A.copy_to(c_A);
//		//p_B.copy_to(c_B);
//		parallel_for_each(
//			c_A.extent,
//			[=](index<1> idx) restrict(amp)
//		{
//			c_A[idx] = p_A[idx];
//			c_B[idx] = p_B[idx];
//		}
//		);
//		//inter finish
//	}
//}

// generate parameter space of GS-Model of specific F
// X-axis: k from(0.056~0.066)?
// Y-axis: l from 7 regions (0, 1, 2, ..., 6)
int RD::GrayScottModel()
{
	clock_t	Start_Time = clock();

	int nRows = 500;//todo: fix to use non 500x500
	int nCols = 500;
	array_view< float, 1 > p_flowfield(nRows*nCols * 3, (float*)Flowfield.data);

	array_view< float, 1 > alpha_A(nRows*nCols, (float*)alpha_A.data);
	//array_view< float, 1 > alpha_B( nRows*nCols, (float*)e.alpha_B.data );
	array_view< float, 1 > c_A(nRows*nCols, (float*)c_A->data);
	array_view< float, 1 > c_B(nRows*nCols, (float*)c_B->data);
	array_view< float, 1 > p_A(nRows*nCols, (float*)p_A->data);
	array_view< float, 1 > p_B(nRows*nCols, (float*)p_B->data);
	array_view< float, 1 > a_A(nRows*nCols, (float*)Addition_A.data);
	array_view< float, 1 > a_B(nRows*nCols, (float*)Addition_B.data);
	array_view< const float, 1 > m(nRows*nCols, (float*)Mask.data);
	array_view< const float, 1 > m_s(nRows*nCols, (float*)Mask_s.data);

	float theta0 = 0;

	int larray[500];// = this->l;
	for (int i = 0; i < 500; i++) larray[i] = i / (500/7);  //7: 0, 1, 2,...6

	array_view<int, 1> l(500, larray); //Y-axis: divide to 7 region with parameter l 


	// Gray-Scott models' paramaters
	float farray[500];// = 0.0375;
	float karray[500];

	//for (int i = 0; i < 500; i++){
	//	int kbias = (i + 1) / 25;
	//	karray[i] = 0.056 + ((float)kbias / 2000.0);
	//}
	//farray[500] = 0.0375;
	for (int i = 0; i < 500; i++)
	{
		karray[i] = 0.056 + (float)(i + 1) / 42000.0;
		farray[i] = this->f; //(float)(i + 1) / 8334.0;
	}
	//for (int i = 0; i < 501; i++) farray[i] = 0.0 + ((float)(i + 1) / 8333.3);
	array_view< const float, 1 > k(500, karray);
	array_view< const float, 1 > f(500, farray);
	//array_view< float, 1 > f(nRows, farray);
	//float k;

	// Coefficient(weight)
	float addA = this->addA;
	float addB = this->addB;

	// Scaling factors: Sd, Sr  (Refer to Section 3.2.1 in Paper)
	float sd = 0.7;	//speed of diffusion
	float sr = 0.3; //speed of reaction

	int kh = 3;
	int kw = 3;
	//inter start
	int t = 0;
	//const int innerAMPloopsize = 64;
	while (t < innerAMPloopsize)
	{
		t++;
		parallel_for_each(alpha_A.extent,
			[=](index<1> idx) restrict(amp)
		{
			index<1> x = idx / nCols;
			index<1> y = idx % nCols;
			//Gradient
			float sx = 0;
			float sy = 0;
			for (int i = -1; i <= 1; i++)
			{
				index<1> j1 = ((x + 1 + nRows) % nRows)*nCols + (y + i + nCols) % nCols;
				index<1> j2 = ((x - 1 + nRows) % nRows)*nCols + (y + i + nCols) % nCols;
				index<1> j3 = ((x + i + nRows) % nRows)*nCols + (y + 1 + nCols) % nCols;
				index<1> j4 = ((x + i + nRows) % nRows)*nCols + (y - 1 + nCols) % nCols;
				sx += c_A[j1] - c_A[j2];
				sy += c_A[j3] - c_A[j4];
			}

			//anisotropic function
			index<1> ix3 = idx * 3;
			float axb = sqrt(sx*sx + sy*sy) * sqrt(p_flowfield[ix3] * p_flowfield[ix3] + p_flowfield[ix3 + 1] * p_flowfield[ix3 + 1]);
			float cos_theta = 0;
			float cross_product = 0;
			if (axb == 0)
			{
				cos_theta = 0;
				cross_product = 0;
			}
			else
			{
				cos_theta = (sx*p_flowfield[ix3] + sy*p_flowfield[ix3 + 1]) / axb;
				cross_product = (sx*p_flowfield[ix3 + 1] - sy*p_flowfield[ix3]) / axb;
			}
			if (cos_theta >= 1.0)
			{
				cos_theta = 1.0;
			}
			else if (cos_theta <= -1.0)
			{
				cos_theta = -1.0;
			}
			float theta = acos(cos_theta);
			if (cross_product < 0)
			{
				theta = 2 * M_PI - theta;
			}
			int ll = l[x];
			float temp = 0.5*(1 + cos(ll*(theta + theta0)))*0.9 + 0.1;
			float temp2 = 1 / temp*0.1;
			alpha_A[idx] = temp2;
		}
		);

		parallel_for_each(p_A.extent,
			[=](index<1> idx) restrict(amp)
		{

			//anisotropic diffusion
			index<1> x = idx / nCols;
			index<1> y = idx % nCols;
			float da = 0;
			float db = 0;
			for (int q = -kh / 2; q <= kh / 2; q++)
			{
				for (int p = -kw / 2; p <= kw / 2; p++)
				{
					//if (x+q<0 || x+q>=nRows || y+p<0 || y+p>=nCols)
					//	continue;
					index<1> j = ((x + q + nRows) % nRows)*nCols + (y + p + nCols) % nCols;
					da += 0.5*(alpha_A[j] + alpha_A[idx])*(c_A[j] - c_A[idx]);
					db += 0.5*(alpha_A[j] + alpha_A[idx])*(c_B[j] - c_B[idx]);
				}
			}
			da /= (kw*kh - 1);
			db /= (kw*kh - 1);

			//reaction diffusion
			float a = c_A[idx];
			float b = c_B[idx];
			float DA = sd*1.0*da;
			float DB = sd*0.5*db;
			float ff = f[0];//0.0375;
			float RA = sr*(-a*b*b + ff*(1 - a));
			float RB = sr*(a*b*b - (k[y] + ff)*b);
			float AA = addA*a_A[idx];
			float AB = addB*a_B[idx];
			p_A[idx] = max(min(a + (double)(DA + RA), 1.0), 0.0);
			p_B[idx] = max(min(b + (double)(DB + RB + 0.04*(AB - AA)), 1.0), 0.0);
		}
		);
		//--------------------------------------------- SWAP-------------------------------------------------------
		parallel_for_each(c_A.extent,
			[=](index<1> idx) restrict(amp)
		{
			c_A[idx] = p_A[idx];
			c_B[idx] = p_B[idx];
		}
		);
		//inter finish
	}

	clock_t	End_Time = clock();
	clock_t Elapsed_Time = End_Time - Start_Time;
	if (Elapsed_Time < (1000 / 60)) {
		innerAMPloopsize *= 1.5;
		if (innerAMPloopsize > 1024) innerAMPloopsize = 1024;
	}
	else {
		if (Elapsed_Time > (1000 / 30)) {
			innerAMPloopsize *= 0.75;
			if (innerAMPloopsize < 4) innerAMPloopsize = 2;
		}

	}

	return innerAMPloopsize;
}

// Testing checkboard Size mask
void RD::CheckboardSizeMask()
{

	for (int i = 0; i < Mask_control.rows; i++)
	{
		for (int j = 0; j < Mask_control.cols; j++)
		{
			int k = (j / 50) % 2;
			int l = (i / 50) % 2;
			if ((k == 0 && l == 0) || (k == 1 && l == 1))
				Mask_control_size.at<float>(i, j) = 0.6;
			else
				Mask_control_size.at<float>(i, j) = 0.2;
		}
	}
}

