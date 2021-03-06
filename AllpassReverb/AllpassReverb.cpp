// AllpassReverb.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "AudioFile.cpp"
#include <iostream>
#include <vector>
#include <string>
#include "AllpassReverbBased.h"
#include "math.h"
using namespace std;

//	class 
class Biquad {
public:
	double coeffs[2][3] = { 0 };
	double state[3] = { 0 };
	int counter = 0;

	int setCoeffs(double *b, double *a);
	int reset();
	double filter(double input, double &output);
	double filter(vector<double> input, vector<double> &output);
};
class FIR {
public:
	double *coeffs;
	double *state;
	int counter = 0;
	int taps;
	double scale = 0.25;

	int init(int taps_num,double *coeffs_val);
	int filter(double input,double & output);
	~FIR() {
		// 回收内存
		delete[] coeffs, state;
	}
};
class Reverberator {
public:
	double *delay_line;			// 根据混响类型，需要不同长度的延时线
	Biquad lpf;
	unsigned int fs = 44.1e3;		// default sampling frequency is 48 000 Hz.
	unsigned int tap;
	unsigned int head, tail,len;
	int init(unsigned int sample_freq);
	int reverb(double input, double &output);
};

vector<double> delay_line_mode(int tap_ms, int total_samples, vector<double> &data_in);	// 测试样例一  - passed
int sampleDelayLineBasedAllpass(double &data_in, double &data_out, double  gain);		// 基础结构，用于各种样例中

double biquad(double input, double &output, double coeff[][3], double *state);		 // 即将废弃，转到 Biquad 类中处理	
double fir_init(double tap_am[][20],double virtal_distance,double real_distance,double phi,double theta,double absort_coef);// early echo fir filter design
int early_echo_fir(double input, double & output, double coeffs[], double state[], int taps, int & counter);// 即将废弃
// 中等房间混响测试样例
int mediumReverb(vector<double> input,vector<double> &output) {							
	// Medium Room Reverb 
	// input: Mono audio data packed with Vector
	// output: The Vector's reference of a output vector,stored the processed audio data.

	// This function implement the reverb introduced in http://gdsp.hf.ntnu.no/lessons/6/36/
	// Reference:
	// The Gardner's paper：The virtual acoustic room, chapter 4

	// lpf coeff (3000 hz buttorth lowpass filter)
	double coeff_lpf[2][3] = { { 0.0299545822080925,0.0599091644161849,0.0299545822080925 },{ 1 ,-1.45424358625159,0.574061915083955 } };
	// 500~1000 hz bandpass filter 
	double coeff_bpf[2][3] = { { 0.0344079415548157,0,- 0.0344079415548157 },{ 1,- 1.92138780564584,0.931184116890369 } };
	double state_lpf[3] = { 0 };	
	double state_bpf[3] = { 0 };

	// Auxiliary variable for the reverb
	double delay_line[300 * 44] = {0};				// initialize the delay line, max delay time is 300 ms
	int head = 0;
	int tail = 0;
	int tap_ms = 44;								// tap_ms points per ms,that is per tap
	int len = 300 * 44;
	double bpf_out = 0;
	double lpf_out = 0;

	for (int n = 0; n != input.size(); ++n) {
		
		
		head = ( head + 1) % len;					// updata the index which data to be pushed in
		delay_line[head] = input[n];				// push data in and update the delay line
		// lpf
		biquad(delay_line[head], lpf_out, coeff_lpf, state_lpf);	

		delay_line[head] = lpf_out + bpf_out * 0.5;

		// Delay line based allpass network medium room reverb
		sampleDelayLineBasedAllpass(delay_line[(head + 0 * tap_ms) % len], delay_line[(head + 35 * tap_ms) % len], 0.25);
		sampleDelayLineBasedAllpass(delay_line[(head + 2 * tap_ms) % len], delay_line[(head + int(10.3 * tap_ms)) % len], 0.35);
		sampleDelayLineBasedAllpass(delay_line[(head + 3 * tap_ms) % len], delay_line[(head + 25 * tap_ms) % len], 0.45);
		double x1 = delay_line[(head + 35 * tap_ms) % len];		

		sampleDelayLineBasedAllpass(delay_line[(head + 40 * tap_ms) % len], delay_line[(head + 70 * tap_ms) % len], 0.45);
		double x2 = delay_line[(head + 137 * tap_ms) % len];	

		delay_line[(head + 152 * tap_ms) % len] = delay_line[(head + 152 * tap_ms) % len] * 0.4 + lpf_out;		
		sampleDelayLineBasedAllpass(delay_line[(head + 152 * tap_ms) % len], delay_line[(head + 191 * tap_ms) % len], 0.25);
		sampleDelayLineBasedAllpass(delay_line[(head + 153 * tap_ms) % len], delay_line[(int(head + (153 + 9.8) * tap_ms)) % len], 0.35);
		double x3 = delay_line[(head + 191 * tap_ms) % len];
		
		// bpf
		biquad( 0.4 * delay_line[tail] , bpf_out,coeff_bpf,state_bpf);		
	
		// output
		output[n] = 0.5 * (x1 + x2 + x3);	

		tail = head;
	}

	return 0;
};		
int gardnerReverb(vector<double> input, vector<double> &output);
int main()
{

	AudioFile<double> af;
	// The location of the .wav file
	std::string current_dir = "D:\\Project\\Repository\\Reverb\\AllpassReverb\\AllpassReverb\\assets";	
	std::string audioFile = current_dir + "\\first.wav";	// your .wav file dir
	af.load(audioFile);
	af.printSummary();

	vector<double> left(af.samples[0]);
	vector<double> right(af.samples[1]);
	int fs = af.getSampleRate();
	int tap_ms = fs / 1000;


	cout << "ready to process" << endl;
	vector<vector<double>> output(2);						// Vector of vector must assign the size
	output[0].assign(left.size(), 0);						// initialize the vector
	output[1].assign(right.size(), 0);
	for (int n = 0; n != left.size(); ++n) {		// stere to mono
		left[n] = (left[n] + right[n]) / 2;
	}

	
	gardnerReverb(left, output[0]);
	gardnerReverb(right, output[1]);
	

	//output[1].assign(output[0].begin(),output[0].end());	// 简单起见，将左通道的直接复制到右通道
	// output the signal
	af.setAudioBuffer(output);
	cout << "ready to output" << endl;
	af.save(current_dir+"\\first_test_reverb_class_12.wav");

	return 0;
}


