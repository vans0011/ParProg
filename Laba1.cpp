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
	int sizeVec = 1000;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Status status;
	  std::vector<int> myVec = getVector(sizeVec);
      int local_count = 0;
	  int total_count = 0;
	  int parEl = sizeVec / size;
	  int ostEl = sizeVec & size;
	
	if (rank == 0) {
		for (int proc = 1; proc < size; proc++) {
			MPI_Send(&myVec[ostEl] + proc * parEl, parEl, MPI_INT, proc, 0, MPI_COMM_WORLD);
		}
	}
	std::vector<int> local_vec(parEl);
	if (rank == 0) {
		local_vec.resize(parEl + ostEl);
		local_vec = std::vector<int>(myVec.begin(), myVec.begin() + parEl + ostEl);
	}
	else {
		MPI_Status status;
		MPI_Recv(&local_vec[0], parEl, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
	}
	
	if (rank == 0) {
		local_count += checkWrongOrder(local_vec, local_vec.size());
		std::vector<int> v(size);
		for (int n = 1; n < size; n++) {
			v[n] = myVec[ostEl - 1 + n * parEl];
			MPI_Send(&v[n], 1, MPI_INT, n, 0, MPI_COMM_WORLD);
		}
	}
	else {
		int v;
		MPI_Recv(&v, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		local_count += checkWrongOrder(local_vec, local_vec.size());
		if (v > local_vec[0]) {
			local_count += 1;
		}
	}
	MPI_Reduce(&local_count, &total_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	MPI_Finalize();
	return 0;
};


std::vector<int> getVector(int size) {
	std::mt19937 gen;
	gen.seed(static_cast<unsigned int>(time(0)));
	std::vector<int> vec(size);
	for (int i = 0; i < size; i++) {
		vec[i] = gen() % 100;
	}
	return vec;
};


int checkWrongOrder(std::vector<int> vector, int size) {
	int count = 0;
	for ( int c = 1; c < size ; c++) {
		if (vector[c] < vector[c - 1]) {
			count++;
		}
	}
	return count;
};




