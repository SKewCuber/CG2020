#include "Poisson.h"
#include <iostream>
#include <Eigen\Sparse>

using namespace std;
using namespace Eigen;

Poisson::Poisson()
{}

Poisson::~Poisson()
{}

void Poisson::Get_Poisson_solver(MatrixXi inpolygon_)
{
	int width = (int)inpolygon_.rows();
	int height = (int)inpolygon_.cols();
	int N = height * width;
	SparseMatrix<double> A(N, N);
	vector<T> tripletlist;


	// compute the coefficient matrix
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			int center = i * height + j;
			if (inpolygon_(i, j) != 1)
			{
				tripletlist.push_back(T(center, center, 1));
			}
			else if (inpolygon_(i, j) == 1)
			{
				tripletlist.push_back(T(center, center, 4));
				if (inpolygon_(i - 1, j) == 1)
					tripletlist.push_back(T(center, center - height, -1));
				if (inpolygon_(i + 1, j) == 1)
					tripletlist.push_back(T(center, center + height, -1));
				if (inpolygon_(i, j - 1) == 1)
					tripletlist.push_back(T(center, center - 1, -1));
				if (inpolygon_(i, j + 1) == 1)
					tripletlist.push_back(T(center, center + 1, -1));
			}
		}
	}
	A.setFromTriplets(tripletlist.begin(), tripletlist.end());
	A.makeCompressed();
	solver.compute(A);

	//另一种初始化方式
	//Solver* p_A = new Solver(A);  用的是构造函数SimplicialLLT的构造函数
}

VectorXd Poisson::Poisson_solve(MatrixXd Data_f, MatrixXd Data_g, MatrixXi inpolygon_,  bool mixed)
//Data_f的第i行第j列元素对于图像的(i,j)像素
{
	int width = (int)Data_f.rows();
	int height = (int)Data_f.cols();
	int N = height * width;
	VectorXd b = VectorXd::Zero(N);

	//compute the gradient
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			int center = i * height + j;
			if (inpolygon_(i, j) != 1)
			{
				b(center) = Data_f(i, j);
			}
			else
			{
				if (inpolygon_(i - 1, j) != 1)
					b(center) += Data_f(i - 1, j);
				if (inpolygon_(i + 1, j) != 1)
					b(center) += Data_f(i + 1, j);
				if (inpolygon_(i, j - 1) != 1)
					b(center) += Data_f(i, j - 1);
				if (inpolygon_(i, j + 1) != 1)
					b(center) += Data_f(i, j + 1);

				if (mixed)
				{
					b(center) += (abs(Data_g(i, j) - Data_g(i + 1, j)) < abs(Data_f(i, j) - Data_f(i + 1, j))) ?
						Data_f(i, j) - Data_f(i + 1, j) : Data_g(i, j) - Data_g(i + 1, j);
					b(center) += (abs(Data_g(i, j) - Data_g(i - 1, j)) < abs(Data_f(i, j) - Data_f(i - 1, j))) ?
						Data_f(i, j) - Data_f(i - 1, j) : Data_g(i, j) - Data_g(i - 1, j);
					b(center) += (abs(Data_g(i, j) - Data_g(i, j + 1)) < abs(Data_f(i, j) - Data_f(i, j + 1))) ?
						Data_f(i, j) - Data_f(i, j + 1) : Data_g(i, j) - Data_g(i, j + 1);
					b(center) += (abs(Data_g(i, j) - Data_g(i, j - 1)) < abs(Data_f(i, j) - Data_f(i, j - 1))) ?
						Data_f(i, j) - Data_f(i, j - 1) : Data_g(i, j) - Data_g(i, j - 1);
				}
				else
				{
					b(center) += 4 * Data_g(i, j) - Data_g(i + 1, j)
						- Data_g(i - 1, j) - Data_g(i, j + 1) - Data_g(i, j - 1);
				}
			}
		}
	}
	VectorXd x = solver.solve(b);
	return x;
}

