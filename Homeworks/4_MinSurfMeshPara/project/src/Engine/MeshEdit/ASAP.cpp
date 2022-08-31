#include <Engine/MeshEdit/MinSurf.h>
#include <Engine/MeshEdit/ASAP.h>

#include <Engine/Primitive/TriMesh.h>

#include <Eigen/Sparse>

#include<cmath>

using namespace Ubpa;

using namespace std;
using namespace Eigen;

ASAP::ASAP(Ptr<TriMesh> triMesh)
	: heMesh(make_shared<HEMesh<V>>())
{
	Init(triMesh);
}

ASAP::ASAP()
	: heMesh(make_shared<HEMesh<V>>())
{}

void ASAP::Clear()
{
	heMesh->Clear();
	triMesh = nullptr;
}

bool ASAP::Init(Ptr<TriMesh> triMesh)
{
	Clear();

	if (triMesh == nullptr)
		return true;

	if (triMesh->GetType() == TriMesh::INVALID)
	{
		printf("ERROR::ASAP::Init:\n"
			"\t""trimesh is invalid\n");
		return false;
	}

	//init half-edge structure
	size_t nV = triMesh->GetPositions().size();
	vector<vector<size_t>> triangles;
	triangles.reserve(triMesh->GetTriangles().size());
	for (auto triangle : triMesh->GetTriangles())
		triangles.push_back({ triangle->idx[0], triangle->idx[1], triangle->idx[2] });
	heMesh->Reserve(nV);
	heMesh->Init(triangles);

	if (!heMesh->IsTriMesh() || !heMesh->HaveBoundary())
	{
		printf("ERROR::ASAP::Init:\n"
			"\t""trimesh is not a triangle mesh or hasn't a boundaries\n");
		heMesh->Clear();
		return false;
	}


	//triangle mesh's positions ->  half-edge structure's positions
	for (int i = 0; i < nV; i++)
	{
		auto v = heMesh->Vertices().at(i);
		v->coord = triMesh->GetTexcoords()[i].cast_to<vecf2>();
		v->pos = triMesh->GetPositions()[i].cast_to<vecf3>();
	}


	//init cot and delta_x
	size_t nT = heMesh->NumPolygons();
	vector<vector<vecf2>> x_plane(nT);
	cot.resize(nT);
	delta_x_x.resize(nT);
	delta_x_y.resize(nT);
	auto triangles_he = heMesh->Polygons();

	for (size_t t = 0; t < nT; t++)
	{
		auto x_t = triangles_he[t]->BoundaryVertice();
		x_plane[t].push_back(vecf2(0, 0));
		x_plane[t].push_back(vecf2(distance(x_t[1], x_t[0]), 0));
		float x = (x_t[2]->pos - x_t[0]->pos).dot(x_t[1]->pos - x_t[0]->pos) / (x_t[1]->pos - x_t[0]->pos).norm();
		x_plane[t].push_back(vecf2(x, sqrt(pow(distance(x_t[2], x_t[0]), 2) - pow(x, 2))));
		//cout << -5 << endl;
		for (int i = 0; i < 3; i++)
		{
			/*int next = (i + 1) % 3;
			int nnext = (i + 2) % 3;*/
			int next = (i + 1 == 3) ? 0 : (i + 1);
			int nnext = (i == 0) ? 2 : (i - 1);
			delta_x_x[t].push_back(x_plane[t][i][0] - x_plane[t][next][0]);
			//cout << -6 << endl;
			delta_x_y[t].push_back(x_plane[t][i][1] - x_plane[t][next][1]);
			cot[t].push_back(Cot(x_t[next], x_t[nnext], x_t[i]));
		}
	}


	this->triMesh = triMesh;
	return true;
}

