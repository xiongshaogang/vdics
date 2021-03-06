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

#ifndef VACTIVEXENUM
#define VACTIVEXENUM

#include "../vstandard.h"
#include "vunknown.hpp"

/** Macro to implement a constructor for enumeration objects that
enumerate other COM objects.*/
#define VWCL_ACTIVEX_ENUM_IMPLEMENT_CONSTRUCTOR(m_pp_class_objects, class_object_type, pp_objects)	\
	VASSERT(pp_objects)												\
	m_pMostDerivedThis = this;										\
	m_pp_class_objects = VNEW class_object_type*[nCountOfElements];	\
	if ( m_pp_class_objects )										\
	{																\
		for ( VULONG i = 0; i < nCountOfElements; i++ )				\
		{															\
			pp_objects[i]->AddRef();								\
			m_pp_class_objects[i] = pp_objects[i];					\
		}															\
	}																\
	else															\
		Error(VTRUE);

/** Macro to implement a constructor for enumeration objects that enumerate
non-COM objects.*/
#define VWCL_ACTIVEX_ENUM_IMPLEMENT_CONSTRUCTOR_NON_OBJECT(m_p_class_array, class_type, p_array)	\
	VASSERT(p_array)										\
	m_pMostDerivedThis = this;								\
	m_p_class_array = VNEW class_type[nCountOfElements];	\
	if ( m_p_class_array )									\
	{														\
		for ( VULONG i = 0; i < nCountOfElements; i++ )		\
			m_p_class_array[i] = p_array[i];				\
	}														\
	else													\
		Error(VTRUE);

/** Macro to implement a destructor for enumeration objects that enumerate
other COM objects.*/
#define VWCL_ACTIVEX_ENUM_IMPLEMENT_DESTRUCTOR(m_pp_class_objects)	\
	for ( VULONG i = 0; i < m_nCountOfElements; i++ )	\
		m_pp_class_objects[i]->Release();				\
	VDELETE_ARRAY m_pp_class_objects;

/** Macro to implement a destructor for enumeration objects that enumerate
non-COM objects.*/
#define VWCL_ACTIVEX_ENUM_IMPLEMENT_DESTRUCTOR_NON_OBJECT(m_p_class_array)	\
	VDELETE_ARRAY m_p_class_array;

/** Macro to implement IEnumXXX::Create() for enumeration objects that
enumerate other COM objects.*/
#define VWCL_ACTIVEX_ENUM_IMPLEMENT_CREATE(class_name, pp_objects)			\
		class_name* pObj = VNEW class_name(pp_objects, nCountOfElements, 0);\
		if ( pObj && !pObj->IsInErrorState() )								\
			return pObj;													\
		if ( pObj )															\
			pObj->Release();												\
		return NULL;

/** Macro to implement IEnumXXX::Create() for enumeration objects that
enumerate non-COM objects.*/
#define VWCL_ACTIVEX_ENUM_IMPLEMENT_CREATE_NON_OBJECT(class_name, p_array)	\
	VWCL_ACTIVEX_ENUM_IMPLEMENT_CREATE(class_name, p_array)

/** Macro to implement IEnumXXX::Clone() for enumeration objects that
enumerate other COM objects.*/
#define VWCL_ACTIVEX_ENUM_IMPLEMENT_CLONE(class_name, m_pp_class_objects, pp_enum_interface)	\
	if ( pp_enum_interface )							\
	{													\
		*pp_enum_interface = NULL;						\
		if ( m_pp_class_objects && m_nCountOfElements )	\
		{												\
			class_name* pNew =							\
				VNEW class_name(m_pp_class_objects,		\
								m_nCountOfElements,		\
								m_nCurrentIndex);		\
			if ( pNew && !pNew->IsInErrorState() )		\
			{											\
				*pp_enum_interface = pNew;				\
				return S_OK;							\
			}											\
			if ( pNew )									\
				pNew->Release();						\
			return E_OUTOFMEMORY;						\
		}												\
		return E_UNEXPECTED;							\
	}													\
	return E_INVALIDARG;

/** Macro to implement IEnumXXX::Clone() for enumeration objects that
enumerate non-COM objects.*/
#define VWCL_ACTIVEX_ENUM_IMPLEMENT_CLONE_NON_OBJECT(class_name, m_p_class_array, pp_enum_interface)\
	VWCL_ACTIVEX_ENUM_IMPLEMENT_CLONE(class_name, m_p_class_array, pp_enum_interface)

