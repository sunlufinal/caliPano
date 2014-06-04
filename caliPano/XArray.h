///////////////////////////////////////////////////////////////////////////////
//
//	XArray.h
//
//	Header file for the CXArray class
//	represents an extendible array
//
//	Alan Brunton 2008
//
///////////////////////////////////////////////////////////////////////////////


#ifndef __AB_XARRAY_H__
#define __AB_XARRAY_H__


#define XARRAY_INIT_BUFFER_LENGTH	8

#include <iostream>

template<class T> class CXArray
{
private:

	///////////////////////////////////////////////////////////////////////////
	//member variables
	///////////////////////////////////////////////////////////////////////////

	int								m_iStart;
	int								m_nLength;
	int								m_nBufferLength;
	T*								m_pBuffer;


	///////////////////////////////////////////////////////////////////////////
	//accessors/modifiers
	///////////////////////////////////////////////////////////////////////////

	bool validatePush()
	{
		int i, iBound;
		T* pNewBuffer;

		if (m_nLength == m_nBufferLength)
		{
			m_nBufferLength *= 2;
			pNewBuffer = new T[m_nBufferLength];
			if (pNewBuffer == NULL)
			{
				m_nBufferLength = m_nLength;
				return false;
			}

			iBound = m_iStart + m_nLength;
			for (i = m_iStart; i < iBound && i < m_nLength; ++i)
				pNewBuffer[i] = m_pBuffer[i];
			for (; i < iBound; ++i)
				pNewBuffer[i] = m_pBuffer[i - m_nLength];

			delete [] m_pBuffer;
			m_pBuffer = pNewBuffer;
		}

		return true;
	}

	bool validatePop()
	{
		int i, iBound;
		T* pNewBuffer;

		if (m_nLength <= (m_nBufferLength >> 2) && m_nBufferLength > XARRAY_INIT_BUFFER_LENGTH)
		{
			m_nBufferLength >>= 1;
			pNewBuffer = new T[m_nBufferLength];
			if (pNewBuffer == NULL)
			{
				m_nBufferLength <<= 2;
				return false;
			}

			iBound = m_iStart + m_nLength;
			if (m_iStart >= m_nBufferLength)
			{
				for (i = 0; i < m_nLength; ++i)
					pNewBuffer[i] = m_pBuffer[i + m_iStart];
				m_iStart = 0;
			}
			else
			{
				for (i = m_iStart; i < iBound && i < m_nBufferLength; ++i)
					pNewBuffer[i] = m_pBuffer[i];
				iBound -= m_nBufferLength;
				for (i = 0; i < iBound; ++i)
					pNewBuffer[i] = m_pBuffer[i + m_iStart];
			}

			delete [] m_pBuffer;
			m_pBuffer = pNewBuffer;
		}

		return true;
	}


public:

	///////////////////////////////////////////////////////////////////////////
	//constructors/destructor
	///////////////////////////////////////////////////////////////////////////
	
        CXArray(): m_iStart(0), m_nLength(0), m_nBufferLength(0), m_pBuffer(NULL)
	{
		init();
	}

	~CXArray()
	{
		clear();
	}


	///////////////////////////////////////////////////////////////////////////
	//accessors/modifiers
	///////////////////////////////////////////////////////////////////////////

	T& start() {return m_pBuffer[m_iStart];}

	T& end()
	{
		int iBack = m_iStart + m_nLength - 1;
		if (iBack >= m_nBufferLength)
			iBack -= m_nBufferLength;
		return m_pBuffer[iBack];
	}

	T& operator[](int index)
	{
		int i = m_iStart + index;
		if (i >= m_nBufferLength)
			i -= m_nBufferLength;
		return m_pBuffer[i];
	}

	T& at(int index)
	{
		int i = m_iStart + index;
		if (i >= m_nBufferLength)
			i -= m_nBufferLength;
		return m_pBuffer[i];
	}

	int length() const {return m_nLength;}


	///////////////////////////////////////////////////////////////////////////
	//actions/operations
	///////////////////////////////////////////////////////////////////////////

	void clear()
	{
		if (m_pBuffer != NULL)
		{
			delete [] m_pBuffer;
			m_pBuffer = NULL;
		}
		m_nBufferLength = 0;
		m_nLength = 0;
		m_iStart = 0;
	}

	void init()
	{
		clear();
		m_pBuffer = new T[XARRAY_INIT_BUFFER_LENGTH];
		if (m_pBuffer != NULL)
			m_nBufferLength = XARRAY_INIT_BUFFER_LENGTH;
	}

	void fastInit()
	{
		m_iStart = m_nLength = 0;
	}

	bool pushStart(T& item)
	{
		if (!validatePush())
			return false;
		--m_iStart;
		if (m_iStart < 0)
			m_iStart = m_nBufferLength - 1;
		m_pBuffer[m_iStart] = item;
		++m_nLength;
		return true;
	}

	bool pushEnd(T& item)
	{
		int iNew;

		if (!validatePush())
			return false;
		iNew = m_iStart + m_nLength;
		if (iNew >= m_nBufferLength)
			iNew -= m_nBufferLength;
		m_pBuffer[iNew] = item;
		++m_nLength;
		return true;
	}

	bool popStart()
	{
		++m_iStart;
		if (m_iStart == m_nBufferLength)
			m_iStart = 0;
		--m_nLength;
		return validatePop();
	}

	bool popEnd()
	{
		--m_nLength;
		return validatePop();
	}

	T* elementAddress(int index)
	{
		int i = m_iStart + index;
		if (i >= m_nBufferLength)
			i -= m_nBufferLength;
		return m_pBuffer + i;
	}

	void copy(CXArray<T>& xa)
	{
		int i;

		fastInit();
		for (i = 0; i < xa.length(); i++)
			pushEnd(xa[i]);
	}

	CXArray<T>& operator=(CXArray<T>& xa)
	{
		copy(xa);
		return *this;
	}

	void removeItemAt(int index)
	{
		int i, j;

		if (index > (m_nLength >> 1))
		{
			i = index + 1;
			for (j = m_iStart + i; j < m_nBufferLength && i < m_nLength; i++, j++)
				m_pBuffer[j - 1] = m_pBuffer[j];
			if (i < m_nLength)
			{
				m_pBuffer[m_nBufferLength - 1] = m_pBuffer[0];
				for (i++, j = 1; i < m_nLength; i++, j++)
					m_pBuffer[j - 1] = m_pBuffer[j];
			}
			popEnd();
		}
		else
		{
			i = index - 1;
			for (j = m_iStart + i; j >= 0 && i >= 0; i--, j--)
				m_pBuffer[j + 1] = m_pBuffer[j];
			if (i >= 0)
			{
				m_pBuffer[0] = m_pBuffer[m_nBufferLength - 1];
				for (i--, j = m_nBufferLength - 2; i >= 0; i--, j--)
					m_pBuffer[j + 1] = m_pBuffer[j];
			}
			popStart();
		}
	}

	void removeOrderedItems(int nitems, int* pindices)
	{
		int i, j, offset;

		if (nitems > 0)
		{
			offset = 1;
			for (i = 0; i < nitems - 1; i++)
			{
				for (j = pindices[i] + 1; j < pindices[i + 1]; j++)
					at(j - offset) = at(j);
			
				offset++;
			}
			for (j = pindices[nitems - 1] + 1; j < m_nLength; j++)
				at(j - offset) = at(j);

			m_nLength -= nitems;
			validatePop();
		}
	}

	void shrinkWrap()
	{
		T* pTempCopy = getArrayCopy();
		int nTempLen = m_nLength;

		clear();

		m_iStart = 0;
		m_nBufferLength = m_nLength = nTempLen;
		m_pBuffer = pTempCopy;
	}

	T* getArrayCopy()
	{
		int nEnd = m_iStart + m_nLength;
		int nFirstCopy = m_nBufferLength - m_iStart;
		int nSecondCopy = nEnd - m_nBufferLength;
		T* pBufferCopy = new T[m_nLength];

		if (pBufferCopy != NULL)
		{
			if (nSecondCopy > 0)
			{
				::memcpy(pBufferCopy, m_pBuffer + m_iStart,	nFirstCopy * sizeof(T));
				::memcpy(pBufferCopy + nFirstCopy, m_pBuffer, nSecondCopy * sizeof(T));
			}
			else
				::memcpy(pBufferCopy, m_pBuffer + m_iStart, m_nLength * sizeof(T));
		}

		return pBufferCopy;
	}

	void copyToArray(T* pData, int nLength)
	{
	        int nCopy = std::min(m_nLength, nLength);
		int nEnd = m_iStart + nCopy;
		int nFirstCopy = m_nBufferLength - m_iStart;
		int nSecondCopy = nEnd - m_nBufferLength;

		if (pData != NULL)
		{
			if (nSecondCopy > 0)
			{
				::memcpy(pData, m_pBuffer + m_iStart,	nFirstCopy * sizeof(T));
				::memcpy(pData + nFirstCopy, m_pBuffer, nSecondCopy * sizeof(T));
			}
			else
				::memcpy(pData, m_pBuffer + m_iStart, nCopy * sizeof(T));
		}
	}

	void copyArray(T* pData, int nLength)
	{
		int i;
		int nCopy = std::min(m_nLength, nLength);
		int nEnd = m_iStart + nCopy;
		int nFirstCopy = m_nBufferLength - m_iStart;
		int nSecondCopy = nEnd - m_nBufferLength;

		if (pData != NULL)
		{
			if (nSecondCopy > 0)
			{
				::memcpy(m_pBuffer + m_iStart, pData, nFirstCopy * sizeof(T));
				::memcpy(m_pBuffer, pData + nFirstCopy, nSecondCopy * sizeof(T));
			}
			else
				::memcpy(m_pBuffer + m_iStart, pData, nCopy * sizeof(T));

			//push remain item in array on the end
			for (i = nCopy; i < nLength; i++)
				pushEnd(pData[i]);
		}
	}

	friend std::ostream& operator<<( std::ostream& out, const CXArray<T>& arr ) {
	  T* pStart = arr.m_pBuffer + arr.m_iStart;
	  int iBack = arr.m_iStart + arr.m_nLength - 1;
	  // Deal with circular storage
	  bool wrapAround = false;
	  if (iBack >= arr.m_nBufferLength) { 
	    iBack = arr.m_nBufferLength-1;
	    wrapAround = true;
	  }
	  for ( int i = arr.m_iStart; i<iBack; ++i ) {
	    out << *(pStart++) << " "; 
	  }
	  if ( wrapAround ) {
	    pStart = arr.m_pBuffer;
	    iBack = arr.m_iStart + arr.m_nLength - arr.m_nBufferLength - 1;
	    for ( int i = arr.m_iStart; i<iBack; ++i ) {
	      out << *(pStart++) << " "; 
	    }
	  }
	  out << std::endl;
	  return out;
	};
	
	// Sets arr to T values from the stream, maximum of n are read 
	// if n < 0 read only stops if no more T can be read
	friend std::istream& readN( std::istream& in, CXArray<T>& arr, int n=-1 ) {
	  arr.clear();
	  arr.fastInit();
	  T data;  
	  int cnt=0;
	  while ( in >> data && (n<0 || cnt < n)) { 
	    arr.pushEnd( data );
	    ++cnt;
	  }
	  if (!in) in.clear(); 
	  return in;
	};

	friend std::istream& operator>>( std::istream& in, CXArray<T>& arr ) {
	  return readN( in, arr );
	};
};


#endif //__AB_XARRAY_H__


