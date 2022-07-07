// implementation of class DArray
#include "DArray.h"
#include"iostream"
#include "assert.h"


using namespace std;
// default constructor
template<class T>
DArray<T>::DArray()
{
	Init();
}

// set an array with default values
template<class T>
DArray<T>::DArray(int nSize, T dValue)
	: m_pData(new T[nSize]), m_nSize(nSize), m_nMax(nSize)
{
	for (int i = 0; i < nSize; i++)
		m_pData[i] = dValue;
}

template<class T>
DArray<T>::DArray(const DArray& arr)
	:m_pData(new T[arr.m_nSize]), m_nSize(arr.m_nSize), m_nMax(arr.m_nSize)
{
	for (int i = 0; i < m_nSize; i++)
	{
		m_pData[i] = arr.m_pData[i];
	}
}


// deconstructor
template<class T>
DArray<T>::~DArray()
{
	Free();
}

// display the elements of the array
template<class T>
void DArray<T>::Print() const
{
	printf("size=%d: ", m_nSize);
	for (int i = 0; i < m_nSize; i++)
	{
		cout << m_pData[i] << " ";
	}
	printf("\n");
}

// initilize the array
template<class T>
void DArray<T>::Init()
{
	m_pData = nullptr;
	m_nMax = 0;
	m_nSize = 0;
}

// free the array
template<class T>
void DArray<T>::Free()
{
	delete[]m_pData;
	m_pData = nullptr;
	m_nSize = 0;
	m_nMax = 0;
}

template<class T>
void DArray<T>::Reserve(int nSize)
{
	if (m_nMax >= nSize)
	{
		return;
	}
	while (m_nMax < nSize)
	{
		m_nMax = m_nMax == 0 ? 1 : 2 * m_nMax;
	}
	T* pData = new T[m_nMax];
	memcpy(pData, m_pData, m_nSize * sizeof(T));
	delete[] m_pData;
	m_pData = pData;
}

// get the size of the array
template<class T>
int DArray<T>::GetSize() const
{
	return m_nSize;
}

// set the size of the array
template<class T>
void DArray<T>::SetSize(int nSize)
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
template<class T>
const T& DArray<T>::GetAt(int nIndex) const
{
	assert(nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
}

// set the value of an element 
template<class T>
void DArray<T>::SetAt(int nIndex, T dValue)
{
	assert(nIndex >= 0 && nIndex < m_nSize);
	m_pData[nIndex] = dValue;
}

// overload operator '[]'
template <class T>
T& DArray<T>::operator[](int nIndex)
{
	assert(nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
}

// overload operator '[]'
template <class T>
const T& DArray<T>::operator[](int nIndex) const
{
	assert(nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
}

// add a new element at the end of the array
template <class T>
void DArray<T>::PushBack(T dValue)
{
	Reserve(m_nSize + 1);
	m_pData[m_nSize] = dValue;
	m_nSize++;
}

// delete an element at some index
template<class T>
void DArray<T>::DeleteAt(int nIndex)
{
	assert(nIndex >= 0 && nIndex < m_nSize);
	for (int i = nIndex; i < m_nSize - 1; i++)
	{
		m_pData[i] = m_pData[i + 1];
	}
	m_pData[m_nSize - 1] = 0;
	m_nSize--;
}

// insert a new element at some index
template<class T>
void DArray<T>::InsertAt(int nIndex, T dValue)
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
template<class T>
DArray<T>& DArray<T>::operator = (const DArray<T>& arr)
{
	Reserve(arr.m_nSize);
	memcpy(m_pData, arr.m_pData, arr.m_nSize * sizeof(T));
	m_nSize = arr.m_nSize;
	return *this;
}
