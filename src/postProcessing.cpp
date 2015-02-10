#include "postProcessing.h"

PP::PP(Size s){
	texture = Mat::ones(s, CV_32FC3);  //why

	alpha = 0.5;
	beta = 0.5;

	float m_alpha = 0.0; //why
	float m_beta = 0.0;
	float t_alpha = 0.0;
	float t_beta = 0.0;

	TextureLoaded = false;
}

void PP::SeedGradient(Mat &src, Mat &Gradient){
	Mat sobel_x, sobel_y;
	Sobel(src, sobel_x, CV_32F, 1, 0, 3, 1, 0, 1);
	Sobel(src, sobel_y, CV_32F, 0, 1, 3, 1, 0, 1);
	vector<Mat> channel;
	channel.push_back(sobel_y);
	channel.push_back(sobel_x);
	channel.push_back(Mat::zeros(src.size(), CV_32F));
	merge(channel, Gradient);
}

//bug
void PP::LIC(Mat &flowfield, Mat &dis)
{
	const float M_PI = 3.14159265358979323846;
	Mat noise = Mat::zeros(flowfield.size(), CV_32F);
	dis = Mat::zeros(flowfield.size(), CV_32F);
	randu(noise, 0, 255);
	noise = noise / 255.0;

	int s = 10;
	int nRows = noise.rows;
	int nCols = noise.cols;
	float sigma = 2 * s*s;
#pragma omp parallel for
	for (int i = 0; i<nRows; i++){
		for (int j = 0; j<nCols; j++){
			float w_sum = 0.0;
			float x = i;
			float y = j;
			for (int k = 0; k<s; k++){
				Vec3f v = normalize(flowfield.at<Vec3f>(int(x + nRows) % nRows, int(y + nCols) % nCols));
				if (v[0] != 0 && v[1] != 0){
					x = x + (abs(v[0]) / float(abs(v[0]) + abs(v[1])))*(abs(v[0]) / v[0]);
					y = y + (abs(v[1]) / float(abs(v[0]) + abs(v[1])))*(abs(v[1]) / v[1]);
				}
				float r2 = k*k;
				float w = (1 / (M_PI*sigma))*exp(-(r2) / sigma);
				dis.at<float>(i, j) += w*noise.at<float>(int(x + nRows) % nRows, int(y + nCols) % nCols);
				w_sum += w;
			}

			//x = i;
			//y = j;
			//for (int k = 0; k<s; k++){
			//	Vec3f v = -normalize(flowfield.at<Vec3f>(int(x + nRows) % nRows, int(y + nCols) % nCols));
			//	x = x + (abs(v[0]) / float(abs(v[0]) + abs(v[1])))*(abs(v[0]) / v[0]);
			//	y = y + (abs(v[1]) / float(abs(v[0]) + abs(v[1])))*(abs(v[1]) / v[1]);
			//	float r2 = k*k;
			//	float w = (1 / (M_PI*sigma))*exp(-(r2) / sigma);
			//	dis.at<float>(i, j) += w*noise.at<float>(int(x + nRows) % nRows, int(y + nCols) % nCols);
			//	w_sum += w;
			//}
			if (w_sum != 0)
				dis.at<float>(i, j) /= w_sum;
		}
	}
}

void PP::motionIllu(Mat &src, Mat &flowfield, Mat &dis){
	vector<Mat> channels;
	Mat Gradient = Mat::zeros(src.size(), CV_32FC3);
	SeedGradient(src, Gradient);
	Mat r = Mat::zeros(src.size(), CV_32F);
	Mat b = Mat::zeros(src.size(), CV_32F);
	Mat g = Mat::zeros(src.size(), CV_32F);

#pragma omp parallel for
	for (int i = 0; i < src.rows; i++){
		for (int j = 0; j < src.cols; j++){
			float cos_theta = normalize(Gradient.at<Vec3f>(i, j)).dot(normalize(flowfield.at<Vec3f>(i, j)));
			float range = alpha / 2.0;
			float center = beta;
			if (src.at<float>(i, j) > center + range){
				r.at<float>(i, j) = 1.0;
				g.at<float>(i, j) = 1.0;
				b.at<float>(i, j) = 0.0;
			}
			else if (src.at<float>(i, j) < center - range){
				r.at<float>(i, j) = 0.0;
				g.at<float>(i, j) = 0.0;
				b.at<float>(i, j) = 1.0;
			}
			else if (cos_theta > 0){
				r.at<float>(i, j) = 0.0;
				g.at<float>(i, j) = 0.0;
				b.at<float>(i, j) = 0.0;
			}
			else {
				r.at<float>(i, j) = 1.0;
				g.at<float>(i, j) = 1.0;
				b.at<float>(i, j) = 1.0;
			}
		}
	}
	channels.push_back(b);
	channels.push_back(g);
	channels.push_back(r);
	merge(channels, dis);
};

