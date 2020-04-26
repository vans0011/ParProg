#include <mpi.h>
#include <vector>
#include <stdlib.h>
#include <random>
#include <ctime>
#include "iostream"
#include <math.h>
#include <cmath>
#include <stack>
#include <algorithm>
#include <cassert>
//22.Построение выпуклой оболочки – проход Грэхема.


static int offset = 0;
const double PI = 3.1415;

struct point {
	double x, y;

	point(double X, double Y) {
		x = X;
		y = Y;
	}

	point() {
		x = 0;
		y = 0;
	}
};

int LowestPoint(const std::vector<point>& points) {
	point first = points[0];
	int first_index = 0;
	int n = points.size();
	for (int i = 1; i < n; ++i)
		if (points[i].x < first.x ||
			(points[i].x == first.x && points[i].y < first.y)) {
			first = points[i];
			first_index = i;
		}
	return first_index;
}

double area_triangle(point a, point b, point c) {
	return a.x * b.y + b.x * c.y + c.x * a.y - a.y * b.x - b.y * c.x - c.y * a.x;
}

int ccw(point p0, point p1, point p2) {
	double tmp = area_triangle(p0, p1, p2);
	if (std::abs(tmp) < 1e-14)
		return 0;
	if (tmp > 0)
		return 1;
	return -1;
}

std::vector<point> Sort(const std::vector<point>& p, point first_point) {
	std::vector<point> vec(p);
	std::sort(vec.begin(), vec.end()
		, [&](const point& a, const point& b) {
		return ccw(first_point, a, b) > 0;
	});
	return vec;
}


std::vector<point> Merge(const std::vector<point>& src1, const std::vector<point>& src2
	, point first_point) {
	int first = src1.size();
	int second = src2.size();

	std::vector<point> dest(first + second);

	int i = 0, j = 0, k = 0;
	while (i < first && j < second) {
		if (ccw(first_point, src1[i], src2[j]) >= 0) {  
			dest[k] = src1[i];
			++i; ++k;
		}
		else {
			dest[k] = src2[j];
			++j; ++k;
		}
	}
	
	while (i < first) {
		dest[k] = src1[i];
		++i; ++k;
	}
	while (j < second) {
		dest[k] = src2[j];
		++j; ++k;
	}
	return dest;
}


std::vector<point> ParallelSort(const std::vector<point>& points, point first_point) {
	int size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Datatype MPI_Point;
	MPI_Type_contiguous(2, MPI_DOUBLE, &MPI_Point);
	MPI_Type_commit(&MPI_Point);

	int n;
	if (rank == 0) {
		n = points.size() / size;
	}
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

	std::vector<int>sendcounts(size), displs(size);
	for (int i = 0; i < size; i++) {
		sendcounts[i] = points.size() / size + (rank < (signed)(points.size() % size) ? 1 : 0);
		if (i != 0)
			displs[i] = displs[i - 1] + sendcounts[i - 1];
	}
	std::vector<point> dest(sendcounts[rank]);
	MPI_Scatterv(points.data(), sendcounts.data(), displs.data(),
		MPI_Point, dest.data(), sendcounts[rank],
		MPI_Point, 0, MPI_COMM_WORLD);

	dest = Sort(dest, first_point);

	if (rank != 0) {
		MPI_Send(dest.data(), sendcounts[rank], MPI_Point, 0, rank, MPI_COMM_WORLD);
	}
	if (rank == 0) {
		std::vector<point> tmp(n);
		for (int i = 1; i < size; ++i) {
			MPI_Status status;
			MPI_Recv(tmp.data(), sendcounts[i], MPI_Point, MPI_ANY_SOURCE,
				MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			dest = Merge(dest, tmp, first_point);
		}
		dest[0] = first_point;
	}

	MPI_Type_free(&MPI_Point);

	return dest;
}

int pow2(int st) {
	int res = 1;
	for (int i = 0; i < st; i++)
		res *= 2;
	return res;
}

double dist(point p1, point p2) {
	return std::sqrt(((p1.x - p2.x) * (p1.x - p2.x))
		+ ((p1.y - p2.y) * (p1.y - p2.y)));

}

std::vector<point> getRandomArray(size_t size, int max_X = 100.0
	, int max_Y = 100.0) {
	std::vector<point> vec(size);
	std::mt19937 gen;
	gen.seed((unsigned)time(0) + ++offset);

	int x, y;
	for (size_t i = 0; i < vec.size(); ++i) {
		x = gen() % max_X;
		y = gen() % max_Y;
		vec[i] = point(x, y);
	}
	return vec;
}

int main(int argc, char** argv) {
	int rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int size = atoi(argv[1]);

	std::vector<point> points;
	std::vector<int> indexes;

	size_t first_index;
	point first_point;
	if (rank == 0) {
		points = getRandomArray(size);

		first_index = LowestPoint(points);
		point first_point = points[first_index];
		point tmp = points[0];
		points[first_index] = tmp;
		points[0] = first_point;
	}

	points = ParallelSort(points, first_point); 

	if (rank == 0) {
		bool isSorted = true;
		if (points.size() < 3)
			isSorted = true;
		else {
			for (size_t i = 2; i < points.size(); ++i) {
				if (ccw(first_point, points[i], points[i - 1]) == 1) {
					isSorted = false;
					break;
				}
			}
		}
		if (isSorted)
			std::cout << "Array is sorted" << std::endl;
		else
			std::cout << "Array is not sorted!!!" << std::endl;

		indexes.resize(points.size());

		int* ip_data = indexes.data();
		ip_data[0] = 0;

		ip_data[1] = 1;

		size_t top = 1;
		size_t i = 2;
		size_t n = points.size();

		while (i < n) {
			int res = ccw(points[indexes[top - 1]], points[indexes[top]], points[i]);

			if (res == 0) { 
				++top;
				indexes[top] = i;
				++i;
			}
			if (res == 1) {  
				++top;
				indexes[top] = i;
				++i;
			}
			if (res == -1) { 
				if (top > 1)
					--top;
				else {
					indexes[top] = i;
					++i;
				}

			}
		}

		indexes.resize(top + 1);

		bool isConvex = true;
		if (indexes.size() < 3)
			isConvex = true;
		else {
			for (size_t i = 2; i < indexes.size(); ++i)
				if (ccw(points[indexes[i - 2]], points[indexes[i - 1]], points[indexes[i]]) < 0) {
					isConvex = false;
					break;
				}
		}

		if (isConvex)
			std::cout << "Convex is hull" << std::endl;
		else
			std::cout << "Convex is not hull!!!" << std::endl;
	}

	MPI_Finalize();
	return 0;
}