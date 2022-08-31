#pragma once
#include <Eigen\Sparse>
using namespace Eigen;
typedef Eigen::Triplet<double> T;
typedef Eigen::SimplicialLLT<SparseMatrix<double>> Solver;


class Poisson
{
public:
	Poisson();
	~Poisson();
	void Get_Poisson_solver(MatrixXi inpolygon_);			
	VectorXd Poisson_solve(MatrixXd Data_f, MatrixXd Data_g, MatrixXi inpolygon_, bool mixed);		// use Poisson solver to slove the equation
	Solver solver;
};
