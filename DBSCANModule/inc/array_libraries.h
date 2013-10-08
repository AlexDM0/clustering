#pragma once


#include <vector>

/**********************************************************************************************************/
/***** Basically, an array3d is a 2d array for all threads, an array2d is an 1d array for all threads *****/
/****************** the Managers are used to contain the arrays for different datasets  *******************/
/**********************************************************************************************************/

// this is the master class, all the others are derivatives of this (on the HOST side, the CUDA side has the cuArray)
template<typename T>
class array3d {
	public:
		//int ID;
		int w,h,z;
		bool pointer_only;
		T* data;
		

		array3d(int width,int height,int depth) { //constructor
			w = width; h = height; z = depth;
			data=new T[w*h*z];
			pointer_only = false;
		}

		array3d(int width,int height,int depth, T* data) { //constructor
			w = width; h = height; z = depth;
			this->data = data;
			pointer_only = true;
		}

		void setDefaultSequence() {
			for (int i = 0; i < w; i++) {
				data[i] = i;
			}
		}

		T getMax() {
			T max_value = -1;
			for (int i = 0; i<w*h*z; i++) {
				if (max_value < data[i])
					max_value = data[i];
			}
			return max_value;
		}

		T getNonZeroMin() {
			double min_value = 1e40;
			for (int i = 0; i<w*h*z; i++) {
				if (min_value > data[i] && data[i] != 0)
					min_value = data[i];
			}
			return min_value;
		}

		T getNonZeroAverage() {
			double sum = 0;
			double steps = 0;
			for (int i = 0; i<w*h; i++) {
				if (data[i] != 0) {
					sum += data[i];
					steps += 1;
				}
			}
			return sum/steps;
		}

		~array3d() { //destructor
			if (pointer_only == false) {
				delete[] data;
				//printf("I deleted myself! \n");
			}
		}

		void set(T inputarray[]) { //assigner
			for (int i=0;i<w*h*z;i++) {
				data[i] = inputarray[i];
			}
		}
		
		void setTo(T a) {
			for (int i=0;i<w*h*z;i++)
				data[i] = a;
		}

		void setForAll(T inputarray[]) { //assign for all threads
			for (int i=0;i<w*h;i++) {
				for (int j=0; j<z;j++) {
					data[i + j*w*h] = inputarray[i];
				}
			}
		}

		T& operator()(int x,int y,int z) { //getData
			return data[x + y*w + z*w*h];
		}

		void loadFromRawFile(std::string filename) {
			int datapoint = 0;
			std::ifstream datafile(filename.c_str());
			std::string line;
			while(std::getline(datafile,line))
			{
				std::stringstream  lineStream(line);
				std::string        cell;
				while(std::getline(lineStream,cell,','))
				{
					this->data[datapoint] = atof(cell.c_str());
					datapoint += 1;
				}
			}
			datafile.close();
		}

		void writeToFile(std::string filename) {
			std::ofstream myfile;
			for (int k=0; k < 1;k++) {
				myfile.open(filename.c_str());
				myfile.precision(15);
				for (int j=0; j < h;j++) {
					for (int i=0; i < w;i++) {
						if (data[i + j*w + k*w*h] != -1)
							myfile << data[i + j*w + k*w*h] << ",";
					}
					myfile << "\n";
				}
				myfile.close();
			}
		}

		 void print() { //printArray
			std::cout << "\n";
			for (int k=0; k < z; k++) {
				for (int j=0; j < h;j++) {
					for (int i=0; i < w;i++) {
						std::cout << "|";
						std::cout <<  data[i + w*j + w*h*k];
						std::cout << "|";
					}
					std::cout << "\n";
				}
			}
			std::cout << "\n";
		}


};

template<typename T>
class array2d : public array3d<T> {
public:

	explicit array2d(int width, int height) : array3d<T>(width,height,1) {
	}

	explicit array2d(int width, int height, T* data) : array3d<T>(width,height,1,data) {
	}

	T& operator()(int x,int y) { //getData
		if (x + y*this->w > this->w*this->h)
			printf("OUT OF BOUNDS! \n");
		return this->data[x + y*this->w];
	}

	T& operator()(int x) { //getData
		if (x > this->w*this->h)
			printf("OUT OF BOUNDS! \n");
		return this->data[x];
	}

	void setForAll(T inputarray[]) { //assign for all threads
		for (int i=0;i<this->w;i++) {
			for (int j=0; j<this->h;j++) {
				this->data[i + j*this->w] = inputarray[i];
			}
		}
	}

	void subtract(array2d<T>& a) {
		for (int i = 0; i < this->w*this->h; i++)
			this->data[i] -= a.data[i];
	}

	float getColumnMean(int column) {
		T mean = 0;
		for (int i = 0; i < this->h; i++) {
			mean += this->data[column + i*this->w];
		}
		return float(mean)/float(this->h);
	}

	~array2d() {

	}

};

template<typename T>
class array1d : public array3d<T> {
public:
	
	explicit array1d(int width) : array3d<T>(width,1,1) {
	}

	explicit array1d(int width,T* data) : array3d<T>(width,1,1,data) {
	}

	T& operator()(int x) { //getData
		if (x > this->w)
			printf("OUT OF BOUNDS! \n");

		return this->data[x];
	}

	~array1d() {

	}
};