bool ASAP::Run(bool show)
{
	if (heMesh->IsEmpty() || !triMesh)
	{
		printf("ERROR::ASAP::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	//cout << -1 << endl;

	kernelASAP();

	if (!show)
	{
		// Set texcoords
		vector<pointf2> texcoords;
		for (auto v : heMesh->Vertices())
			texcoords.push_back(v->coord.cast_to<pointf2>());
		triMesh->Update(texcoords);

		cout << "ASAP done" << endl;
		return true;
	}

	//half-edge structure -> triangle mesh
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();
	vector<pointf3> positions;
	vector<unsigned> indice;
	positions.reserve(nV);
	indice.reserve(3 * nF);

	for (auto v : heMesh->Vertices())
		positions.push_back(v->coord.cast_to<pointf3>());
	for (auto f : heMesh->Polygons())
	{ // f is triangle
		for (auto v : f->BoundaryVertice()) // vertices of the triangle
			indice.push_back(static_cast<unsigned>(heMesh->Index(v)));
	}

	triMesh->Init(indice, positions);
	cout << "show ASAP done" << endl;
	return true;

}


void ASAP::kernelASAP()
{
	//cout << 0 << endl;
	int nT =int( heMesh->NumPolygons());
	int nV =int( heMesh->NumVertices());

	//cout << 0 << endl;
	SparseMatrix<double> A(2 * nV + 2 * nT , 2 * nV + 2 * nT);
	Solver solver;
	vector<T> tripletlist;
	VectorXd b = VectorXd::Zero(2 * nV + 2 * nT );
	
	auto bound = heMesh->Boundaries()[0];
	//把index1，index2固定到(0,0),(1,1)，硬约束
	int index1 = static_cast<int>(heMesh->Index(bound[0]->Origin()));
	size_t a = bound.size() / 2;
	int index2 = static_cast<int>(heMesh->Index(bound[a]->End()));

	auto triangles = heMesh->Polygons();

	//cout << 1 << endl;
	for (int t = 0; t < nT; t++)
	{
		double C1 = 0;
		for (int i = 0; i < 3; i++)
		{
			int nnext = (i + 2) % 3;
			C1 += cot[t][nnext] * (pow(delta_x_x[t][i], 2) + pow(delta_x_y[t][i], 2));
		}
		tripletlist.push_back(T(2 * nV + 2 * t, 2 * nV + 2 * t, C1));
		tripletlist.push_back(T(2 * nV + 2 * t + 1, 2 * nV + 2 * t + 1, C1));
		//cout << 2 << endl;
		for (int i = 0; i < 3; i++)
		{
			int next = (i + 1) % 3;
			int nnext = (i + 2) % 3;
			int k = int(heMesh->Index(triangles[t]->BoundaryVertice()[i]));
			if (k != index1 && k != index2)
			{
				tripletlist.push_back(T(2 * nV + 2 * t, 2 * k,
					- cot[t][nnext] * delta_x_x[t][i] + cot[t][next] * delta_x_x[t][nnext]));
				tripletlist.push_back(T(2 * nV + 2 * t, 2 * k + 1,
					- cot[t][nnext] * delta_x_y[t][i] + cot[t][next] * delta_x_y[t][nnext]));
				tripletlist.push_back(T(2 * nV + 2 * t + 1, 2 * k,
					- cot[t][nnext] * delta_x_y[t][i] + cot[t][next] * delta_x_y[t][nnext]));
				tripletlist.push_back(T(2 * nV + 2 * t + 1, 2 * k + 1,
					cot[t][nnext] * delta_x_x[t][i] - cot[t][next] * delta_x_x[t][nnext]));

				tripletlist.push_back(T(2 * k, 2 * nV + 2 * t,
					- cot[t][nnext] * delta_x_x[t][i] + cot[t][next] * delta_x_x[t][nnext]));
				tripletlist.push_back(T(2 * k + 1, 2 * nV + 2 * t,
					- cot[t][nnext] * delta_x_y[t][i] + cot[t][next] * delta_x_y[t][nnext]));
				tripletlist.push_back(T(2 * k, 2 * nV + 2 * t + 1,
					- cot[t][nnext] * delta_x_y[t][i] + cot[t][next] * delta_x_y[t][nnext]));
				tripletlist.push_back(T(2 * k + 1, 2 * nV + 2 * t + 1,
					cot[t][nnext] * delta_x_x[t][i] - cot[t][next] * delta_x_x[t][nnext]));


				//cout << 3 << endl;

				int k_next =int( heMesh->Index(triangles[t]->BoundaryVertice()[next]));
				int k_nnext =int( heMesh->Index(triangles[t]->BoundaryVertice()[nnext]));
				tripletlist.push_back(T(2 * k, 2 * k, cot[t][nnext] + cot[t][next]));
				tripletlist.push_back(T(2 * k + 1, 2 * k + 1, cot[t][nnext] + cot[t][next]));
				if (k_next != index1 && k_next != index2)
				{
					tripletlist.push_back(T(2 * k, 2 * k_next, - cot[t][nnext]));
					tripletlist.push_back(T(2 * k + 1, 2 * k_next + 1, - cot[t][nnext]));
				}
				else if (k_next == index2)
				{
					b(2 * k) -= (- cot[t][nnext]);
					b(2 * k + 1) -= - cot[t][nnext];
				}

				if (k_nnext != index1 && k_nnext != index2)
				{
					tripletlist.push_back(T(2 * k, 2 * k_nnext, - cot[t][next]));
					tripletlist.push_back(T(2 * k + 1, 2 * k_nnext + 1, - cot[t][next]));
				}
				else if (k_nnext == index2)
				{
					b(2 * k) -= (- cot[t][next]);
					b(2 * k + 1) -= - cot[t][next];
				}
				

				//cout << 4 << endl;
			}
			
			else if (k == index2)
			{
				b(2 * nV + 2 * t)-= ( - cot[t][nnext] * delta_x_x[t][i] + cot[t][next] * delta_x_x[t][nnext]+
					- cot[t][nnext] * delta_x_y[t][i] + cot[t][next] * delta_x_y[t][nnext]);
				b(2 * nV + 2 * t+1) -= (- cot[t][nnext] * delta_x_y[t][i] + cot[t][next] * delta_x_y[t][nnext] +
					cot[t][nnext] * delta_x_x[t][i] - cot[t][next] * delta_x_x[t][nnext]);

				b(2 * k) = 1;
				b(2 * k + 1) = 1;
			}			
		}
	}

	tripletlist.push_back(T(2 * index1, 2 * index1, 1));
	tripletlist.push_back(T(2 * index1+1, 2 * index1+1, 1));
	tripletlist.push_back(T(2 * index2, 2 * index2, 1));
	tripletlist.push_back(T(2 * index2+1, 2 * index2+1, 1));

	A.setFromTriplets(tripletlist.begin(), tripletlist.end());
	A.makeCompressed();
	solver.compute(A);	
	VectorXd u = solver.solve(b);

	for (int i = 0; i < nV; i++)
	{
		V* v = heMesh->Vertices()[i];
		v->coord = vecf2(u(2 * i), u(2 * i + 1));
		//v->coord = vecf2((2 /double( i)), (2 /double( i)));
	}

}


double ASAP::Cot(V* A, V* B, V* C)
{
	double a = distance(B, C);
	double b = distance(A, C);
	double c = distance(A, B);
	double cos = (pow(a, 2) + pow(b, 2) - pow(c, 2)) / (2 * a * b);
	return cos / (sqrt(1 - pow(cos, 2)));
}

double ASAP::distance(V* A, V* B)
{
	return sqrt(pow(A->pos[0] - B->pos[0], 2) +
		pow(A->pos[1] - B->pos[1], 2) + pow(A->pos[2] - B->pos[2], 2));
}








