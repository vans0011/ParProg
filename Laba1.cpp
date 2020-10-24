
#include <iostream>
#include <vector>


class point {
public:
	int x;
	int y;

	point() {
		x = 0;
		y = 0;
	};
	point(int _x, int _y) {
		x = _x;
		y = _y;
	};
    
	void pr(){
		std::cout << x << " " << y << std::endl;
		
	};

	point& operator=(const point& right) {
		if (this == &right) {
			return *this;
		}
		x = right.x;
		y = right.y;
		return *this;
	}

	~point(void) {
		x = 0;
		y = 0;
	};
};

bool rotate(point a, point b, point c);

int main() {
	int n = 10;
//создание точек
	point *allPoint = new point[n];
	for (int i = 0; i < n; i++) {
		allPoint[i] = point(rand() % 10 +1, rand() % 10 +1) ;
	};

 // обход грехэма 
	//1 найти нижнюю левую точку, непосредственно входящую в МВО
	for (int i = 1; i < n; i++) {
		if ((allPoint[0].x > allPoint[i].x) || (allPoint[0].x == allPoint[i].x) && (allPoint[0].y > allPoint[i].y)) {
			std::swap(allPoint[0], allPoint[i]);
		}
	  }
   //2 отсортировать по полярному углу
	for (int i = 0, j; i < n; i++) {
		j = i;
		while (j>1 && rotate(allPoint[0], allPoint[j - 1], allPoint[j]))
		{
			std::swap(allPoint[j], allPoint[j - 1]);
			j--;
		}
	}
	for (int i = 0; i< n; i++) {
		allPoint[i].pr();
	};
	std::cout << "Polar sort" << std::endl;
	//3 определить левый поворот и построение мво
	std::vector<point> vipykl;
	
	for (int i = 0 , j = 0; i < n; i++) {
		while (vipykl.size() >= 2 && rotate(vipykl[j - 2], vipykl[j - 1], allPoint[i])) {
			vipykl.pop_back();
			j--;
		}
		vipykl.push_back(allPoint[i]);
		j++;
	};

	for (int i = 0; i < vipykl.size(); i++) {
		vipykl[i].pr();
	}
	
 	std::cout << "vipukl is done" << std::endl;
	 system("pause");
	return 0;

};


bool rotate(point a, point b, point c) {
	if ((b.x - a.x)*(c.y - b.y) - (b.y - a.y)*(c.x - b.x) < 0) return true;
	return false;
};