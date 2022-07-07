#include "PolynomialMap.h"

#include <iostream>
#include <fstream>
#include <cassert>
#include <cmath>
const double EPSILON = 1.0e-10;
using namespace std;

PolynomialMap::PolynomialMap(const PolynomialMap& other) :m_Polynomial(other.m_Polynomial) {}

PolynomialMap::PolynomialMap(const string& file)
{
	ReadFromFile(file);
}

PolynomialMap::PolynomialMap(const double* cof, const int* deg, int n)
{
	for (int i = 0; i < n; i++)
	{
		coff(deg[i]) = cof[i];
	}
}

PolynomialMap::PolynomialMap(const vector<int>& deg, const vector<double>& cof)
{
	assert(deg.size() == cof.size());
	for (vector<double>::size_type i = 0; i < cof.size(); i++)
	{
		coff(deg[i]) = cof[i];
	}
}

double PolynomialMap::coff(int i) const
{
	const auto& iter = m_Polynomial.find(i);
	if (iter == m_Polynomial.end()) return 0.;
	return iter->second;
}

double& PolynomialMap::coff(int i)
{
	return m_Polynomial[i];
}

void PolynomialMap::compress()
{
	/*map<int, double> tmpPoly = m_Polynomial;
	m_Polynomial.clear();
	for (const auto& term : tmpPoly)
	{
		if (fabs(term.second) > EPSILON)
			coff(term.first) = term.second;
	}*/
	for (auto iter = m_Polynomial.begin(); iter != m_Polynomial.end(); )
	{
		if (fabs(iter->second) < EPSILON)
		{
			iter = m_Polynomial.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

PolynomialMap PolynomialMap::operator+(const PolynomialMap& right) const
{
	PolynomialMap Poly(*this);
	for (const auto& term : right.m_Polynomial)
	{
		Poly.coff(term.first) += term.second;
	}
	Poly.compress();
	return Poly;
}

PolynomialMap PolynomialMap::operator-(const PolynomialMap& right) const
{
	PolynomialMap Poly(*this);
	for (const auto& term : right.m_Polynomial)
	{
		Poly.coff(term.first) -= term.second;
	}
	Poly.compress();
	return Poly;
}

PolynomialMap PolynomialMap::operator*(const PolynomialMap& right) const
{
	PolynomialMap poly;
	for (const auto& term1 : m_Polynomial)
	{
		for (const auto& term2 : right.m_Polynomial)
		{
			poly.coff(term1.first + term2.first) += term1.second * term2.second;
		}
	}
	poly.compress();
	return poly;
}

PolynomialMap& PolynomialMap::operator=(const PolynomialMap& right)
{
	m_Polynomial = right.m_Polynomial;
	return *this;
}

void PolynomialMap::Print() const
{
	if (m_Polynomial.begin() == m_Polynomial.end())
	{
		printf("undefined\n");
		return;
	}
	printf("f(x)=");
	for (auto iter = m_Polynomial.begin(); iter != m_Polynomial.end(); iter++)
	{
		cout << iter->second;
		if (iter->first != 0)
		{
			cout << "x^" << iter->first;
		}
		if (std::next(iter) != m_Polynomial.end() && next(iter)->second > 0)
		{
			cout << "+";
		}
	}
	cout << endl;
}

bool PolynomialMap::ReadFromFile(const string& file)
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
			int deg;
			double cof;
			infile >> deg;
			infile >> cof;
			coff(deg) = cof;
		}
		infile.close();
		return true;
	}
	return false;
}
