/**
 * @file DBSCANModuleMain.cpp
 * @brief ...
 *
 * This file is created at Almende B.V. It is open-source software and part of the Common 
 * Hybrid Agent Platform (CHAP). A toolbox with a lot of open-source tools, ranging from 
 * thread pools and TCP/IP components to control architectures and learning algorithms. 
 * This software is published under the GNU Lesser General Public license (LGPL).
 *
 * It is not possible to add usage restrictions to an open-source license. Nevertheless,
 * we personally strongly object against this software being used by the military, in the
 * bio-industry, for animal experimentation, or anything that violates the Universal
 * Declaration of Human Rights.
 *
 * Copyright © 2012 Your Name <your@email.address>
 *
 * @author  ...
 * @date    ...
 * @company Almende B.V. (or your university, institute or company)
 * @case    Artificial Intelligence Module
 */
#include <DBSCANModuleExt.h>

#include <stdlib.h>
#include <iostream>

#include <stdio.h>
#include <sstream>
#include <math.h>	// math
#include <fstream>	// file IO
#include <string>	// string
#include <time.h>	// profiling
#include <stdarg.h>

#include <array_libraries.h>

using namespace rur;
using namespace std;

void mergeClusters(array2d<int>& indices, int old_cluster, int new_cluster) {
	for (int i = 0; i < indices.h; i++) {
		if (indices(1,i) == new_cluster) {
			indices(1,i) = old_cluster;
		}
	}
}

void checkForUniqueClusters(array2d<int>& indices, int cluster_id) {
	int amount_of_clusters = 0;
	array1d<int> clusters(cluster_id); clusters.setTo(-1);

	for (int i = 0; i < indices.h; i++) {
		clusters(indices(1,i)) = 1;
	}


	for (int i = 1; i < clusters.w; i++) {
		if (clusters(i) == 1) {
			amount_of_clusters += 1;
		}
	}
	std::cout << "Amount of clusters:" << amount_of_clusters << std::endl;
}

void getDimensionsOfData(std::string filename, int& x, int& y) {
	int max_x = 0;
	std::ifstream datafile(filename.c_str());
	std::string line;
	while(std::getline(datafile,line))
	{
		std::stringstream  lineStream(line);
		std::string        cell;
		x = 0;
		while(std::getline(lineStream,cell,','))
		{
			x += 1;
		}
		if (x > max_x)
			max_x = x;
		y += 1;
	}
	datafile.close();

}

void determineRand(array2d<int>& indices, array2d<float>& dataset, int cluster_id) {
	std::cout << "Calculating Rand Index" << std::endl;

	array1d<int> clusters(cluster_id); clusters.setTo(-1);
	array2d<float> visualization(dataset.w,dataset.h);
	for (int i = 0; i < dataset.w-1; i++) {
		for (int j = 0; j < dataset.h; j++) {
			visualization(i,j) = dataset(i,j);
		}
	}
	for (int j = 0; j < dataset.h; j++) {
		visualization(visualization.w-1,j) = indices(indices.w-1,j);
	}
	visualization.writeToFile("visualization.data");

	int match = 0;
	int fail = 0;

	int a = 0;
	int b = 0;
	int c = 0;
	int d = 0;

	//indices.writeToFile("indices.txt");
	for (int i = 0; i < dataset.h; i++) {
		clusters(indices(1,i)) = dataset(dataset.w-1,i);
	}
	//clusters.print();
	/*
	for (int i = 0; i < dataset.h; i++) {
		if (clusters(indices(1,i)) == dataset(dataset.w-1,i))
			match += 1;
		else
			fail += 1;
	}

	std::cout << "match: " << match << "\tfail: " << fail << std::endl;
	*/
	for (int i = 0; i < dataset.h; i++) {
		for (int j = i; j < dataset.h; j++) {

			if (indices(1,i) == indices(1,j)) {
				// in the same set in X
				if (dataset(dataset.w-1,i) == dataset(dataset.w-1,j)) {
					// in the same set in Y
					a += 1;
				}
				else {
					// in a different set in Y
					c += 1;
				}
			}
			else {
				// in a different set in X
				if (dataset(dataset.w-1,i) == dataset(dataset.w-1,j)) {
					// in the same set in Y
					d += 1;
				}
				else {
					// in a different set in Y
					b += 1;
				}
			}

		}
	}

	double rand = double(a+b)/double(a+b+c+d);

	std::cout << "A: " << a << "\t B: " << b << "\tC: " << c << "\t D: " << d << std::endl;
	std::cout << "Rand index: " << rand << std::endl;

	std::cout << std::endl;
}

