#include <Engine/MeshEdit/Simulate.h>
#include <Engine/Primitive/TriMesh.h>

#include <Eigen/Sparse>
#include <cmath>
#include<iostream>
using namespace Ubpa;

using namespace std;
using namespace Eigen;


void Simulate::Clear()
{
	this->positions.clear();
	this->velocity.clear();
}

bool Simulate::Init()
{
	//Clear();
	this->velocity.resize(positions.size());
	for (int i = 0; i < positions.size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			this->velocity[i][j] = 0;
		}
	}

	nV = positions.size();
	nE = edgelist.size() / 2;
	//设弹簧初始无伸缩
	l.resize(nE);
	for (int i = 0; i < nE; i++)
	{
		auto index1 = edgelist[2 * i];
		auto index2 = edgelist[2 * i + 1];
		//Vector3f x1 = x_n.segment(3 * index1, 3);
		//Vector3f x2 = x_n.segment(3 * index2, 3);
		auto x1 = positions[index1];
		auto x2 = positions[index2];
		l[i] = (x1 - x2).norm();
	}

	//外力
	f_ext = Eigen::VectorXf::Zero(3 * nV);
	for (int i = 0; i < nV; i++)
	{
		f_ext[3 * i] = 9.8f*mass;
		f_ext[3 * i + 1] = 0.f;
		f_ext(3 * i + 2) = - 9.8f * mass;
	}

	//初始化is_fixed
	if (fixed_id.empty()) 
	{
		fixed_id.push_back(0);
		fixed_id.push_back(10);
	}
	is_fixed.resize(nV);
	std::fill(is_fixed.begin(), is_fixed.end(), false);
	for (int i = 0; i < fixed_id.size(); i++)
	{
		is_fixed[fixed_id[i]] = true;
	}

	K.resize(3 * nV - 3 * fixed_id.size(), 3 * nV);
	K.setZero();
	vector<Tf> tripletlist_K;
	for (int i = 0, j = 0; i < nV - fixed_id.size(); j++)
	{
		if (!is_fixed[j])
		{
			tripletlist_K.push_back(Tf(3 * i, 3 * j, 1));
			tripletlist_K.push_back(Tf(3 * i + 1, 3 * j + 1, 1));
			tripletlist_K.push_back(Tf(3 * i + 2, 3 * j + 2, 1));
			i++;
		}
	}

	K.setFromTriplets(tripletlist_K.begin(), tripletlist_K.end());
	K.makeCompressed();
	Kt = K.transpose();
	//cout << K << endl;

	g.resize(3 * nV);
	gradient_g.resize(3 * nV, 3 * nV);
	y.resize(3 * nV);
	x_n.resize(3 * nV);
	x_k.resize(3 * nV);
	v_n.resize(3 * nV);

	return true;
}

bool Simulate::Run()
{
	SimulateOnce();

	// half-edge structure -> triangle mesh
	return true;
}

void Ubpa::Simulate::SetLeftFix()
{
	//固定网格x坐标最小点
	fixed_id.clear();
	double x = 100000;
	for (int i = 0; i < positions.size(); i++)
	{
		if (positions[i][0] < x)
		{
			x = positions[i][0];
		}
	}

	for (int i = 0; i < positions.size(); i++)
	{
		if (abs(positions[i][0] - x) < 1e-5)
		{
			fixed_id.push_back(i);
		}
	}


	Init();
}

void Simulate::SimulateOnce()
{
	for (int i = 0; i < nV; i++)
	{
		x_n(3 * i) = positions[i][0];
		x_n(3 * i + 1) = positions[i][1];
		x_n(3 * i + 2) = positions[i][2];
		v_n(3 * i) = velocity[i][0];
		v_n(3 * i + 1) = velocity[i][1];
		v_n(3 * i + 2) = velocity[i][2];
	}
	y = x_n + h * v_n + h * h * f_ext / mass;
	x_k = y;
	for (int i = 0; i < fixed_id.size(); i++)
	{
		x_k.segment(3 * fixed_id[i], 3) = x_n.segment(3 * fixed_id[i], 3);
	}
	x_f = K * x_k;
	b = x_k - Kt * x_f;
	for (int k = 0; k < max; k++)
	{
		vector<Tf> tripletlist;
		f_int = Eigen::VectorXf::Zero(3 * nV);
		for (int i = 0; i < 3 * nV; i++)
		{
			tripletlist.push_back(Tf(i, i, mass));
		}
		for (int i = 0; i < nE; i++)
		{
			auto index1 = edgelist[2 * i];
			auto index2 = edgelist[2 * i + 1];
			Vector3f x1 = x_k.segment(3 * index1, 3);
			Vector3f x2 = x_k.segment(3 * index2, 3);
			float r = (x1 - x2).norm();
			f_int.segment(3 * index1, 3) += stiff * (r - l[i]) * (x2 - x1) / r;
			f_int.segment(3 * index2, 3) += stiff * (r - l[i]) * (x1 - x2) / r;
			Matrix3f temp;
			temp = stiff * (l[i] - r) / r * Matrix3f::Identity() -
				stiff * l[i] * (x1 - x2) * ((x1 - x2).transpose()) / (r * r * r);
			temp = h * h * temp;
			for (int u = 0; u < 3; u++)
			{
				for (int v = 0; v < 3; v++)
				{
					tripletlist.push_back(Tf(3 * index1 + u, 3 * index1 + v, -temp(u, v)));
					tripletlist.push_back(Tf(3 * index1 + u, 3 * index2 + v, temp(u, v)));
					tripletlist.push_back(Tf(3 * index2 + u, 3 * index1 + v, temp(u, v)));
					tripletlist.push_back(Tf(3 * index2 + u, 3 * index2 + v, -temp(u, v)));
				}
			}
		}
		//cout << K*f_int << endl << endl;
		g = mass * (x_k - y) - h * h * f_int;
		//cout<<g<<endl<<endl;
		if ((K * g).cwiseAbs().maxCoeff() < epsilon)
			break;
		gradient_g.setFromTriplets(tripletlist.begin(), tripletlist.end());
		gradient_g.makeCompressed();
		solver.compute(K * gradient_g * Kt);
		//VectorXf delta_x_k = solver.solve(b);
		VectorXf delta_x_k = solver.solve(-K * g);
		//VectorXf delta_x_k=solver.solve(-g);

		x_f += delta_x_k;
		x_k = Kt * x_f + b;
		


		if (delta_x_k.cwiseAbs().maxCoeff() < delta)
			break;
	}

	for (int i = 0; i < nV; i++)
	{
		/*if (!is_fixed[i])
		{*/
			for (int j = 0; j < 3; j++)
			{
				velocity[i][j] = (x_k(3 * i + j) - positions[i][j]) / h;
				positions[i][j] = x_k(3 * i + j);
			}
		//}
	}
	/*for (int idx : fixed_id)
	{
		velocity[idx] = pointf3(0.f, 0.f, 0.f);
	}*/
}
