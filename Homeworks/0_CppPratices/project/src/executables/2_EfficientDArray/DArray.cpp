// implementation of class DArray
#include "DArray.h"

//#include"cstdio"
//#include"cstring"
#include"iostream"
#include "assert.h"

using namespace std;
// default constructor
DArray::DArray()
{
	Init();
}

// set an array with default values
DArray::DArray(int nSize, double dValue)
	: m_pData(new double[nSize]), m_nSize(nSize), m_nMax(nSize)
{
	for (int i = 0; i < nSize; i++)
		m_pData[i] = dValue;
}

DArray::DArray(const DArray& arr)
	:m_pData(new double[arr.m_nSize]), m_nSize(arr.m_nSize), m_nMax(arr.m_nSize)
{
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
	printf("size=%d: ", m_nSize);
	for (int i = 0; i < m_nSize; i++)
	{
		printf("%7.2f", m_pData[i]);
	}
	printf("\n");
}

// initilize the array
void DArray::Init()
{
	m_pData = nullptr;
	m_nMax = 0;
	m_nSize = 0;
}

// free the array
void DArray::Free()
{
	delete[]m_pData;
	m_pData = nullptr;
	m_nSize = 0;
	m_nMax = 0;
}


void DArray::Reserve(int nSize)
{
	if (m_nMax >= nSize)
	{
		return;
	}
	while (m_nMax < nSize)
	{
		m_nMax = m_nMax == 0 ? 1 : 2 * m_nMax;
	}
	double* pData = new double[m_nMax];
	memcpy(pData, m_pData, m_nSize * sizeof(double));
	delete[] m_pData;
	m_pData = pData;
}

// get the size of the array
int DArray::GetSize() const
{
	return m_nSize;
}

// set the size of the array
void DArray::SetSize(int nSize)
{
	assert(nSize >= 0);
	if (m_nSize == nSize)
	{
		return;
	}
	Reserve(nSize);
	for (int i = m_nSize; i < m_nMax; i++)
	{
		m_pData[i] = 0;
	}
}

// get an element at an index
const double& DArray::GetAt(int nIndex) const
{
	assert(nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
}

// set the value of an element 
void DArray::SetAt(int nIndex, double dValue)
{
	assert(nIndex >= 0 && nIndex < m_nSize);
	m_pData[nIndex] = dValue;
}

// overload operator '[]'
double& DArray::operator[](int nIndex)
{
	assert(nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
}

// overload operator '[]'
const double& DArray::operator[](int nIndex) const
{
	assert(nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
}

// add a new element at the end of the array
void DArray::PushBack(double dValue)
{
	Reserve(m_nSize + 1);
	m_pData[m_nSize] = dValue;
	m_nSize++;
}

// delete an element at some index
void DArray::DeleteAt(int nIndex)
{
	assert(nIndex >= 0 && nIndex < m_nSize);
	for (int i = nIndex; i < m_nSize-1; i++)
	{
		m_pData[i] = m_pData[i + 1];
	}
	m_pData[m_nSize - 1] = 0;
	m_nSize--;
}

// insert a new element at some index
void DArray::InsertAt(int nIndex, double dValue)
{
	assert(nIndex >= 0 && nIndex <= m_nSize);
	Reserve(m_nSize + 1);
	for (int i = m_nSize; i > nIndex; i--)
	{
		m_pData[i] = m_pData[i - 1];
	}
	m_pData[nIndex] = dValue;
	m_nSize++;
}

// overload operator '='
DArray& DArray::operator = (const DArray& arr)
{
	Reserve(arr.m_nSize);
	memcpy(m_pData, arr.m_pData, arr.m_nSize * sizeof(double));
	m_nSize = arr.m_nSize;
	return *this;
}
