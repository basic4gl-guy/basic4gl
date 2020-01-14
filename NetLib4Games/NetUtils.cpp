/*	NetUtils.cpp
	
	Created 9-Jan-2004: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	Utility classes for net engine.
*/

#include "NetUtils.h"
#include <assert.h>

namespace NetLib4Games {

///////////////////////////////////////////////////////////////////////////////
//	NetRevolvingBitBuffer
NetRevolvingBitBuffer::NetRevolvingBitBuffer (unsigned int size, unsigned int initialTop) :
	m_size (size), 
	m_hi (initialTop), 
	m_low (initialTop), 
	m_hiRevolving (0), 
	m_lowRevolving (0) {
	
	assert (m_size > 0);
	m_data = new bool [m_size];
}

NetRevolvingBitBuffer::~NetRevolvingBitBuffer () {
	delete[] m_data;
}

void NetRevolvingBitBuffer::SetTop (unsigned int index, bool initialValue, bool& truesRemoved, bool& falsesRemoved) {
	truesRemoved	= false;
	falsesRemoved	= false;

	// Remove low array elements
	if (index > m_size) {
		unsigned int lowIndex = index - m_size;		
		while (m_low < lowIndex) {

			// Calculate value removed.
			bool value = m_low < m_hi ? m_data [m_lowRevolving] : initialValue;

			// Update flags accordingly
			truesRemoved  = truesRemoved  ||  value;
			falsesRemoved = falsesRemoved || !value;

			// Update low poiners
			if (++m_lowRevolving >= m_size)
				m_lowRevolving = 0;			
			m_low++;
		}
	}

	// Initialise high array elements
	while (m_hi < index) {

		// Initialise data
		if (m_hi >= m_low)
			m_data [m_hiRevolving] = initialValue;

		// Update high pointers
		if (++m_hiRevolving >= m_size)
			m_hiRevolving = 0;
		m_hi++;
	}
}

}
