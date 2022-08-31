#pragma once

#include <Basic/HeapObj.h>
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>
#include <Eigen/Sparse>


typedef Eigen::Triplet<double> T;
typedef Eigen::SparseLU<Eigen::SparseMatrix<double>>  Solver;
//typedef Eigen::SimplicialLLT<Eigen::SparseMatrix<double>>  Solver;

namespace Ubpa {
	class TriMesh;
	class Paramaterize;

	class MinSurf : public HeapObj {
	public:
		MinSurf(Ptr<TriMesh> triMesh);
		MinSurf();
	public:
		static const Ptr<MinSurf> New(Ptr<TriMesh> triMesh) {
			return Ubpa::New<MinSurf>(triMesh);
		}
	public:
		// clear cache data
		void Clear();

		// init cache data (eg. half-edge structure) for Run()
		bool Init(Ptr<TriMesh> triMesh);

		// call it after Init()
		bool Run(bool cot,Ptr<MinSurf> min);

		
	private:
		// kernel part of the algorithm, uniform weight
		void Minimize_Uniform();

		// kernel part of the algorithm, cot weight
		//cotȨ�ص�ϵ��������ԭ����ĵ������йأ���������ʱ�Ȱ�����ı߽�ӳ����ƽ���ϣ��ı��˵�����
		//��������ﴫ�����min���߽������δ�ı������
		void Minimize_Cot(Ptr<MinSurf> min); 


	private:
		class V;   //Vertex
		class E;   //Edge
		class P;   //Polygon
		class V : public TVertex<V, E, P> {
		public:
			vecf3 pos;
		};
		class E : public TEdge<V, E, P> { };
		class P :public TPolygon<V, E, P> { };
	private:
		friend class Paramaterize;

		Ptr<TriMesh> triMesh;
		const Ptr<HEMesh<V>> heMesh; // vertice order is same with triMesh


		//Eigen::SparseMatrix<double> A;    //Coefficient matrix
		//Solver solver;
	public:
		//�������㣬�����ACB��cot
		double Cot(V* A, V* B, V* C);

		double distance(V* A, V* B);
	};
}
