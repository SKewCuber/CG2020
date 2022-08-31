#pragma once

#include <Basic/HeapObj.h>
//#include <Engine/Primitive/MassSpring.h>
#include <UGM/UGM>
#include <Eigen/Dense>
#include <Eigen/Sparse>

typedef Eigen::Triplet<float> Tf;
//typedef Eigen::SparseLU<Eigen::SparseMatrix<float>>  Solverf;
typedef Eigen::SimplicialLLT<Eigen::SparseMatrix<float>>  Solverf;


namespace Ubpa {
	class Simulate : public HeapObj {
	public:
		Simulate(const std::vector<pointf3>& plist,
			const std::vector<unsigned>& elist) {
			edgelist = elist;
			this->positions.resize(plist.size());
			for (int i = 0; i < plist.size(); i++)
			{
				for (int j = 0; j < 3; j++)
				{
					this->positions[i][j] = plist[i][j];
				}
			}
		};
	public:
		static const Ptr<Simulate> New(const std::vector<pointf3>& plist,
			const std::vector<unsigned> &elist) {
			return Ubpa::New<Simulate>(plist, elist);
		}
	public:
		// clear cache data
		void Clear();

		// init cache data (eg. half-edge structure) for Run()
		bool Init();
		//bool Init();

		// call it after Init()
		bool Run();
		
		const std::vector<pointf3>& GetPositions() const { return positions; };

		const float GetStiff() { return stiff; };
		void SetStiff(float k) { stiff = k;  };
		const float GetTimeStep() { return h; };
		void SetTimeStep(float k) { h = k; };
		std::vector<unsigned>& GetFix() { return this->fixed_id; };
		void SetFix(const std::vector<unsigned>& f) { this->fixed_id = f; Init();};
		const std::vector<pointf3>& GetVelocity() { return velocity; };
		//void SetVelocity(const std::vector<pointf3>& v) { velocity = v; };

		void SetLeftFix();

		void SetMethodEuler() { this->method = 1; Init(); };
		void SetMethodAccel() { this->method = 2; Init(); };

	private:
		// kernel part of the algorithm
		void SimulateOnce();

	private:
		int method = 1;//1 Euler 2 Accelerate

		float h = 0.03f;  //����
		int max = 10;  //��ֹ����1������������
		float epsilon = 1e-4f , delta = 1e-4f;  //��ֹ����2��3������ֵ��x_k�ı���
		float stiff=1000.f;  //����ϵ��k
		float mass = 1.f;       //ÿ���������  
		std::vector<unsigned> fixed_id;  //fixed point id
		//std::vector<pointf3> f_ext;//����
		//std::vector<pointf3> f_int;//ÿ���������
		


		//mesh data
		std::vector<unsigned> edgelist;
		int nV;  //������
		int nE;  //���� 

		//simulation data
		std::vector<pointf3> positions;
		std::vector<pointf3> velocity;
		
		Eigen::VectorXf x_n;   //ͬdocument
		Eigen::VectorXf x_k;   //x^(k)  ~ţ�ٷ�
		Eigen::VectorXf x_f;   //���ɽڵ�
		Eigen::VectorXf b;   //document ~Լ��
		Eigen::VectorXf y;     //ͬdocument
		Eigen::VectorXf f_ext; //����    
		Eigen::VectorXf f_int; //����    
		Eigen::VectorXf v_n;
		std::vector<float> l;  //����ԭ��
		
		Eigen::VectorXf g;   //g(x)
		Eigen::SparseMatrix<float> gradient_g;
		Solverf solver;

		std::vector<bool> is_fixed;
		Eigen::SparseMatrix<float> K; //ͬdocument  ~λ��Լ��
		Eigen::SparseMatrix<float> Kt; //K.transpose()
	};
}
