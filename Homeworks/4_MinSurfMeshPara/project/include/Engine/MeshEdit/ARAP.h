#pragma once

#include <Basic/HeapObj.h>
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>
#include <Eigen/Sparse>
//#include <Eigen/OrderingMethods>

typedef Eigen::Triplet<double> T;
//typedef Eigen::SparseQR<Eigen::SparseMatrix<double>,COLAMDOrdering<int>>  Solver;
typedef Eigen::SparseLU<Eigen::SparseMatrix<double>>  Solver;

namespace Ubpa
{
	class TriMesh;
	class Paramaterize;

	class ARAP : public HeapObj
	{
	public:
		ARAP(Ptr<TriMesh> triMesh);
		ARAP();
	public:
		static const Ptr<ARAP> New(Ptr<TriMesh> triMesh)
		{
			return Ubpa::New<ARAP>(triMesh);
		}
	public:
		// clear cache data
		void Clear();

		// init cache data (eg. half-edge structure) for Run()
		bool Init(Ptr<TriMesh> triMesh);

		// call it after Init()
		bool Run(bool show,int times);

	private:
		void Local();
		void Global();
		void Global_pre();

	private:
		class V;   //Vertex
		class E;   //Edge
		class P;   //Polygon
		class V : public TVertex<V, E, P>
		{
		public:
			vecf2 coord;
			vecf3 pos;
		};
		class E : public TEdge<V, E, P> {};
		class P :public TPolygon<V, E, P>
		{
		public:
			Eigen::Matrix2d L;
		};
	private:
		friend class Paramaterize;

		Ptr<TriMesh> triMesh;
		const Ptr<HEMesh<V>> heMesh; // vertice order is same with triMesh

		std::vector<std::vector<double>> cot;
		std::vector<std::vector<double>> delta_x_x;
		std::vector<std::vector<double>> delta_x_y;
		std::vector<std::vector<vecf2>> x_plane;

		Solver global_solver;

		int fix_index1, fix_index2;

	public:
		double Cot(V* A, V* B, V* C);
		double distance(V* A, V* B);
	};
}




