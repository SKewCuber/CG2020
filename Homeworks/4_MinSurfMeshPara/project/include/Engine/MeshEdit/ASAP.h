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

	class ASAP : public HeapObj
	{
	public:
		ASAP(Ptr<TriMesh> triMesh);
		ASAP();
	public:
		static const Ptr<ASAP> New(Ptr<TriMesh> triMesh)
		{
			return Ubpa::New<ASAP>(triMesh);
		}
	public:
		// clear cache data
		void Clear();

		// init cache data (eg. half-edge structure) for Run()
		bool Init(Ptr<TriMesh> triMesh);

		// call it after Init()
		bool Run(bool show);

	private:
		void kernelASAP();


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
		class P :public TPolygon<V, E, P> {};
	private:
		friend class Paramaterize;

		Ptr<TriMesh> triMesh;
		const Ptr<HEMesh<V>> heMesh; // vertice order is same with triMesh

		std::vector<std::vector<double>> cot;
		std::vector<std::vector<double>> delta_x_x;
		std::vector<std::vector<double>> delta_x_y;

	public:
			double Cot(V* A, V* B, V* C);
			double distance(V* A, V* B);
	};
}