double biquad(double input, double &output, double coeff[][3], double *state) {
	// biquad filter
	// coeff[0][-] B  ; coeff[1][-] A
	// 需要使用延时线，延时线 state 的长度为 filter_order + 1 ,比如 2 阶的延时线为 3
	// state[n]: 延时线中的第n号元素
	// updata the state(delay line),由于每次都需要更新延时线，因此速度上会有所损失
	// 在优化时可以考虑更新使用标志位（循环队列）
	//ref： https://en.wikipedia.org/wiki/Digital_biquad_filter


	state[2] = state[1];
	state[1] = state[0];
	state[0] = input + (-coeff[1][1]) * state[1] + (-coeff[1][2]) * state[2];
	// caculate the output
	output = coeff[0][0] * state[0] + coeff[0][1] * state[1] + coeff[0][2] * state[2];
	return 0;

	/*
	Usage:
	// Design a 3000 hz lowpass filter in the Matlab for this test.
	double coeff[2][3] = { { 0.0299545822080925,0.0599091644161849,0.0299545822080925 },{ 1 ,-1.45424358625159,0.574061915083955 } };
	double state[3] = { 0 };
	for (int n = 0; n != INPUT.size(); ++n) {
	biquad(INPUT[n], OUTPUT[n], coeff, state);
	}
	*/
}
double fir_init(double tap_am[][20],double virtal_distance, double real_distance, double phi, double theta, double absort_coef)
{
	// virtal_distance : 虚拟音源与听者的距离  单位 m
	// real_distance : 两个喇叭距离听者的距离（A B喇叭视为一样）
	// phi ：两个喇叭的角度AOB	，单位为角度，参考值为 60 度
	// theta : 虚拟音源与A的夹角 AOV		参考值为 20 度
	// absort_coeff： 墙体的吸收系数， 参考值为 0.85
	// PS：默认 4 面墙
	double pi = 3.141592;
	phi = phi / 180 * pi;			// 将角度转换为弧度制
	theta = theta / 180 * pi;
	double alpha = absort_coef * absort_coef * absort_coef * absort_coef;

	//double tap_am[2][20] = { 0 };

	for (int n = 0; n != 20; ++n) {
		tap_am[0][n] = alpha * (n + 1) / real_distance * cos(pi * theta / (2 * pi)) / virtal_distance;
		tap_am[1][n] = alpha * (n + 1) / real_distance * sin(pi * theta / (2 * pi)) / virtal_distance;
	}
	//vector<vector<double>> tap(tap_am[0], tap_am[0] + 1);
	return 0.0;
}
int early_echo_fir(double input, double & output, double coeffs[], double state[],int taps,int & counter)
{
	// FIR 阶数为 N , taps = N + 1
	// input  : 输入的数据，点数据
	// output : 输出的数据, 点数据
	// coeffs[]: 保存着 FIR 的系数，系数个数为 taps 个，即 N+1 个系数
	// state[]: 保存着 x[n] x[n-1] x[n-2]...x[n-N],总计 N + 1 个数据，
	//			这是一个使用循环队列实现的延时线，使用前需要初始化为 0 
	// counter: 辅助实现循环队列，是否++需要视情况而定
	// 注意，数组不能引用，因此无法修改state中的内容，此函数需要在外部辅助修改 state
	//  double &state[]  与 double (&state)[]
	double scale = 0.25;
	output = 0;
	state[(++counter)%taps] = scale * input;		// 这个操作有可能需要移到外边去
	// output += b0 * input;
	for (int n = 0; n != taps; ++n) {
		output += state[ (n + counter) % taps] * coeffs[n];
	}
	return 0;
}
int gardnerReverb(vector<double> input, vector<double>& output)
{
	// The gardner's reverb algorithm
	// input : THe total mono audio input
	// output: The output data
	//	input -> lp -> fir -> reverb -> iir_gain -> output

	// 1500 hz lowpass butterworth filter,fs = 48e3 hz
	Biquad lpf;
	double lpf_coeffs[2][3] = { { 0.00844269292907995,0.0168853858581599,0.00844269292907995 },
								{ 1,- 1.72377617276251,0.757546944478829 } };	
	lpf.setCoeffs(lpf_coeffs[0], lpf_coeffs[1]);	// setting the lpf coefficients

	FIR fir;
	double fir_coeff_ab[2][20] = { 0 };
	fir_init(fir_coeff_ab, 6, 4, 60, 45, 0.85);		// campute the early echo fir coefficients
	fir.init(20, fir_coeff_ab[0]);

	Reverberator medium_reverb;
	medium_reverb.init(44100);

	double lpf_out = 0;
	double fir_out = 0;
	double reverb_out = 0;
	int n = 0;

	double IIR_gain = 0.7;
	double rate = 0.3; // dry wet rate
	for (auto data : input) {
		// pre lpf
		lpf.filter(data, lpf_out);
		// fir filter
		fir.filter(lpf_out, fir_out);

		// the reverb 
		medium_reverb.reverb(fir_out, reverb_out);
		
		// output the signal
		output[n++] = data * rate +  ( 1 - rate ) * ( fir_out +  reverb_out * IIR_gain );
		//output[n++] = ( fir_out +  reverb_out * IIR_gain );
	}

	return 0;
};



