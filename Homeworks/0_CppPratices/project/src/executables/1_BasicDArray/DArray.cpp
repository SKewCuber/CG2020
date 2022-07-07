// implementation of class DArray
#include "DArray.h"
#include "iostream"
#include"cstdio"
using namespace std;
// default constructor
DArray::DArray()
{
	Init();
}

// set an array with default values
DArray::DArray(int nSize, double dValue)
{
	m_pData = new double[nSize];
	for (int i = 0; i < nSize; i++)
	{
		m_pData[i] = dValue;
	}
	m_nSize = nSize;
}

DArray::DArray(const DArray& arr)
{
	m_pData = new double[arr.m_nSize];
	m_nSize = arr.m_nSize;
	for (int i = 0; i < m_nSize; i++)
	{
		m_pData[i] = arr.m_pData[i];
	}
}

// deconstructor
DArray::~DArray()
{
	Free();
}

// display the elements of the array
void DArray::Print() const
{
	for (int i = 0; i < m_nSize; i++)
	{
		//std::cout << m_pData[i] << " ";
		printf("%7.2f", m_pData[i]);
	}
	//std::cout << std::endl;
	printf("\n");
}

// initilize the array
void DArray::Init()
{
	m_pData = nullptr;  //不建议用m_pData=NULL,  ~函数重载  ，~C++中（void*）不能转换为任意类型的指针  ~C++中NULL就是0，不是(void*)0
	//m_pData = NULL;
	m_nSize = 0;
}

// free the array
void DArray::Free()
{
	delete[] m_pData;
	m_pData = nullptr;
	m_nSize = 0;
}

// get the size of the array
int DArray::GetSize() const
{
	return m_nSize;
}

// set the size of the array
void DArray::SetSize(int nSize)
{
	double* pData = new double[nSize];
	if (m_nSize >= nSize)
	{
		for (int i = 0; i < nSize; i++)
		{
			pData[i] = m_pData[i];
		}
	}
	else
	{
		for (int i = 0; i < m_nSize; i++)
		{
			pData[i] = m_pData[i];
		}
		for (int i = m_nSize; i < nSize; i++)
		{
			pData[i] = 0;
		}
	}
	delete[] m_pData;
	m_pData = pData;
	m_nSize = nSize;
}

// get an element at an index
const double& DArray::GetAt(int nIndex) const
{
	//static double ERROR; // you should delete this line
	//return ERROR; // you should return a correct value
	if (nIndex < m_nSize && nIndex >= 0)
	{
		return m_pData[nIndex];
	}
	exit(1);
}

// set the value of an element 
void DArray::SetAt(int nIndex, double dValue)
{
	if (nIndex < m_nSize && nIndex >= 0)
	{
		m_pData[nIndex] = dValue;
	}
	else
	{
		exit(1);
	}
}

// overload operator '[]' ,可以连续赋值
const double& DArray::operator[](int nIndex) const 
{
	if (nIndex < m_nSize && nIndex >= 0)
	{
		return m_pData[nIndex];
	}
	exit(1);
}

// add a new element at the end of the array
void DArray::PushBack(double dValue)
{
	double* pData = new double[m_nSize + 1];
	for (int i = 0; i < m_nSize; i++)
	{
		pData[i] = m_pData[i];
	}
	pData[m_nSize] = dValue;
	delete[] m_pData;
	m_pData = pData;
	m_nSize++;
}

// delete an element at some index
void DArray::DeleteAt(int nIndex)
{
	if (nIndex < m_nSize && nIndex >= 0)
	{
		double* pData = new double[m_nSize - 1];
		for (int i = 0; i < nIndex; i++)
		{
			pData[i] = m_pData[i];
		}
		for (int i = nIndex; i < m_nSize - 1; i++)
		{
			pData[i] = m_pData[i + 1];
		}
		delete []m_pData;
		m_pData = pData;
		m_nSize--;
	}
	else
	{
		exit(1);
	}
}

// insert a new element at some index
void DArray::InsertAt(int nIndex, double dValue)
{
	if (nIndex <= m_nSize && nIndex >= 0)
	{
		double* pData = new double[m_nSize + 1];

		for (int i = 0; i < nIndex; i++)
		{
			pData[i] = m_pData[i];
		}
		pData[nIndex] = dValue;
		for (int i = nIndex + 1; i <= m_nSize; i++)
		{
			pData[i] = m_pData[i - 1];
		}
		delete[]m_pData;
		m_pData = pData;
		m_nSize++;
	}
	else
	{
		exit(1);
	}
}

// overload operator '='
DArray& DArray::operator = (const DArray& arr)
{
	delete[] m_pData;
	m_nSize = arr.m_nSize;
	m_pData = new double[m_nSize];
	for (int i = 0; i < m_nSize; i++)
	{
		m_pData[i] = arr.m_pData[i];
	}

	return *this;
}