/**
 * Every module is a separate binary and hence has its own main method. It is recommended
 * to have a version of your code running without any middleware wrappers, so preferably
 * have this file and the DBSCANModule header and code in a separate "aim" directory.
 */
int main(int argc, char *argv[])  {
	DBSCANModuleExt *m = new DBSCANModuleExt();

	std::cout << "Amount of arguments " << argc << std::endl;

	int neighbours_threshold = 4;
	float distance_threshold = 100;
	int print_dist = 0;

	// neighbours 3 and distance 0.165 is best for abalone3

	if (argc >= 3) {
		neighbours_threshold = atoi(argv[1]);
		distance_threshold = atof(argv[2]);
		std::cout << "neighbours: " << neighbours_threshold << std::endl;
		std::cout << "distance_threshold: " << distance_threshold << std::endl;
	}

	if (argc == 4) {
		print_dist = atoi(argv[3]);
		std::cout << "print_dist: " << 1 << std::endl;
	}
	clock_t begin = clock();

	std::cout << std::endl;


	int cluster_id = 1;

	int x = 0;
	int y = 0;

	std::string dataset_name = "abalone3.data";
	// get dimensions
	getDimensionsOfData(dataset_name,x,y);

	std::cout << "Dataset Dimensions: " << x << "," << y << std::endl;


	array2d<int> 	indices(2,y); indices.setTo(0);// visited or noise, clusterID
	array2d<float> 	dataset(x,y);
	array1d<float>  normalization_distances(x);

	// do not take the cluster id into account
	x = x - 1;

	dataset.loadFromRawFile(dataset_name);

	for (int i = 0; i < x; i++) {
		normalization_distances(i) = 1.0/(dataset.getColumnMean(i));
		//std::cout << "normalized distance factors: " << normalization_distances(i) << std::endl;
	}


	int amount_of_neighbours;
	float distance;

	for (int i = 0; i < y; i++) {
		// Start with an arbitrary starting point that has not been visited.
		if (indices(0,i) == 0) {
			// Extract the neighborhood of this point using ε (All points which are within the ε distance are neighborhood).
			amount_of_neighbours = 0;
			for (int j = i+1; j < y; j++) {
				distance = 0;
				// check the distance in all directions
				for (int k = 0; k < x; k++) {
					// the distances are normalized
					distance += pow((dataset(k,i) - dataset(k,j))*normalization_distances(k),2);
				}
				//std::cout << sqrt(distance) << std::endl;
				if (sqrt(distance) <= distance_threshold) {
					if (print_dist == 1)
						std::cout << sqrt(distance) << std::endl;
					amount_of_neighbours += 1;
					// if a neighbour is in a cluster, so is this little guy, he starts clustering his friends
					if (indices(1,j) != 0) {
						indices(0,i) = 1;
						indices(1,i) = indices(1,j);
						break;
					}
				}

				// If there are sufficient neighborhood around this point then clustering process starts and
				// point is marked as visited else this point is labeled as noise
				// 	(Later this point can become the part of the cluster).
				if (amount_of_neighbours == neighbours_threshold) {
					indices(0,i) = 1;
					break;
				}

			}
			if (indices(0,i) == 1) {
				// create cluster
				if (indices(1,i) == 0) {
					indices(1,i) = cluster_id;
					cluster_id += 1;
				}

				// clustering
				for (int j = i+1; j < y; j++) {
					distance = 0;
					// check the distance in all directions
					for (int k = 0; k < x; k++) {
						distance += pow((dataset(k,i) - dataset(k,j))*normalization_distances(k),2);
					}

					if (sqrt(distance) <= distance_threshold) {
						if (indices(1,j) == 0) {
							// if this guy is not in a cluster
							indices(1,j) = indices(1,i);
						}
						else {
							if (indices(1,j) != indices(1,i))
								mergeClusters(indices, indices(1,j), indices(1,i));
						}
					}
				}
			}
		}
	}

	checkForUniqueClusters(indices,cluster_id);
	determineRand(indices,dataset,cluster_id);

	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

	std::cout << "time elapsed: " << elapsed_secs << std::endl;

	/*
	std::string identifier = argv[1];
	m->Init(identifier);

	do {
		m->Tick();
	} while (!m->Stop()); 
	*/
	delete m;

	return EXIT_SUCCESS;
}