vector<double> delay_line_mode(int tap_ms, int total_samples, vector<double> &data_in)
{
	// 一个简单的测试例子，测试 sampleDelayLineBasedAllpass 函数以及基础 delay line - passed

	// use a single delay line to implement the sample delay line demo - passed 2018年9月7日18:53:32
	// The schematic in figure 4.8.
	// e.g.	output[0] = delay_line_mode(tap_ms, af.getNumSamplesPerChannel(), left);
	//		output[1] = delay_line_mode(tap_ms, af.getNumSamplesPerChannel(), right);

	int delay_len = 110 * tap_ms;
	double * delay_line = new double[delay_len];				// initialize the delay line,the length is 110*tap_ms
	for (int count = 0; count != delay_len; ++count) {
		delay_line[count] = 0;
	}
	int head = 0;												// the next position to push data in
	int tail = 0;
	vector<double> out;
	for (int n = 0; n != total_samples; ++n) {

		head = ( (head+1)*tap_ms ) % delay_len;	
		out.push_back(delay_line[head]);						// pop(),output the previous sample data
		delay_line[head] = data_in[n];							// push() data in,update the current data at the head index
							
		// According to the reverb model solution,update the data in the delay line
		sampleDelayLineBasedAllpass( delay_line[(head + 25 * tap_ms) % delay_len], delay_line[(head + 75 * tap_ms) % delay_len], 0.50);
		sampleDelayLineBasedAllpass( delay_line[(head + 40 * tap_ms) % delay_len], delay_line[(head + 60 * tap_ms) % delay_len], 0.30);
		sampleDelayLineBasedAllpass( delay_line[(head + 80 * tap_ms) % delay_len], delay_line[tail], 0.70);
		
		// update the tail index for easier understandind
		tail = head;
	}

	return out;
}

int sampleDelayLineBasedAllpass(double & data_in, double & data_out, double gain)
{
	// 在 allpass delay line 中使用
	// The implement of the sample delay line based allpass filter.
	// H_z = (G_z - g ) / (1 - g * G_z), G_z is the delay_line work independently outside the function.
	// The length of delay line is the distance between "data_in" and "data_out".
	double temp = data_in * (-gain) + data_out;
	data_in = data_in + gain * temp;
	data_out = temp;
	return 0;
}

