/* VWCL - The Virtual Windows Class Library.
Copyright (C) 1996-2000 The VWCL Alliance

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

Primary Author of this source code file:  Todd Osborne (todd@vwcl.org)
Other Author(s) of this source code file:
*/

#ifndef VENUMUNKNOWN
#define VENUMUNKNOWN

#include "../vstandard.h"
#include "vunknown.hpp"
#include "vactivexenum.hpp"

/** VEnumUnknown is an implementation of the IEnumUnknown interface which
is used to enumerate through the objects in a component containing multiple
objects. VEnumUnknown has the same methods as all enumerator interfaces:
Next(), Skip)(), Reset(), and Clone(). For general information on these
methods, refer to IEnumXXX in the Win32 documentation. You can implement
this whenever you want a caller to be able to enumerate the objects
contained in another object. You get a pointer to IEnumUnknown through a
call to IOleContainer::EnumObjects. Call the methods of IEnumUnknown to
enumerate the objects in a compound document, when you get a pointer to
the interface on the enumerator through a call to
IOleContainer::EnumObjects(). It is important to note that this class
makes a private copy of the IUnknown object pointer(s) that are passed to
the Create() function of this class. These objects are AddRef()'d. This
ensures the objects will be available at least as long as this object is in
scope. It also means that client code does not have to keep the
IUnknown objects available in memory.*/
class VEnumUnknown : public IEnumUnknown, public VActiveXEnum
{
public:
	/** Create and initialize an instance of this object. If a memory error
	occurs, NULL will be returned. ppUnknowns must be a valid pointer to
	an IUnknown object, or a pointer to an array of them. nCountOfElements
	is the number of IUnknown objects pointed to by ppUnknowns, if this
	represents an array. The IUnknown object(s) must already have been
	created or retreived by client code, and are copied into the new
	objects private data and AddRef()'d to ensure they remain in scope at
	least as long as this object.*/
	static VEnumUnknown*	Create(	IUnknown**	ppUnknowns,
									VULONG		nCountOfElements = 1)
		{ VWCL_ACTIVEX_ENUM_IMPLEMENT_CREATE(VEnumUnknown, ppUnknowns) }

	/** Implement IUnknown methods.*/
	VWCL_ACTIVEX_IMPLEMENT_IUNKNOWN_METHODS(VUnknown)

	/** Creates another enumerator that contains the same enumeration state
	as the current one. Using this function, a client can record a
	particular point in the enumeration sequence, and then return to that
	point at a later time. The new enumerator supports the same interface
	as the original one.*/
	STDMETHODIMP			Clone(IEnumUnknown** ppIEnumUnknown)
	{
		VWCL_ACTIVEX_ENUM_IMPLEMENT_CLONE(VEnumUnknown, m_ppUnknowns, ppIEnumUnknown)
	}

	/** Retrieves the next nCountOfElements items in the enumeration
	sequence. If there are fewer than the requested number of elements
	left in the sequence, it retrieves the remaining elements. The number of
	elements actually retrieved is returned through pnCountOfElementsFetched
	(unless the caller passed in NULL for that parameter). ppUnknowns must
	be a pointer to a pointer to an IUnknown object, or a pointer to a
	pointer of an array of them. nCountOfElements is the number of
	IUnknown object pointers to return, if this represents an array. The
	IUnknown pointers do not need to have been initialized by client code
	as they are initialized by this function.*/
	STDMETHODIMP			Next(	VULONG		nCountOfElements,
									IUnknown**	ppUnknowns,
									VULONG*		pnCountOfElementsFetched)
		{ VWCL_ACTIVEX_ENUM_IMPLEMENT_NEXT(m_ppUnknowns, ppUnknowns) }

	/** Resets the enumeration sequence to the beginning. COM SPEC: A call
	to this method, resetting the sequence, does not guarantee that the
	same set of objects will be enumerated after the reset, because it
	depends on the collection being enumerated. A static collection is
	reset to the beginning, but it can be too expensive for some
	collections, such as files in a directory, to guarantee this condition.*/
	STDMETHODIMP			Reset()
		{ return VActiveXEnum::Reset(); }

	/** Skips over the next nCountOfElements elements in the enumeration
	sequence. Returns S_OK if the number of elements skipped is
	nCountOfElements, otherwise S_FALSE.*/
	STDMETHODIMP			Skip(VULONG nCountOfElements)
		{ return VActiveXEnum::Skip(nCountOfElements); }

protected:
	/** Protected constructor used for cloning and private initialization.*/
	VEnumUnknown(	IUnknown**	ppUnknowns,
					VULONG		nCountOfElements,
					VULONG		nCurrentIndex)
		: VActiveXEnum(IID_IEnumUnknown, nCountOfElements, nCurrentIndex)
	{
		VWCL_ACTIVEX_ENUM_IMPLEMENT_CONSTRUCTOR(m_ppUnknowns, IUnknown, ppUnknowns)
	}

	/** Protected destructor releases allocated memory and objects.*/
	virtual ~VEnumUnknown()
		{ VWCL_ACTIVEX_ENUM_IMPLEMENT_DESTRUCTOR(m_ppUnknowns) }

	/** Array of IUnknown objects that we copy and own.*/
	IUnknown**				m_ppUnknowns;
};

#endif /* VENUMUNKNOWN*/
