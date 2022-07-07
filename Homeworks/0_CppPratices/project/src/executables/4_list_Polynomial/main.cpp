#include "PolynomialList.h"

#include <list>
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
	PolynomialList p1("../data/P3.txt");
	PolynomialList p2("../data/P4.txt");
	PolynomialList p3;
	p1.Print();
	p2.Print();

	p3 = p1 + p2;
	p3.Print();
	p3 = p1 - p2;
	p3.Print();

	p3 = p1 * p2;
	p3.Print();


	double cof[3] = {1,2,3};
	int deg[3] = { 0,1,2 };
	PolynomialList p4(cof, deg, 3);
	p4.Print();

	return 0;
}