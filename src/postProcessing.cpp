#include "postProcessing.h"

PP::PP(Size s){
	texture = Mat::ones(s, CV_32FC3);  //why

	alpha = 0.2;
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

void PP::LIC(Mat &flowfield, Mat &dis)
{
	const float M_PI = 3.14159265358979323846;
	Mat noise = Mat::zeros(cv::Size(flowfield.cols / 2, flowfield.rows / 2), CV_32F);
 	dis = Mat::zeros(flowfield.size(), CV_32F);
	randu(noise, 0, 1.0f);
	resize(noise, noise, flowfield.size(), 0, 0, INTER_NEAREST);

	int s = 10;
	int nRows = noise.rows; //h
	int nCols = noise.cols; //w
	float sigma = 2 * s*s;
#pragma omp parallel for
	for (int j = 0; j<nRows; j++){
		for (int i = 0; i<nCols; i++){
			float w_sum = 0.0;
			float x = i;
			float y = j;
			for (int k = 0; k < s; k++){
				Vec3f v = normalize(flowfield.at<Vec3f>(int(x + nCols) % nCols, int(y + nRows) % nRows));
				x = x + (abs(v[0]) / float(abs(v[0]) + abs(v[1])))*(abs(v[0]) / v[0]);
				y = y + (abs(v[1]) / float(abs(v[0]) + abs(v[1])))*(abs(v[1]) / v[1]);
				float r2 = k*k;
				float w = (1 / (M_PI*sigma))*exp(-(r2) / sigma);
				dis.at<float>(j, i) += w*noise.at<float>(int(x + nCols) % nCols, int(y + nRows) % nRows);
				w_sum += w;
			}

			x = i;
			y = j;
			for (int k = 0; k<s; k++){
				Vec3f v = -normalize(flowfield.at<Vec3f>(int(x + nCols) % nCols, int(y + nRows) % nRows));
				x = x + (abs(v[0]) / float(abs(v[0]) + abs(v[1])))*(abs(v[0]) / v[0]);
				y = y + (abs(v[1]) / float(abs(v[0]) + abs(v[1])))*(abs(v[1]) / v[1]);

				float r2 = k*k;
				float w = (1 / (M_PI*sigma))*exp(-(r2) / sigma);
				dis.at<float>(j, i) += w*noise.at<float>(int(x + nCols) % nCols, int(y + nRows) % nRows);
				w_sum += w;
			}
			dis.at<float>(j, i) /= w_sum;
		}
	}

}


void PP::Flowtest(Mat &flowfield, Mat &dis)
{
	const float M_PI = 3.14159265358979323846;

	dis = Mat::zeros(flowfield.size(), CV_32F);


	int s = 10;
	int nRows = flowfield.rows;
	int nCols = flowfield.cols;
	float step = nRows*(alpha>0? alpha: 0.1);
#pragma omp parallel for
	for (int j = 0; j<nRows; j += step){
		for (int i = 0; i<nCols; i += step){

			float x = i;
			float y = j;

			Vec3f v = normalize(flowfield.at<Vec3f>(int(x + nCols) % nCols, int(y + nRows) % nRows));
			//Vec3f v = Vec3f(1, 0, 0);
			//Vec3f v = Vec3f(0, 1, 0);
			//Vec3f v = normalize(Vec3f(x - 0.5*nCols, y - 0.5*nRows, 0));  //source


			int thickness = 1;
			int lineType = 8;

			circle(dis,
				Point(x, y),
				3,
				Scalar(255, 0, 255));

			line(dis,
				Point(x, y), 
				Point(x + 0.5*step*v[0], y + 0.5*step*v[1]),
				Scalar(255, 0, 0),
				thickness,
				lineType);
		}
	}

	float x = 0.5*nCols;
	float y = 0.5*nRows;
	static float angle = 0;

	line(dis,
		Point(x, y),
		Point(x + 10, y),
		Scalar(255, 0, 0),
		2);
	line(dis,
		Point(x, y),
		Point(x, y + 10),
		Scalar(255, 0, 0),
		2,
		3);

	line(dis,
		Point(x, y),
		Point(x + 10*cos(angle), y + 10*sin(angle)),
		Scalar(255, 0, 0),
		1);
	angle += 0.01;

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

// X-aixs: angle between grident and flowvector
// Y-aixs: density
void PP::dirTexture(Mat &src, Mat &flowfield, Mat &dis)
{
	const float M_PI = 3.14159265358979323846;

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

			float x = acos(cos_theta) / M_PI; //cos(theta) [-1, 1] map to [0, pi] to [0, 1]
			float y = src.at<float>(i, j);	  //density
			
			x = max(min(x, 1.0f), 0.0f);
			y = max(min(y, 1.0f), 0.0f);

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

// Polar coordinate (Angle, Radius)
void PP::dirTexture_Polar(Mat &src, Mat &flowfield, Mat &dis)
{
	const float M_PI = 3.14159265358979323846;

	vector<Mat> channels;
	Mat Gradient = Mat::zeros(src.size(), CV_32FC3);
	SeedGradient(src, Gradient);
	Mat r = Mat::zeros(src.size(), CV_32F);
	Mat b = Mat::zeros(src.size(), CV_32F);
	Mat g = Mat::zeros(src.size(), CV_32F);

#pragma omp parallel for
	for (int i = 0; i < src.rows; i++){
		for (int j = 0; j < src.cols; j++){
			//float shift_angle = theta0 / 360 * 2 * M_PI;
			float cos_theta = normalize(Gradient.at<Vec3f>(i, j)).dot(normalize(flowfield.at<Vec3f>(i, j)));
			float Radius = 1 - src.at<float>(i, j);	//density

			//use alpha, beta to adjust
			float x = 0.5 + alpha + beta*Radius*cos_theta;
			float y = 0.5 + alpha + beta*Radius*sin(acos(cos_theta));

			x = max(min(x, 1.0f), 0.0f);
			y = max(min(y, 1.0f), 0.0f);

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

