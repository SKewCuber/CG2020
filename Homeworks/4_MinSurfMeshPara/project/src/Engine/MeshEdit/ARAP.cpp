#include <Engine/MeshEdit/ASAP.h>

#include <Engine/MeshEdit/ARAP.h>

#include <Engine/Primitive/TriMesh.h>

#include <Eigen/Sparse>
#include <Eigen/Dense>

#include <Engine/MeshEdit/Paramaterize.h>

using namespace Ubpa;
using namespace std;
using namespace Eigen;

ARAP::ARAP(Ptr<TriMesh> triMesh)
	: heMesh(make_shared<HEMesh<V>>())
{
	Init(triMesh);
}

ARAP::ARAP()
	: heMesh(make_shared<HEMesh<V>>())
{}

void ARAP::Clear()
{
	heMesh->Clear();
	triMesh = nullptr;
}

bool ARAP::Init(Ptr<TriMesh> triMesh)
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

	auto asap = ASAP::New(triMesh);
	asap->Run(false);

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
	x_plane.resize(nT);
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

	Global_pre();


	this->triMesh = triMesh;
	return true;
}

bool ARAP::Run(bool show, int times)
{
	if (heMesh->IsEmpty() || !triMesh)
	{
		printf("ERROR::MinSurf::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}
	for (int i = 0; i < times; i++)
	{
		Local();
		Global();
	}

	if (!show)
	{
		// Set texcoords
		vector<pointf2> texcoords;
		for (auto v : heMesh->Vertices())
			texcoords.push_back(v->coord.cast_to<pointf2>());
		triMesh->Update(texcoords);

		cout << "ARAP done" << endl;
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

void ARAP::Local()
{
	size_t nT = heMesh->NumPolygons();
	for (size_t t = 0; t < nT; t++)
	{
		auto triangle = heMesh->Polygons()[t];
		Matrix2d S;
		S.setZero();
		for (int i = 0; i < 3; i++)
		{
			int next = (i + 1) % 3;
			V* v = triangle->BoundaryVertice()[i];
			V* v1 = triangle->BoundaryVertice()[next];
			Matrix2d tmp;
			tmp << delta_x_x[t][i] * (v->coord - v1->coord)[0], delta_x_y[t][i] * (v->coord - v1->coord)[0],
				delta_x_x[t][i] * (v->coord - v1->coord)[1], delta_x_y[t][i] * (v->coord - v1->coord)[1];
			S += cot[t][i] * tmp;
		}
		JacobiSVD<Matrix2d> svd(S, ComputeThinU | ComputeThinV);
		if (S.determinant() > 0)
			triangle->L = svd.matrixU() * svd.matrixV().transpose();
		else
		{
			Matrix2d D;
			D << 1, 0, 0, -1;
			triangle->L = svd.matrixU() * D * svd.matrixV().transpose();
		}
	}
}


void ARAP::Global_pre()
{
	vector<T> tripletlist;
	int nT = int(heMesh->NumPolygons());
	int nV = int(heMesh->NumVertices());

	auto bound = heMesh->Boundaries()[0];
	//把index1，index2固定到(0,0),(1,1)，硬约束
	fix_index1 = static_cast<int>(heMesh->Index(bound[0]->Origin()));
	size_t a = bound.size() / 2;
	fix_index2 = static_cast<int>(heMesh->Index(bound[a]->End()));

	for (int t = 0; t < nT; t++)
	{
		auto triangle = heMesh->Polygons()[t];
		for (int i = 0; i < 3; i++)  //triangle的第i条半边
		{
			auto he = triangle->BoundaryHEs()[i];
			int origin, end;  //半边的起点、终点在三角形中的序号
			for (origin = 0; origin < 3 && he->Origin() != triangle->BoundaryVertice()[origin]; origin++);
			for (end = 0; end < 3 && he->End() != triangle->BoundaryVertice()[end]; end++);
			int last = 3 - origin - end;  //三角形中剩下的一个点
			//半边的起点、终点在整个网格中的序号
			int index1 = static_cast<int>(heMesh->Index(triangle->BoundaryVertice()[origin]));
			int index2 = static_cast<int>(heMesh->Index(triangle->BoundaryVertice()[end]));
			if (index1 != fix_index1 /*&& index1 != fix_index2*/)
			{
				tripletlist.push_back(T(index1, index1, cot[t][last]));
				if (index2 != fix_index1 /*&& index2 != fix_index2*/)
				{
					tripletlist.push_back(T(index1, index2, -cot[t][last]));
					tripletlist.push_back(T(index2, index1, -cot[t][last]));
				}
			}
			if (index2 != fix_index1 /*&& index2 != fix_index2*/)
				tripletlist.push_back(T(index2, index2, cot[t][last]));
		}
	}
	tripletlist.push_back(T(fix_index1, fix_index1, 1));
	//tripletlist.push_back(T(fix_index2, fix_index2, 1));
	
	SparseMatrix<double> A(nV,nV);
	A.setFromTriplets(tripletlist.begin(), tripletlist.end());
	A.makeCompressed();
	global_solver.compute(A);
}


void ARAP::Global()
{
	int nT = int(heMesh->NumPolygons());
	int nV = int(heMesh->NumVertices());
	VectorXd bx = VectorXd::Zero(nV);
	VectorXd by = VectorXd::Zero(nV);
	for (int t = 0; t < nT; t++)
	{
		auto triangle = heMesh->Polygons()[t];
		for (int i = 0; i < 3; i++)  
		{
			auto he = triangle->BoundaryHEs()[i];
			int origin, end;
			for (origin = 0; origin < 3 && he->Origin() != triangle->BoundaryVertice()[origin]; origin++);
			for (end = 0; end < 3 && he->End() != triangle->BoundaryVertice()[end]; end++);
			int last = 3 - origin - end;
			int index1 = static_cast<int>( heMesh->Index(triangle->BoundaryVertice()[origin]));
			int index2 = static_cast<int>( heMesh->Index(triangle->BoundaryVertice()[end]));
			bx(index1) += cot[t][last] * (triangle->L(0, 0) * (x_plane[t][origin][0] - x_plane[t][end][0])+
				triangle->L(0, 1) * (x_plane[t][origin][1] - x_plane[t][end][1]));
			bx(index2) += cot[t][last] * (triangle->L(0, 0) * (x_plane[t][end][0] - x_plane[t][origin][0]) +
				triangle->L(0, 1) * (x_plane[t][end][1] - x_plane[t][origin][1]));

			by(index1) += cot[t][last] * (triangle->L(1, 0) * (x_plane[t][origin][0] - x_plane[t][end][0]) +
				triangle->L(1, 1) * (x_plane[t][origin][1] - x_plane[t][end][1]));
			by(index2) += cot[t][last] * (triangle->L(1, 0) * (x_plane[t][end][0] - x_plane[t][origin][0]) +
				triangle->L(1, 1) * (x_plane[t][end][1] - x_plane[t][origin][1]));


			/*if (index1 == fix_index2)
			{
				bx(index2) += cot[t][last];
				by(index2) += cot[t][last];
			}
			if (index2 == fix_index2)
			{
				bx(index1) += cot[t][last];
				by(index1) += cot[t][last];
			}*/
		}
	}

	bx(fix_index1) = 0;
	by(fix_index1) = 0;
	/*bx(fix_index2) = 1;
	by(fix_index2) = 1;*/

	VectorXd ux = global_solver.solve(bx);
	VectorXd uy = global_solver.solve(by);

	for (int i = 0; i < nV; i++)
	{
		V* v = heMesh->Vertices()[i];
		v->coord = vecf2(ux(i), uy(i));
	}
}

double ARAP::Cot(V* A, V* B, V* C)
{
	double a = distance(B, C);
	double b = distance(A, C);
	double c = distance(A, B);
	double cos = (pow(a, 2) + pow(b, 2) - pow(c, 2)) / (2 * a * b);
	return cos / (sqrt(1 - pow(cos, 2)));
}

double ARAP::distance(V* A, V* B)
{
	return sqrt(pow(A->pos[0] - B->pos[0], 2) +
		pow(A->pos[1] - B->pos[1], 2) + pow(A->pos[2] - B->pos[2], 2));
}