void PP::dirTexture(Mat &src, Mat &flowfield, Mat &dis){
	vector<Mat> channels;
	Mat Gradient = Mat::zeros(src.size(), CV_32FC3);
	SeedGradient(src, Gradient);
	Mat r = Mat::zeros(src.size(), CV_32F);
	Mat b = Mat::zeros(src.size(), CV_32F);
	Mat g = Mat::zeros(src.size(), CV_32F);

#pragma omp parallel for
	for (int i = 0; i < src.rows; i++){
		for (int j = 0; j < src.cols; j++){
			float cos_theta = normalize(Gradient.at<Vec3f>(i, j)).dot(normalize(flowfield.at<Vec3f>(i, j)));
			float x = abs(cos_theta + 1)*0.5;
			float y = src.at<float>(i, j);
			r.at<float>(i, j) = texture.at<Vec3b>(x*(texture.rows - 1), y*(texture.cols - 1))[2] / 255.0;
			b.at<float>(i, j) = texture.at<Vec3b>(x*(texture.rows - 1), y*(texture.cols - 1))[0] / 255.0;
			g.at<float>(i, j) = texture.at<Vec3b>(x*(texture.rows - 1), y*(texture.cols - 1))[1] / 255.0;
		}
	}
	channels.push_back(b);
	channels.push_back(g);
	channels.push_back(r);
	merge(channels, dis);
};

void PP::Thresholding(Mat &src, Mat &dis){
	vector<Mat> channels;
	Mat r = Mat::zeros(src.size(), CV_32F);
	Mat b = Mat::zeros(src.size(), CV_32F);
	Mat g = Mat::zeros(src.size(), CV_32F);
#pragma omp parallel for
	for (int i = 0; i < src.rows; i++){
		for (int j = 0; j < src.cols; j++){
			float range = alpha / 2;
			float center = beta;
			if (src.at<float>(i, j) > center + range){
				r.at<float>(i, j) = 1.0;
				g.at<float>(i, j) = 1.0;
				b.at<float>(i, j) = 1.0;
			}
			else if (src.at<float>(i, j) < center - range){
				r.at<float>(i, j) = 0.0;
				g.at<float>(i, j) = 0.0;
				b.at<float>(i, j) = 0.0;
			}
			else {
				r.at<float>(i, j) = (src.at<float>(i, j) - (center - range)) / (range * 2);
				g.at<float>(i, j) = (src.at<float>(i, j) - (center - range)) / (range * 2);
				b.at<float>(i, j) = (src.at<float>(i, j) - (center - range)) / (range * 2);
			}
		}
	}
	channels.push_back(b);
	channels.push_back(g);
	channels.push_back(r);
	merge(channels, dis);
};

void PP::adaThresholding(Mat &src, Mat &mask, Mat &dis){
	vector<Mat> channels;
	Mat r = Mat::zeros(src.size(), CV_32F);
	Mat b = Mat::zeros(src.size(), CV_32F);
	Mat g = Mat::zeros(src.size(), CV_32F);
#pragma omp parallel for
	for (int i = 0; i < src.rows; i++){
		for (int j = 0; j < src.cols; j++){
			float center = ((1 - (mask.at<float>(i, j)))*beta + (mask.at<float>(i, j))*alpha);
			if (src.at<float>(i, j) > center){
				r.at<float>(i, j) = 1.0;
				g.at<float>(i, j) = 1.0;
				b.at<float>(i, j) = 1.0;
			}
			else if (src.at<float>(i, j) < center){
				r.at<float>(i, j) = 0.0;
				g.at<float>(i, j) = 0.0;
				b.at<float>(i, j) = 0.0;
			}
		}
	}
	channels.push_back(b);
	channels.push_back(g);
	channels.push_back(r);
	merge(channels, dis);
};

void PP::ReadTexture(string file){
	texture = imread(file, 1);
	TextureLoaded = true;
}