/** Macro to implement IEnumXXX::Next() for enumeration objects that
enumerate other COM objects.*/
#define VWCL_ACTIVEX_ENUM_IMPLEMENT_NEXT(m_pp_class_objects, pp_objects)	\
	if ( pp_objects )														\
		VZEROMEMORY(pp_objects, nCountOfElements * 4);						\
	if ( pnCountOfElementsFetched )											\
		*pnCountOfElementsFetched = 0;										\
	VULONG nGetCount = VMIN(nCountOfElements, m_nCountOfElements - m_nCurrentIndex);	\
	if ( pp_objects && nGetCount )											\
	{																		\
		for ( VULONG i = 0; i < nGetCount; i++ )							\
		{																	\
			m_pp_class_objects[m_nCurrentIndex + i]->AddRef();				\
			pp_objects[i] = m_pp_class_objects[m_nCurrentIndex + i];		\
		}																	\
		m_nCurrentIndex += nGetCount;										\
		if ( pnCountOfElementsFetched )										\
			*pnCountOfElementsFetched = nGetCount;							\
		return S_OK;														\
	}																		\
	return S_FALSE;

/** Macro to implement IEnumXXX::Next() for enumeration objects that
enumerate non-COM objects.*/
#define VWCL_ACTIVEX_ENUM_IMPLEMENT_NEXT_NON_OBJECT(m_p_class_array, p_array, sizeof_single_element)\
	HRESULT hResult = S_FALSE;											\
	if ( p_array )														\
		VZEROMEMORY(p_array, nCountOfElements * sizeof_single_element);	\
	if ( pnCountOfElementsFetched )										\
		*pnCountOfElementsFetched = 0;									\
	VULONG nGetCount = VMIN(nCountOfElements, m_nCountOfElements - m_nCurrentIndex);	\
	if ( p_array && nGetCount )											\
	{																	\
		for ( VULONG i = 0; i < nGetCount; i++ )						\
			p_array[i] = m_p_class_array[m_nCurrentIndex + i];			\
		m_nCurrentIndex += nGetCount;									\
		if ( pnCountOfElementsFetched )									\
			*pnCountOfElementsFetched = nGetCount;						\
		hResult = S_OK;													\
	}

/** VActiveXEnum is the base for for other IEnumXXX COM enumeration
implementation objects and cannot be directly instantiated.*/
class VActiveXEnum : public VUnknown
{
protected:
	/** Protected constructor used by derived classes initializes class
	variables and base class.*/
	VActiveXEnum(	IID const&	classIID,
					VULONG		nCountOfElements,
					VULONG		nCurrentIndex =
									0)
		: VUnknown(classIID)
	{
		VASSERT(nCountOfElements)
		m_nCountOfElements =	nCountOfElements;
		m_nCurrentIndex =		nCurrentIndex;
	}

	/** Set the current error condition. On error, this function will reset
	internal element count and current index.*/
	virtual void	Error(VBOOL bError)
	{
		VUnknown::Error(bError);

		if ( bError )
			m_nCountOfElements = m_nCurrentIndex = 0;
	}

	/** Resets the enumeration sequence to the beginning. COM SPEC: A call
	to this method, resetting the sequence, does not guarantee that the
	same set of objects will be enumerated after the reset, because it
	depends on the collection being enumerated. A static collection is
	reset to the beginning, but it can be too expensive for some collections,
	such as files in a directory, to guarantee this condition.*/
	STDMETHODIMP	Reset()
	{
		m_nCurrentIndex = 0;
		return S_OK;
	}

	/** Skips over the next specified number of elements in the enumeration
	sequence. Return S_OK if the number of elements skipped is
	nCountOfElements, otherwise S_FALSE.*/
	STDMETHODIMP	Skip(VULONG nCountOfElements)
	{
		if ( m_nCurrentIndex + nCountOfElements < m_nCountOfElements )
		{
			m_nCurrentIndex += nCountOfElements;
			return S_OK;
		}

		return S_FALSE;
	}

	/** Embedded Members.*/
	VULONG			m_nCountOfElements;
	VULONG			m_nCurrentIndex;
};

#endif /* VACTIVEXENUM*/
