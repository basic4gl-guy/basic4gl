/*	NetUtils.h
	
	Created 9-Jan-2004: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	Utility classes for net engine.
*/

#ifndef _NetUtils_h
#define _NetUtils_h

#include <assert.h>

namespace NetLib4Games {

///////////////////////////////////////////////////////////////////////////////
//	NetRevolvingBitBuffer
//
/// Used to track which packets have already been received by their ID.
/// Current implementation as an array of bools.
/// Could be replaced by a proper bit array as an optimisation.

class NetRevolvingBitBuffer {

	unsigned int	m_size;
	bool			*m_data;

	// Position pointers
	unsigned int	m_hi, 
					m_hiRevolving,
					m_low,
					m_lowRevolving;

public:
	NetRevolvingBitBuffer (unsigned int size, unsigned int initialTop);
	~NetRevolvingBitBuffer ();

	unsigned int	Hi ()	{ return m_hi; }
	unsigned int	Low ()	{ return m_low; }

	bool InRange (unsigned int index) {
		return (index >= m_low && index < m_hi);
	}
	unsigned int Revolving (unsigned int index) {
		assert (InRange (index));
		return (m_lowRevolving + index - m_low) % m_size;
	}
	bool& Value (unsigned int index) {
		return m_data [Revolving (index)];
	}
	void SetTop (unsigned int index, bool initialValue, bool& truesRemoved, bool& falsesRemoved);
};

}

#endif
