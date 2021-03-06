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

#ifndef VSSSTORAGEHELPER
#define VSSSTORAGEHELPER

/** Structured Storage helpers with enhancements including ANSI/ASCII/UNICODE
based string functions, boolean return values, and extended functionality.
When using these classes it is assumed that the class takes ownership of the
structured storage interface pointer. As such, it will call Release() when
appropriate. If calling code wishes to keep an interface pointer longer than
the scope of this object, you will need to AddRef() the interface before this
object goes out of scope. Doing this, however, may prevent the rename element
caching from working since it can only rename elements after they have been
released for the final time. Care must be taken here.*/

#include "../vstandard.h"
#include "../collections/vptrarray.hpp"
#include "../strings/vstringconverter.hpp"
#include "vunknown.hpp"
#include "vssstreamhelper.hpp"

/** VSSStorageHelper wraps access to IStorage and IStream objects.*/
class VSSStorageHelper : public VSSObjectHelper
{
public:
	/** Default constructor initializes by taking ownership of an existing
	IStorage object, or none.*/
	VSSStorageHelper(IStorage* pStorage = NULL)
		: VSSObjectHelper(VSS_RTTI_STORAGE, (IUnknown*)pStorage, NULL)
		{;}

	/** Constructor to initialize from a structured storage file. If
	bCreateIfNeeded is VTRUE, a file will be created if the file does
	not already exist. In either case, the mode of opening should be
	specified or left at defaults.*/
	VSSStorageHelper(	VSTRING_CONST	pszFileName,
						VBOOL			bCreateIfNeeded =
											VTRUE,
						VDWORD			nMode =
											VSS_DEFAULT_MODE)
		: VSSObjectHelper(VSS_RTTI_STORAGE, NULL, NULL)
	{
		m_nMode = nMode;
		OpenFile(pszFileName, bCreateIfNeeded);
	}

	/** Copy the contents of this storage object to another.*/
	VBOOL				CopyTo(IStorage* pDestStorage) const
	{
		VASSERT(pDestStorage)
		VASSERT(GetStorage())

		return (SUCCEEDED(GetStorage()->CopyTo(	0,
												NULL,
												NULL,
												pDestStorage)))
												? VTRUE : VFALSE;
	}

	/** Create a named storage as a sub-storage of this object, returning a
	pointer to it on success, NULL on failure. The object returned from this
	function is owned by this class, and its lifetime is controlled here.
	Client code should not attempt to Release() or delete this object.*/
	VSSStorageHelper*	CreateStorage(VSTRING_CONST pszName)
		{ return CreateOrOpenStorage(pszName, VTRUE); }

	/** Create a named stream as a sub-stream of this object, returning a
	pointer to it on success, NULL on failure. The object returned from
	this function is owned by this class, and its lifetime is controlled here.
	Client code should not attempt to Release() or delete this object.*/
	VSSStreamHelper*	CreateStream(VSTRING_CONST pszName)
		{ return CreateOrOpenStream(pszName, VTRUE); }

	/** Destroy a named element, a sub-storage or sub-stream. Returns VTRUE on
	success, VFALSE on failure. If the object is owned by this class, it will
	be released and removed from the internal list of elements. If not, the
	named element will simply be destroyed and removed from the storage.
	Returns VTRUE on success, VFALSE on failure.*/
	VBOOL				DestroyElement(VSTRING_CONST pszName)
		{ return ReleaseElement(pszName, VTRUE); }

	/** Retrieves a pointer to an enumerator object that can be used to
	enumerate the storage and stream objects contained within this storage
	object. Returns VTRUE on success, VFALSE on failure. This is not the same
	as walking the internal elements list. The enumerator is an ActiveX
	provided enumerator and does not neccessarily represent open sub-elements.
	In addition, the element names will not contain any renamed elements.*/
	VBOOL				EnumElements(IEnumSTATSTG** ppIEnumSTATSTG)
	{
		VASSERT(GetStorage())

		return (SUCCEEDED(GetStorage()->EnumElements(	0,
														NULL,
														0,
														ppIEnumSTATSTG)))
														? VTRUE : VFALSE;
	}

