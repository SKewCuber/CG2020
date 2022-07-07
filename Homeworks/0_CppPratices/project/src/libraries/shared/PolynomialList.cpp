#include "PolynomialList.h"
#include "iostream"
#include "assert.h"
#include <list>
#include <vector>
#include<cmath>
#include<fstream>
const double EPSILON = 1.0e-10;
using namespace std;

PolynomialList::PolynomialList(const PolynomialList& other)
	:m_Polynomial(other.m_Polynomial)
{}


PolynomialList::PolynomialList(const string& file)
{
	ReadFromFile(file);
}

PolynomialList::PolynomialList(const double* cof, const int* deg, int n)
{

	for (size_t i = 0; i < n; i++)
	{
		AddOneTerm(Term(deg[i], cof[i]));
	}
}

PolynomialList::PolynomialList(const vector<int>& deg, const vector<double>& cof)
{
	assert(deg.size() == cof.size());
	for (vector<double>::size_type i = 0; i < cof.size(); i++)
	{
		AddOneTerm(Term(deg[i], cof[i]));
	}
}

double PolynomialList::coff(int i) const
{
	for (const auto& term : m_Polynomial)  //基于范围的for语句
	{
		if (term.deg == i)
		{
			return term.cof;
		}
		if (term.deg > i)
		{
			break;
		}
	}
}

double& PolynomialList::coff(int i)    //这个函数是可以作为左值的
{
	//for (auto& term : m_Polynomial)  //基于范围的for语句
	//{
	//	if (term.deg == i)
	//	{
	//		return term.cof;
	//	}
	//	if (term.deg > i)   //空引用
	//	{
	//		break;
	//	}
	//}
	return AddOneTerm(Term(i, 0)).cof;  
}

void PolynomialList::compress()
{
	/*for (auto iter = m_Polynomial.begin(); iter != m_Polynomial.end(); iter++)
	{
		if (fabs(iter->cof) < EPSILON)
		{
			m_Polynomial.erase(iter);
		}
	}*/     //迭代器指向的东西erase掉之后不能++，但为什么只在debug下有问题，release下没问题
	for (auto iter = m_Polynomial.begin(); iter != m_Polynomial.end(); )
	{
		if (fabs(iter->cof) < EPSILON)
		{
			iter = m_Polynomial.erase(iter);
		}
		else iter++;
	}
}

PolynomialList PolynomialList::operator+(const PolynomialList& right) const
{
	PolynomialList Poly(*this);
	for (const auto& term : right.m_Polynomial)
	{
		Poly.AddOneTerm(term);
	}
	Poly.compress();
	return Poly;
}

PolynomialList PolynomialList::operator-(const PolynomialList& right) const
{
	PolynomialList Poly(*this);
	for (const auto& term : right.m_Polynomial)
	{
		Poly.AddOneTerm(Term(term.deg, -term.cof));
	}
	Poly.compress();
	return Poly;
}

PolynomialList PolynomialList::operator*(const PolynomialList& right) const
{
	PolynomialList poly;
	for (const auto& term1 : m_Polynomial)
	{
		for (const auto& term2 : right.m_Polynomial)
		{
			poly.AddOneTerm(Term(term1.deg + term2.deg, term1.cof * term2.cof));
		}
	}
	poly.compress();
	return poly;
}

PolynomialList& PolynomialList::operator=(const PolynomialList& right)
{
	m_Polynomial = right.m_Polynomial;
	return *this;
}

void PolynomialList::Print() 
{
	this->compress();
	if (m_Polynomial.begin() == m_Polynomial.end())
	{
		printf("undefined\n");
		return;
	}
	printf("f(x)=");
	for (auto iter = m_Polynomial.begin(); iter != m_Polynomial.end(); iter++)
	{
		cout << iter->cof;
		if (iter->deg != 0)
		{
			cout << "x^" << iter->deg;
		}
		if (std::next(iter) != m_Polynomial.end() && next(iter)->cof>0)
		{
			cout << "+";
		}
	}
	cout << endl;
}

bool PolynomialList::ReadFromFile(const string& file)
{
	m_Polynomial.clear();
	ifstream infile;
	infile.open(file.c_str());
	if (infile.is_open())
	{
		char ch;
		int n;
		infile >> ch;
		infile >> n;
		for (int i = 0; i < n; i++)
		{
			Term term;
			infile >> term.deg;
			infile >> term.cof;
			AddOneTerm(term);
		}
		infile.close();
		return true;
	}
	return false;
}

PolynomialList::Term& PolynomialList::AddOneTerm(const Term& term)
{
	auto iter = m_Polynomial.begin();
	for (; iter != m_Polynomial.end(); iter++)
	{
		if (iter->deg == term.deg)
		{
			iter->cof += term.cof;
			return *iter;
		}
		if (iter->deg > term.deg)
		{
			break;
		}
		
	}
	return *m_Polynomial.insert(iter, term);
}