//int first_order_lowpass(double fc, double *buff, double current)
//{
//	// 弃用，改成 biquad
//	// buff[0]:the output, current:the input, buff[1]:previous output
//	double alpha = 1.0 / (1.0 + 2 * 3.1415926 * fc);
//	buff[0] = alpha * buff[1] + (1 - alpha)*current;
//	buff[1] = buff[0];
//	return 0;
//}

int Biquad::setCoeffs(double * b, double * a)
{
	for (int n = 0; n != 3; ++n) {
		coeffs[0][n] = *(b + n);
		coeffs[1][n] = *(a + n);
	}
	return 0;
}

int Biquad::reset()
{
	state[3] = { 0 };
	return 0;
}

double Biquad::filter(double input, double & output)
{
	// 单点滤波
	state[2] = state[1];
	state[1] = state[0];
	state[0] = input + (-coeffs[1][1]) * state[1] + (-coeffs[1][2]) * state[2];
	// compute the output
	output = coeffs[0][0] * state[0] + coeffs[0][1] * state[1] + coeffs[0][2] * state[2];
	return 0;
}

double Biquad::filter(vector<double> input, vector<double>& output)
{
	for (int n = 0; n != input.size();++n) {
		Biquad::filter(input.at(n), output.at(n));
	}
	return 0.0;
}

int FIR::init(int taps_num, double * coeffs_val)
{
	// initialize the fir filter, setting the tap number and the coefficients.
	taps = taps_num;
	//coeffs =(double *)malloc( sizeof(double)*taps );
	coeffs = new double[taps];	// 动态创建数组
	for (int n = 0; n != taps; ++n) {
		coeffs[n] = coeffs_val[n];
	}
	state = new double[taps] {0};
	return 0;
}

int FIR::filter(double input, double & output)
{
	// 单点滤波
	output = 0;
	state[(++counter) % taps] = scale * input;		// 这个操作有可能需要移到外边去
													// output += b0 * input;
	for (int n = 0; n != taps; ++n) {
		output += state[(n + counter) % taps] * coeffs[n];
	}
	return 0;
}

int Reverberator::init(unsigned int sample_freq)
{
	fs = sample_freq;
	tap = int(fs / 1000);
	len = 299 * tap;
	delay_line = new double[len]{0}; // initialize the delay line 
	double lpf_coeff[2][3] = { { 0.0251761145544014,0.0503522291088028,0.0251761145544014 },
								{ 1,- 1.50369534129922,0.604399799516827 } };
	lpf.setCoeffs(lpf_coeff[0], lpf_coeff[1]);
	head = 0;
	tail = 0;
	return 0;
}

int Reverberator::reverb(double input, double & output)
{
	double lpf_out = 0;
	head = (head + 1) % len;					// updata the index which data to be pushed in
	delay_line[head] = input;					// push data in and update the delay line
												// lpf
	lpf.filter(delay_line[tail], lpf_out);
	delay_line[head] = 0.4 * lpf_out + delay_line[head]; // 注释用来测试

	// Delay line based allpass network medium room reverb
	sampleDelayLineBasedAllpass(delay_line[(head + 0 * tap) % len], delay_line[(head + 35 * tap) % len], 0.25);
	sampleDelayLineBasedAllpass(delay_line[(head + 2 * tap) % len], delay_line[(head + int(10.3 * tap)) % len], 0.35);
	sampleDelayLineBasedAllpass(delay_line[(head + 3 * tap) % len], delay_line[(head + 25 * tap) % len], 0.45);
	double x1 = delay_line[(head + 35 * tap) % len];

	sampleDelayLineBasedAllpass(delay_line[(head + 40 * tap) % len], delay_line[(head + 70 * tap) % len], 0.45);
	double x2 = delay_line[(head + 137 * tap) % len];

	delay_line[(head + 152 * tap) % len] = delay_line[(head + 152 * tap) % len] * 0.4 + delay_line[head];
	sampleDelayLineBasedAllpass(delay_line[(head + 152 * tap) % len], delay_line[(head + 191 * tap) % len], 0.25);
	sampleDelayLineBasedAllpass(delay_line[(head + 153 * tap) % len], delay_line[(int(head + (153 + 9.8) * tap)) % len], 0.35);
	double x3 = delay_line[(head + 191 * tap) % len];
	
	// output
	output = 0.5 * (x1 + x2 + x3);
	//output = 0.6; // for test

	tail = head;

	return 0;
}