	/** Given a named storage, locate the storage that is owned by this object.
	The object returned from this function is owned by this class, and its
	lifetime is controlled here. Client code should not attempt to Release()
	or delete this object.*/
	VSSStorageHelper*	FindStorage(VSTRING_CONST pszName) const
	{
		return (VSSStorageHelper*)FindObject(	pszName,
												VSS_RTTI_STORAGE,
												NULL);
	}

	/** Given a named stream, locate the stream that is owned by this object.
	The object returned from this function is owned by this class, and its
	lifetime is controlled here. Client code should not attempt to Release()
	or delete this object.*/
	VSSStreamHelper*	FindStream(VSTRING_CONST pszName) const
	{
		return (VSSStreamHelper*)FindObject(	pszName,
												VSS_RTTI_STREAM,
												NULL);
	}

	/** Get the CLSID for this storage. Returns CLSID_NULL on failure.*/
	CLSID				GetClass()
	{
		STATSTG stg;
		return (Stat(stg)) ? stg.clsid : CLSID_NULL;
	}

	/** Return a reference to the internal element list array. The list contains
	VSSStorageHelper and VSSStreamHelper objects.*/
	VPtrArray&			GetCollection() const
		{ return (VPtrArray&)m_listElements; }

	/** Copies or moves a substorage or stream from this storage object to
	another storage object, possibly renaming it in the process if pszNewName
	is not NULL. Returns VTRUE on success, VFALSE on failure. If the element
	pszName is open, it will be closed and removed from the internal elements
	list.*/
	VBOOL				MoveElementTo(	VSTRING_CONST	pszName,
										IStorage*		pDestStorage,
										VSTRING_CONST	pszNewName =
															NULL,
										VDWORD			nFlags =
															STGMOVE_MOVE)
	{
		VASSERT(VSTRLEN_CHECK(pszName))
		VASSERT(pDestStorage)
		VASSERT(GetStorage())

		/* Release the element if needed.*/
		ReleaseElement(pszName, VFALSE);

		return SUCCEEDED(GetStorage()->MoveElementTo(
			VStringConverter(pszName),
			pDestStorage,
			VStringConverter((pszNewName) ? pszNewName : pszName),
			nFlags))
			? VTRUE : VFALSE;
	}

	/** Open a compound document file to initialize this object. The file is
	expected to already exist, and will be created only if bCreateIfNeeded
	is VTRUE. Returns VTRUE on success, VFALSE on failure.*/
	VBOOL				OpenFile(	VSTRING_CONST	pszFileName,
									VBOOL			bCreateIfNeeded =
														VTRUE)
	{
		/* This object should NOT already be initialized!*/
		VASSERT(!m_pWrappedObject)

		if ( FAILED(StgOpenStorage(
			VStringConverter(pszFileName),
			NULL,
			m_nMode,
			NULL,
			0,
			(IStorage**)&m_pWrappedObject)) && bCreateIfNeeded )
		{
			StgCreateDocfile(	VStringConverter(pszFileName),
								STGM_CREATE | m_nMode,
								0,
								(IStorage**)&m_pWrappedObject);
		}

		return (m_pWrappedObject) ? VTRUE : VFALSE;
	}

	/** Open a named storage as a sub-storage of this object, returning a
	pointer to it on success, NULL on failure. The object returned from
	this function is owned by this class, and its lifetime is controlled
	here. Client code should not attempt to Release() or delete this
	object. The element must already exist for this function to succeed.*/
	VSSStorageHelper*	OpenStorage(VSTRING_CONST pszName)
		{ return CreateOrOpenStorage(pszName, VFALSE); }

	/** Open a named stream as a sub-stream of this object, returning a pointer
	to it on success, NULL on failure. The object returned from this function
	is owned by this class, and its lifetime is controlled here. Client code
	should not attempt to Release() or delete this object. The element must
	already exist for this function to succeed.*/
	VSSStreamHelper*	OpenStream(VSTRING_CONST pszName)
		{ return CreateOrOpenStream(pszName, VFALSE); }

	/** Remove a contained child item from the list, deleting the object, but
	don't delete the element from the storage. Returns VTRUE if the object
	was removed from the internal list, VFALSE otherwise.*/
	VBOOL				ReleaseElement(VSTRING_CONST pszName)
		{ return ReleaseElement(pszName, VFALSE); }

