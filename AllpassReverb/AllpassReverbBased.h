#pragma once
#include <iostream>
#include <vector>
#include <string>
using namespace std;
class queue {
public:
	int len = 0;
	bool is_empty = true;
	int head = 0;							// Current location of the head.
	int tail = head;						// The location of tail,which is the next index to push data.
	double *array = NULL;					// The pointer  point to the array which store datas.

	double push(double data);
	// double push(int push_times,double data);		// Todo: push the 'data' into queue 'push_times' times
	// double update(int index,double data);		// Todo: update the data in the index of  queue
	// double at(index);							// Todo: return the data in the index of the queue
	double pop(double data);
	bool isEmpty();
	bool isFull();
	void print();
	queue(int array_len, double init_value);
	queue(int array_len);
	~queue();
};

vector<double>  sample_delay_line(int tap_ms, int total_samples, vector<double> data_in) {

	// The implement of the sample delay line demo in the paper:
	// 'The Virtual Acoustic Room - William Grant Gardner 1982 - Thesis.pdf '
	vector<double> data_out;
	queue d_25(25 * tap_ms, 0);
	queue d_50(50 * tap_ms, 0);
	queue d_20(20 * tap_ms, 0);
	queue d_5(5 * tap_ms, 0);
	queue d_30(30 * tap_ms, 0);
	cout << "ready to process" << endl;
	for (int n = 0; n != total_samples; ++n) {
		double t1 = d_25.push(data_in.at(n));
		double t2 = d_50.push(0.5*t1);
		double t3 = d_20.push(0.3*t1);
		double t4 = d_5.push(t2 + t3);
		double t5 = d_30.push(0.7*t4);
		data_out.push_back(t5);
	};
	return data_out;
}
double queue::push(double data)
{
	if (isEmpty()) {
		//cout << "empty" << endl;
		array[head] = data;
		is_empty = false;
		tail = (tail + 1) % len;	// update head and tail
		return -1;
	}
	double temp = 0;
	if (isFull()) {
		// overflowed
		temp = pop(head);			// pop the head
		push(data);
		return temp;
	}
	else {
		array[tail] = data;
		tail = (tail + 1) % len;	// update head and tail
		return -1;
	}
	return -1;
}

double queue::pop(double data)
{
	double temp = 0;
	if (isEmpty()) {

		return -1;

	}
	else {
		temp = array[head];
		head = (head + 1) % len;
		if (head == tail)			// update is_empty flag
			is_empty = true;
		return temp;
	}
}

bool queue::isEmpty()
{
	return is_empty;

}

bool queue::isFull()
{
	//return (tail + 1) % len == head;
	return (tail == head) && (!is_empty);	// 非空且含有元素的时候
}

void queue::print()
{
	// print the whole datas in queue.
	for (int n = 0; n != len; ++n) {
		std::cout << array[n];
		std::cout << " ";
	}
}

queue::queue(int array_len, double init_value)
{
	// initialize a array_len 's queue with init_value.
	len = array_len;				// update sequeue's len
	array = new double[len];
	for (int n = 0; n != len; ++n) {
		push(init_value);			// used the push function to initialize the queue
	}
}

queue::queue(int array_len)
{
	// initialize a array_len 's queue ,and the queue is empty.
	len = array_len;
	array = new double[len];
}

queue::~queue()
{
	delete[] array;
}
