#include <Engine/MeshEdit/Paramaterize.h>

#include <Engine/MeshEdit/MinSurf.h>

#include <Engine/Primitive/TriMesh.h>

using namespace Ubpa;

using namespace std;
using namespace Eigen;

Paramaterize::Paramaterize(Ptr<TriMesh> triMesh)
	:min(make_shared<MinSurf>())
{
	Init(triMesh);
}

void Paramaterize::Clear() {
	min->Clear();
	min = nullptr;
}

bool Paramaterize::Init(Ptr<TriMesh> triMesh) {
	
	return min->Init(triMesh);
}

bool Paramaterize::Run(bool show_para,bool cot) {
	if (min->heMesh->IsEmpty() || !min->triMesh)
	{
		printf("ERROR::Paramaterize::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	Ptr<TriMesh> t = min->triMesh;
	Ptr<TriMesh> trimesh= TriMesh::New(t->GetIndice(), t->GetPositions(),t->GetNormals(), t->GetTexcoords(), t->GetTangents());
	MinSurf tmp = MinSurf(trimesh);  //直接用MinSurf(trimesh)会修改原网格，(tmp和min的trimesh实际上指向同一对象)
	auto boundary = tmp.heMesh->Boundaries()[0];
	size_t nB = boundary.size();
	double k = double(nB) / 4;
	double step = 4 / double(nB);

	
	for (size_t i = 0; i < nB; i++)
	{
		if (i < k)
			boundary[i]->End()->pos = vecf3(i * step, 0, 0);
		else if (i < 2 * k)
			boundary[i]->End()->pos = vecf3(1, i * step - 1, 0);
		else if (i < 3 * k)
			boundary[i]->End()->pos = vecf3(3 - i * step, 1, 0);
		else
			boundary[i]->End()->pos = vecf3(0, 4 - i * step, 0);
	}
	tmp.Run(cot,min);
	vector<pointf2> texcoord;
	for (const auto &pos : tmp.triMesh->GetPositions())
	{
		texcoord.push_back(pointf2(pos[0], pos[1]));
	}
	min->triMesh->Update(texcoord);

	if (show_para)
	{
		min->triMesh->Update(tmp.triMesh->GetPositions());
	}
	return true;
}