	/** If RenameElement() fails because an element is open, set the rename
	string with GetRenameText().String() to cause the element to be renamed
	after release. Returns VTRUE on success, VFALSE on failure.*/
	VBOOL				RenameElement(	VSTRING_CONST pszOldName,
										VSTRING_CONST pszNewName)
	{
		VASSERT(VSTRLEN_CHECK(pszOldName))
		VASSERT(VSTRLEN_CHECK(pszNewName))
		VASSERT(GetStorage())

		return (SUCCEEDED(GetStorage()->RenameElement(
			VStringConverter(pszOldName),
			VStringConverter(pszNewName))))
			? VTRUE : VFALSE;
	}

	/** Assigns the specified CLSID to this storage object. Returns VTRUE on
	success, VFALSE on failure.*/
	VBOOL				SetClass(REFCLSID clsid)
	{
		VASSERT(GetStorage())
		return (SUCCEEDED(GetStorage()->SetClass(clsid))) ? VTRUE : VFALSE;
	}

	/** Sets the modification, access, and creation times of the specified
	storage element, if supported by the underlying file system. Any of
	the time values may be NULL if you do not need to alter them.*/
	VBOOL				SetElementTimes(VSTRING_CONST	pszName,
										FILETIME*		pCreateTime,
										FILETIME*		pLastAccessTime,
										FILETIME*		pModificationtime)
	{
		VASSERT(VSTRLEN_CHECK(pszName))
		VASSERT(GetStorage())

		return (SUCCEEDED(GetStorage()->SetElementTimes(
			VStringConverter(pszName),
			pCreateTime,
			pLastAccessTime,
			pModificationtime)))
			? VTRUE : VFALSE;
	}

	/** Stores up to 32 bits of state information in this storage object.
	Returns VTRUE on success, VFALSE on failure.*/
	VBOOL				SetStateBits(	VDWORD nStateBits,
										VDWORD nMask)
	{
		VASSERT(GetStorage())
		return (SUCCEEDED(GetStorage()->SetStateBits(	nStateBits,
														nMask)))
														? VTRUE : VFALSE;
	}

protected:
	/** Protected constructor use when this object create new storages.*/
	VSSStorageHelper(	IStorage*			pStorage,
						VSSStorageHelper*	pParentStorage)
		: VSSObjectHelper(VSS_RTTI_STORAGE, (IUnknown*)pStorage, pParentStorage)
	{
		VASSERT(pStorage)
		VASSERT(pParentStorage)
	}

	/** Internal function to create or open a sub-storage.*/
	VSSStorageHelper*	CreateOrOpenStorage(	VSTRING_CONST	pszName,
												VBOOL			bCreate)
	{
		VASSERT(VSTRLEN_CHECK(pszName))
		VASSERT(GetStorage())

		VSSStorageHelper*	pNewObject =	NULL;
		IStorage*			pNewStorage =	NULL;

		/* Create the storage? Otherwise open it.*/
		if ( bCreate )
			GetStorage()->CreateStorage(	VStringConverter(pszName),
											STGM_CREATE | m_nMode,
											0,
											0,
											&pNewStorage);
		else
			GetStorage()->OpenStorage(	VStringConverter(pszName),
										NULL,
										m_nMode,
										NULL,
										0,
										&pNewStorage);

		if ( pNewStorage )
		{
			/* Create the new wrapper for this object.*/
			pNewObject = VNEW VSSStorageHelper(pNewStorage, this);

			if ( pNewObject )
			{
				/* Add this object to our managed list.*/
				if ( m_listElements.Add(pNewObject) != -1 )
					return pNewObject;

				/* Delete object on failure.*/
				VDELETE_NULL(pNewObject)
			}

			/* Release the object.*/
			pNewStorage->Release();

			/* Destroy the element on failure if we create it.*/
			if ( bCreate )
				GetStorage()->DestroyElement(VStringConverter(pszName));
		}

		return pNewObject;
	}

