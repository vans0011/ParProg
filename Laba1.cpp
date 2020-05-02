
#include <mpi.h>
#include <vector>
#include <stdlib.h>
#include <random>
#include <ctime>
#include "iostream"
//6.Нахождение числа нарушений упорядоченности соседних элементов вектора

std::vector<int> getVector(int size);
int checkWrongOrder(std::vector<int> vector, int size);


int main(int argc, char** argv) {
	int rank, size;
	int sizeVec = rand() % 1000 + 100;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	//	MPI_Status status;
	int local_count, count = 0;
	std::vector<int> myVec;

	if (rank == 0) { // пункт 1 создать ОДИН вектор
		myVec = getVector(sizeVec);
	};
      //Пункт 2 разбить 
	int local_size = sizeVec / size;
	int tail = sizeVec % size;
	std::vector<int> locVector(local_size);

	MPI_Scatter(&myVec[0], local_size, MPI_INT, &locVector[0], local_size, MPI_INT, 0, MPI_COMM_WORLD);
	
	local_count = checkWrongOrder(locVector, local_size);
	std::cout << local_count << " First loc cheak  " << std::endl;
	if (rank == 0) {
		for (int i = 1; i < size; i++) {
			if (myVec[(local_size * i)] < myVec[local_size * i - 1]) {
				local_count++;
			}
		}
		for (int i = 0; i < tail; i++) {
			if (myVec[local_size*size + i] < myVec[(local_size*size) + i - 1])
				local_count++;

		};
	};
	//std::cout << local_count << " local cheak" << std::endl;
	MPI_Reduce(&local_count, &count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	if (rank == 0) {
		std::cout << count << " total cheak" << std::endl;
	}

	MPI_Finalize();

	std::cout << checkWrongOrder(myVec, sizeVec) << " TEST cheak";
	return 0;
};


std::vector<int> getVector(int size) {
	std::mt19937 gen;
	gen.seed(static_cast<unsigned int>(time(0)));
	std::vector<int> vec(size);
	for (int i = 0; i < size; i++) {
		vec[i] = gen() % 100;
		std::cout << vec[i] << " ";
	}
	std::cout << std::endl;
	return vec;
};


int checkWrongOrder(std::vector<int> vector, int size) {
	int count = 0;
	for (int c = 1; c < size; c++) {
		if (vector[c] < vector[c - 1]) {
			count++;
		}
	}
	return count;
};

