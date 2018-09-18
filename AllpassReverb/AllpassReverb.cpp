// AllpassReverb.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "AudioFile.cpp"
#include <iostream>
#include <vector>
#include <string>
#include "AllpassReverbBased.h"
using namespace std;

vector<double> delay_line_mode(int tap_ms, int total_samples, vector<double> &data_in);	// 测试样例一  - passed
int sampleDelayLineBasedAllpass(double &data_in, double &data_out, double  gain);		// paper 1. 基础结构，用于各种样例中
//int first_order_lowpass(double fc, double *buff, double current); //- 弃用
double biquad(double input, double &output, double coeff[][3], double *state);			// 用于辅助测试样例二

int mediumReverb(vector<double> input,vector<double> &output) {							// 测试样例二，中等房间混响
	// 中等房间混响  最新需求 2018年9月13日14:14:50
	// initialize the lpf
	// initialize the delay line

	// lpf coeff (3000 hz buttorth lowpass filter)
	double coeff_lpf[2][3] = { { 0.0299545822080925,0.0599091644161849,0.0299545822080925 },{ 1 ,-1.45424358625159,0.574061915083955 } };
	// 500~1000 hz bandpass filter 
	double coeff_bpf[2][3] = { { 0.0344079415548157,0,- 0.0344079415548157 },{ 1,- 1.92138780564584,0.931184116890369 } };
	double state_lpf[3] = { 0 };	
	double state_bpf[3] = { 0 };

	double delay_ms = 44;				//int(44100 / 1000)
	double delay_line[300 * 44] = {0};	//299
	int head = 0;
	int tail = 0;
	int tap_ms = 44;
	int len = 300 * 44;

	double bpf_out = 0;
	double lpf_out = 0;

	for (int n = 0; n != input.size(); ++n) {

		// update the delay line
		//head = ( (head + 1)*tap_ms) % len;
		head = ( head + 1) % len;// 重点测试

		delay_line[head] = input[n];		// 将数据读取到延时线中

		// lpf

		biquad(delay_line[head], lpf_out, coeff_lpf, state_lpf);	
		delay_line[head] = lpf_out + bpf_out * 0.5;

		// Delay line based allpass network reverb
		// Just implement a network for test
		sampleDelayLineBasedAllpass(delay_line[(head + 0 * tap_ms) % len], delay_line[(head + 35 * tap_ms) % len], 0.25);
		sampleDelayLineBasedAllpass(delay_line[(head + 2 * tap_ms) % len], delay_line[(head + int(10.3 * tap_ms)) % len], 0.35);
		sampleDelayLineBasedAllpass(delay_line[(head + 3 * tap_ms) % len], delay_line[(head + 25 * tap_ms) % len], 0.45);
		double x1 = delay_line[(head + 35 * tap_ms) % len];		// tap1 passed

		sampleDelayLineBasedAllpass(delay_line[(head + 40 * tap_ms) % len], delay_line[(head + 70 * tap_ms) % len], 0.45);
		double x2 = delay_line[(head + 137 * tap_ms) % len];	// tap2 passed

		delay_line[(head + 152 * tap_ms) % len] = delay_line[(head + 152 * tap_ms) % len] * 0.4 + lpf_out;		// 这地方是 0.4 不是 4
		sampleDelayLineBasedAllpass(delay_line[(head + 152 * tap_ms) % len], delay_line[(head + 191 * tap_ms) % len], 0.25);
		sampleDelayLineBasedAllpass(delay_line[(head + 153 * tap_ms) % len], delay_line[(int(head + (153 + 9.8) * tap_ms)) % len], 0.35);
		double x3 = delay_line[(head + 191 * tap_ms) % len];


		// bpf
		biquad( 0.4 * delay_line[tail] , bpf_out,coeff_bpf,state_bpf);		// 500-1000 hz bpf 
		//bpf_out = 0;																	// output
		output[n] = 0.5 * (x1 + x2 + x3);	// 输出

		//delay_line[tail] = bpf_out * 0.7 + lpf_out;


		tail = head;




		/*
		// 原先的
		sampleDelayLineBasedAllpass(delay_line[(head + 0 * tap_ms) % len], delay_line[(head + 35 * tap_ms) % len], 0.25);
		sampleDelayLineBasedAllpass(delay_line[(head + 1 * tap_ms) % len], delay_line[(int(head + 9.3 * tap_ms)) % len], 0.35);
		sampleDelayLineBasedAllpass(delay_line[(head + 3 * tap_ms) % len], delay_line[(head + 25 * tap_ms) % len], 0.45);
		double x1 = delay_line[(head + 35 * tap_ms) % len];
		sampleDelayLineBasedAllpass(delay_line[(head + 40 * tap_ms) % len], delay_line[(head + 70 * tap_ms) % len], 0.45);
		double x2 = delay_line[(head + 137 * tap_ms) % len];
		delay_line[(head + 152 * tap_ms) % len] = delay_line[(head + 152 * tap_ms) % len] * 4 +delay_line[head];
		sampleDelayLineBasedAllpass(delay_line[(head + 152 * tap_ms) % len], delay_line[(head + 191 * tap_ms) % len], 0.25);
		sampleDelayLineBasedAllpass(delay_line[(head + 153 * tap_ms) % len], delay_line[(int(head + (153+9.8) * tap_ms)) % len], 0.35);
		double x3 = delay_line[(head + 191 * tap_ms) % len];
		
		*/
	}

	cout << "The data in delay line :" << endl;
	int count = 0;
	for (int n = 0; n != len; ++n) {
		if (delay_line[n] > 0.001 || delay_line[n] < -0.001) {
			cout << n << "     : " << delay_line[n] << endl;
			if (++count > 30)
				return 0;
		}
	}
	return 0;
};
int main()
{

	AudioFile<double> af;
	// The location of the .wav file
	std::string current_dir = "D:\\Project\\Repository\\Reverb\\AllpassReverb\\AllpassReverb\\assets";	
	std::string audioFile = current_dir + "\\杨章隐 - 梦中的婚礼.wav";	// your .wav file dir
	af.load(audioFile);
	af.printSummary();

	vector<double> left(af.samples[0]);
	vector<double> right(af.samples[1]);
	int fs = af.getSampleRate();
	int tap_ms = fs / 1000;


	cout << "ready to process" << endl;
	vector<vector<double>> output(2);						// Vector of vector must assign the size
	output[0].assign(left.size(), 1);						// initialize the vector
	output[1].assign(right.size(), 1);
	// test mediun reverb
	mediumReverb(left, output[0]);
	mediumReverb(right, output[1]);


	// test the biquad -pass
	

	// output the signal
	af.setAudioBuffer(output);
	cout << "ready to output" << endl;
	af.save(current_dir+"\\杨章隐 - 梦中的婚礼_medium room reverb.wav");

	return 0;
}
double biquad(double input, double &output, double coeff[][3], double *state) {
	// coeff[0][-] B  ; coeff[1][-] A
	// 需要使用延时线，延时线 state 的长度为 filter_order + 1 ,比如 2 阶的延时线为 3
	// state[n]: 延时线中的第n号元素
	// updata the state(delay line),由于每次都需要更新延时线，因此速度上会有所损失
	// 在优化时可以考虑更新使用标志位（循环队列）
	// 注意，只支持二阶滤波器

	state[2] = state[1];
	state[1] = state[0];
	state[0] = input + (-coeff[1][1]) * state[1] + (-coeff[1][2]) * state[2];
	// caculate the output
	output = coeff[0][0] * state[0] + coeff[0][1] * state[1] + coeff[0][2] * state[2];
	return 0;
	//ref： https://en.wikipedia.org/wiki/Digital_biquad_filter

	/*
	usaged:
	// Design a 3000 hz lowpass filter in the Matlab for this test.
	double coeff[2][3] = { { 0.0299545822080925,0.0599091644161849,0.0299545822080925 },{ 1 ,-1.45424358625159,0.574061915083955 } };
	double state[3] = { 0 };
	for (int n = 0; n != INPUT.size(); ++n) {
	biquad(INPUT[n], OUTPUT[n], coeff, state);
	}
	*/
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