	/** Internal function to create or open a sub-stream.*/
	VSSStreamHelper*	CreateOrOpenStream(	VSTRING_CONST	pszName,
											VBOOL			bCreate)
	{
		VASSERT(VSTRLEN_CHECK(pszName))
		VASSERT(GetStorage())

		VSSStreamHelper*	pNewObject =	NULL;
		IStream*			pNewStream =	NULL;

		/* Create the stream.? Otherwise open it.*/
		if ( bCreate )
			GetStorage()->CreateStream(	VStringConverter(pszName),
										STGM_CREATE | m_nMode,
										0,
										0,
										&pNewStream);
		else
			GetStorage()->OpenStream(	VStringConverter(pszName),
										NULL,
										m_nMode,
										0,
										&pNewStream);

		if ( pNewStream )
		{
			/* Create the new wrapper for this object.*/
			pNewObject = VNEW VSSStreamHelper(pNewStream, this);

			if ( pNewObject )
			{
				/* Add this object to our managed list.*/
				if ( m_listElements.Add(pNewObject) != -1 )
					return pNewObject;

				/* Delete object on failure.*/
				VDELETE_NULL(pNewObject)
			}

			/* Release the object.*/
			pNewStream->Release();

			/* Destroy the element on failure if we create it.*/
			if ( bCreate )
				GetStorage()->DestroyElement(VStringConverter(pszName));
		}

		return pNewObject;
	}

	/** Internal version of FindObject.*/
	VSSObjectHelper*	FindObject(	VSTRING_CONST	pszName,
									VRTTI_TYPE		nType,
									VINT*			pnIndex) const
	{
		VASSERT(VSTRLEN_CHECK(pszName))

		if ( pnIndex )
			*pnIndex = -1;

		VPTRARRAY_INDEX		nSize = m_listElements.Size();
		VSSObjectHelper*	pObject;
		VString				strName;

		for ( VPTRARRAY_INDEX i = 0; i < nSize; i++ )
		{
			/* Get this element.*/
			pObject = (VSSObjectHelper*)m_listElements[i];

			if (	pObject->GetRTTI() == nType &&
					pObject->GetName(strName) &&
					strName.CompareNoCase(pszName) )
			{
				if ( pnIndex )
					*pnIndex = i;

				return pObject;
			}
		}

		return NULL;
	}

	/** Internal function to release and optionally destroy an element.*/
	VBOOL				ReleaseElement(	VSTRING_CONST	pszName,
										VBOOL			bDestroyElement)
	{
		VASSERT(VSTRLEN_CHECK(pszName))
		VASSERT(GetStorage())

		VINT nIndex;

		VSSObjectHelper* pObject =
			FindObject(pszName, VSS_RTTI_STORAGE, &nIndex);

		/* Did we find a storage to delete? If not, look for a stream.*/
		if ( pObject )
			pObject = FindObject(pszName, VSS_RTTI_STREAM, &nIndex);

		/* Did we find an object?*/
		if ( pObject )
		{
			if ( bDestroyElement )
			{
				VString strName;

				/* Get the objects real name, before renaming, if any.*/
				if ( pObject->GetName(strName, VFALSE) )
				{
					/* Reset any renaming that may be in effect.*/
					m_strRename.Empty();

					/* Remove it from our managed list.*/
					m_listElements.RemoveAt(nIndex);

					/* Delete the object.*/
					VDELETE_NULL(pObject)

					/* Now destroy based on the real name.*/
					if ( SUCCEEDED(GetStorage()->
						DestroyElement(VStringConverter(strName))) )
					{
						return VTRUE;
					}
				}
			}
			else
			{
				/* Remove it from our managed list.*/
				m_listElements.RemoveAt(nIndex);

				/* Delete the object.*/
				VDELETE_NULL(pObject)

				return VTRUE;
			}
		}
		/* We don't own the object, so try to delete by name.*/
		else if ( bDestroyElement )
			return (SUCCEEDED(GetStorage()->
				DestroyElement(VStringConverter(pszName)))) ? VTRUE : VFALSE;

		return VFALSE;
	}


	/** List of internally opened VSStreamHelper or VSSStorageHelper objects.*/
	VPtrArray			m_listElements;
};

#endif	/* VSSSTORAGEHELPER*/
