#include <Engine/MeshEdit/MinSurf.h>

#include <Engine/Primitive/TriMesh.h>

#include <Eigen/Sparse>

#include<cmath>

using namespace Ubpa;

using namespace std;
using namespace Eigen;

MinSurf::MinSurf(Ptr<TriMesh> triMesh)
	: heMesh(make_shared<HEMesh<V>>())
{
	Init(triMesh);
}

MinSurf::MinSurf()
	: heMesh(make_shared<HEMesh<V>>())
{}

void MinSurf::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
}

bool MinSurf::Init(Ptr<TriMesh> triMesh) {
	Clear();

	if (triMesh == nullptr)
		return true;

	if (triMesh->GetType() == TriMesh::INVALID) {
		printf("ERROR::MinSurf::Init:\n"
			"\t""trimesh is invalid\n");
		return false;
	}

	// init half-edge structure
	size_t nV = triMesh->GetPositions().size();
	vector<vector<size_t>> triangles;
	triangles.reserve(triMesh->GetTriangles().size());
	for (auto triangle : triMesh->GetTriangles())
		triangles.push_back({ triangle->idx[0], triangle->idx[1], triangle->idx[2] });
	heMesh->Reserve(nV);
	heMesh->Init(triangles);

	if (!heMesh->IsTriMesh() || !heMesh->HaveBoundary()) {
		printf("ERROR::MinSurf::Init:\n"
			"\t""trimesh is not a triangle mesh or hasn't a boundaries\n");
		heMesh->Clear();
		return false;
	}

	// triangle mesh's positions ->  half-edge structure's positions
	for (int i = 0; i < nV; i++) {
		auto v = heMesh->Vertices().at(i);
		v->pos = triMesh->GetPositions()[i].cast_to<vecf3>();
	}

	this->triMesh = triMesh;
	return true;
}

bool MinSurf::Run(bool cot, Ptr<MinSurf> min)
{
	if (heMesh->IsEmpty() || !triMesh) 
	{
		printf("ERROR::MinSurf::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}
	if(cot)
		Minimize_Cot(min);
	else
		Minimize_Uniform();
	// half-edge structure -> triangle mesh
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();
	vector<pointf3> positions;
	vector<unsigned> indice;
	positions.reserve(nV);
	indice.reserve(3 * nF);
	for (auto v : heMesh->Vertices())
		positions.push_back(v->pos.cast_to<pointf3>());
	for (auto f : heMesh->Polygons()) 
	{ // f is triangle
		for (auto v : f->BoundaryVertice()) // vertices of the triangle
			indice.push_back(static_cast<unsigned>(heMesh->Index(v)));
	}

	triMesh->Init(indice, positions);

	return true;
}

void MinSurf::Minimize_Uniform() 
{
	size_t nV = heMesh->NumVertices();
	SparseMatrix<double> A(nV, nV);
	Solver solver;
	vector<T> tripletlist;
	VectorXd bx = VectorXd::Zero(nV);
	VectorXd by = VectorXd::Zero(nV);
	VectorXd bz = VectorXd::Zero(nV);

	for (size_t i = 0; i < nV; i++)
	{
		V* v = heMesh->Vertices()[i];
		const auto adjVs = v->AdjVertices();
		double d = double(adjVs.size());
		if (!v->IsBoundary())
		{
			tripletlist.push_back(T(i, i, d));
			for (size_t j = 0; j < adjVs.size(); j++)
			{
				if (!adjVs[j]->IsBoundary())
				{
					tripletlist.push_back(T(i, heMesh->Index(adjVs[j]), -1));
				}
				else
				{
					bx(i) += adjVs[j]->pos[0];
					by(i) += adjVs[j]->pos[1];
					bz(i) += adjVs[j]->pos[2];
				}
			}
		}
		else
		{
			tripletlist.push_back(T(i, i, 1));
			bx(i) = v->pos[0];
			by(i) = v->pos[1];
			bz(i) = v->pos[2];
		}
	}
	A.setFromTriplets(tripletlist.begin(), tripletlist.end());
	A.makeCompressed();
	solver.compute(A);

	VectorXd x = solver.solve(bx);
	VectorXd y = solver.solve(by);
	VectorXd z = solver.solve(bz);

	for (size_t i = 0; i < nV; i++)
	{
		V* v = heMesh->Vertices()[i];
		v->pos[0] = x(i);
		v->pos[1] = y(i);
		v->pos[2] = z(i);
	}
	
}


void MinSurf::Minimize_Cot(Ptr<MinSurf> min)
{
	size_t nV = heMesh->NumVertices();
	SparseMatrix<double> A(nV, nV);
	Solver solver;
	vector<T> tripletlist;
	VectorXd bx = VectorXd::Zero(nV);
	VectorXd by = VectorXd::Zero(nV);
	VectorXd bz = VectorXd::Zero(nV);

	for (size_t i = 0; i < nV; i++)
	{
		V* v = heMesh->Vertices()[i];
		V* min_v = min->heMesh->Vertices()[i];
		const auto adjVs = v->AdjVertices();
		const auto min_adjVs = min_v->AdjVertices();
		double d = double(adjVs.size());
		vector<double> w;
		for (size_t j = 0; j < adjVs.size(); j++)
		{
			int next=(j + 1) % adjVs.size();
			int front = (j == 0) ? (adjVs.size() - 1) : (j - 1);
			w.push_back( Cot(min_adjVs[j], min_v, min_adjVs[next]) + Cot(min_adjVs[j], min_v, min_adjVs[front]));
		}

		if (!v->IsBoundary())
		{
			
			for (size_t j = 0; j < adjVs.size(); j++)
			{
				tripletlist.push_back(T(i, i, w[j]));
				if (!adjVs[j]->IsBoundary())
				{
					tripletlist.push_back(T(i, heMesh->Index(adjVs[j]), -w[j]));
				}
				else
				{
					bx(i) +=w[j]* adjVs[j]->pos[0];
					by(i) +=w[j]* adjVs[j]->pos[1];
					bz(i) +=w[j]* adjVs[j]->pos[2];
				}
			}
		}
		else
		{
			tripletlist.push_back(T(i, i, 1));
			bx(i) = v->pos[0];
			by(i) = v->pos[1];
			bz(i) = v->pos[2];
		}
	}

	A.setFromTriplets(tripletlist.begin(), tripletlist.end());
	A.makeCompressed();
	solver.compute(A);

	VectorXd x = solver.solve(bx);
	VectorXd y = solver.solve(by);
	VectorXd z = solver.solve(bz);

	for (size_t i = 0; i < nV; i++)
	{
		V* v = heMesh->Vertices()[i];
		v->pos[0] = x(i);
		v->pos[1] = y(i);
		v->pos[2] = z(i);
	}
}


double MinSurf::Cot(V* A, V* B, V* C)
{
	double a = distance(B, C);
	double b = distance(A, C);
	double c = distance(A, B);
	double cos = (pow(a, 2) + pow(b, 2) - pow(c, 2)) / (2 * a * b);
	return cos / (sqrt(1 - pow(cos, 2)));
}

double MinSurf::distance(V* A, V* B)
{
	return sqrt(pow(A->pos[0] - B->pos[0], 2) +
		pow(A->pos[1] - B->pos[1], 2) + pow(A->pos[2] - B->pos[2], 2));
}