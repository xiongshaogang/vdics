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

#ifndef VDIRECTORYTREEVIEW
#define VDIRECTORYTREEVIEW

#include "../vstandard.h"
#include "../collections/vsortedstringlinkedlist.hpp"
#include "../numbers/vrtti.hpp"
#include "../strings/vstring.hpp"
#include "../utility/vmemoryblock.hpp"
#include "../utility/vrecursivedirectoryenumerator.hpp"
#include "../utility/vwaitcursor.hpp"
#include "../windows/vwindow.hpp"
#include "vcommoncontrol.hpp"
#include "vtreeview.hpp"

/** VDirectoryTreeView displays directory and/or file structure in a Win32
Tree common control. The tree can only display drives that have an assigned
drive letter, and cannot currently deal with UNC drive mapping. This may be
added in a future version.*/
class VDirectoryTreeView :
	public VTreeView, public VRecursiveDirectoryEnumeratorCallback
{
public:
	/** Default constructor initializes default options and RTTI information.*/
	VDirectoryTreeView(	VUINT		nOptions =
									OPTION_TOGGLE_IMAGE_ON_EXPAND |
									OPTION_DELAYED_POPULATION |
									OPTION_DIR_TREE_DISPLAY_DIRECTORIES |
									OPTION_DIR_TREE_DISPLAY_FILES |
									OPTION_DIR_TREE_DISPLAY_FILE_EXTENSIONS |
									OPTION_DIR_TREE_DISPLAY_REMOTE |
									OPTION_DIR_TREE_DISPLAY_REMOVABLE |
									OPTION_DIR_TREE_DISPLAY_FIXED |
									OPTION_DIR_TREE_DISPLAY_CDROM |
									OPTION_DIR_TREE_DISPLAY_RAMDISK,
						VRTTI_TYPE	nRTTI = VWCL_RTTI_DIRECTORY_TREEVIEW)
		: VTreeView(nOptions, nRTTI)
		{;}

	/** The type represented by every item in the tree is held in the
	lParam (user data) of each tree item.*/
	enum {				TYPE_DRIVE_UNKNOWN,
						TYPE_DRIVE_NO_ROOT_DIR,
						TYPE_DRIVE_REMOVABLE,
						TYPE_DRIVE_FIXED,
						TYPE_DRIVE_REMOTE,
						TYPE_DRIVE_CDROM,
						TYPE_DRIVE_RAMDISK,
						TYPE_DIRECTORY,
						TYPE_FILE,
						TYPE_USER,
		};

	/** Add a new directory below hParent to the tree. This does NOT create
	the directory on the disk. Returns the HTREEITEM of the new directory
	on success, NULL on failure.*/
	HTREEITEM			AddDirectory(	HTREEITEM		hParent,
										VSTRING_CONST	pszName)
		{ return AddFolderOrFile(hParent, pszName, VTRUE); }

	/** Add a new file below hParent to the tree. This does NOT create the
	file on the disk. Returns the HTREEITEM of the new file on success,
	NULL on failure.*/
	HTREEITEM			AddFile(	HTREEITEM		hParent,
									VSTRING_CONST	pszName)
		{ return AddFolderOrFile(hParent, pszName, VFALSE); }

	/** This function builds the volume information displayed to the user.
	This base class function builds a string from the drive letter and
	volume name in a manner that is consistent with Windows Explorer. An
	override of this method can perform any other type of display string
	generation. It is important to note that if you override this method,
	you may also need to override FindRootDirectoryParent() and possibly
	GetItemText(). For volumes that are not named, this method will use
	the drive letter only.*/
	virtual void		BuildVolumeDisplay(	VString&		strBuffer,
											VSTRING_CONST	pszRootPath)
	{
		VCHAR sz[VWCL_MAX_SINGLE_TREE_ITEM_LENGTH];

		/* If volume information cannot be returned, put None
		in its place.*/
		if ( GetVolumeInformation(	pszRootPath,
									sz,
									VARRAY_SIZE(sz),
									NULL,
									NULL,
									NULL,
									NULL,
									0) )
		{
			strBuffer = sz;
		}
			
		if ( pszRootPath )
		{
			if ( strBuffer.IsNotEmpty() )
				strBuffer += VTEXT(" ");

			strBuffer += VTEXT("(");
			strBuffer.Append(pszRootPath, 2);
			strBuffer += VTEXT(")");
		}
	}

	/** Given a drive letter and trailing colon, find the root item that is
	considered to be the parent folder for the specified drive. The base class
	function looks for a root item that contains the drive letter and colon
	anywhere in the tree text.*/
	virtual HTREEITEM	FindRootDirectoryParent(VSTRING_CONST pszDriveLetter)
	{
		/* Init() must have been called!*/
		HTREEITEM hRootParent = GetChild(NULL);
		VASSERT(hRootParent)

		/* We only look at the first 2 characters.*/
		VString sDriveLetter(pszDriveLetter, 2);
		VString sItem;

		/* Find child item where the drive letter and : is found.
		We will not use FindChildItemByString() since that would
		populate the entire root of the tree!*/
		while ( hRootParent )
		{
			if ( GetItemText(sItem, hRootParent) )
			{
				if ( sItem.FindFirst(sDriveLetter, VFALSE) != -1 )
					return hRootParent;

				hRootParent = GetNextSib(hRootParent);
			}
			else
				break;
		}

		return NULL;
	}

	/** Given a HTREEITEM, get the parent directory. If hItem is a file, the
	parent directory (folder) or hItem is returned. If hItem is already a
	folder, hItem is returned, not its parent. If hItem is not a directory
	or file, NULL is returned. If hItem is NULL on entry, ItemOrSelection()
	will be used to get it.*/
	HTREEITEM			GetDirectory(HTREEITEM hItem) const
	{
		/* Get default?*/
		hItem = ItemOrSelection(hItem);

		/* Is hItem a file? If yes, get the parent folder.*/
		if ( IsItemFile(hItem) )
			hItem = GetParent(hItem);

		return hItem;
	}

	/** Return a string that identifies the installed drives. This function
	is not directly related to the tree, but the information is required
	for Init(). This can be a CPU-Disk intensive operation, so when
	multiple tree's may need to be displayed, it is best to hold on to this
	string throughout the application and use it in each call to Init(). The
	string returned is simply the VString strBuffer's internal buffer. On
	failure, strBuffer will not be modified in any way.*/
	static VSTRING		GetDrives(VString& strBuffer)
	{
		/* Find out how much memory is required.*/
		VDWORD nMemRequired = GetLogicalDriveStrings(0, NULL);

		VMemoryBlock memBlock;

		/* Allocate memory now.*/
		if ( nMemRequired && memBlock.AllocForString(nMemRequired) )
		{
			GetLogicalDriveStrings(nMemRequired, (VSTRING)memBlock.Get());
			strBuffer = memBlock;

			return strBuffer;
		}

		return NULL;
	}

	/** Return a reference to the string object that holds a list of file
	extension to show.This string is separated by \ characters. Each
	extension must have the leading . before the extension. For example:
	FileExtensions(".CPP\.HPP\.H")
	NOT
	FileExtensions("CPP\HPP\H").*/
	VString&			GetFileExtensions() const
		{ return (VString&)m_strFileExtensions; }

	/** Override gets text, but for root items, returns only the drive letter,
	not volume info. The returned text will be the drive letter and the
	trailing colon.*/
	virtual VSTRING		GetItemText(	VString&	strBuffer,
										HTREEITEM	hItem =
														NULL,
										VUINT		nMaxChars =
														0)
	{
		/* If hItem is a root item, get text, but fix-up so that it
		contains only the drive letter.*/
		if (	VTreeView::GetItemText(strBuffer, hItem, nMaxChars) &&
				IsRootItem(hItem) )
		{
			/* Find the colon marker.*/
			VINT nColon = strBuffer.FindFirst(VTEXT(':'));

			if ( nColon )
				strBuffer = strBuffer.Mid(nColon - 1, 2);
		}

		return strBuffer;
	}

	/** Initialize this class. Gets drive information and initializes root
	drives into tree. This should be called after initializing the image
	list (if one is used). The image list can be initialized with the
	standard VWCL dirimage.bmp file, which contains the images in the
	correct order. If pszDrives is NULL, the function will get the drive
	information, which can be a CPU-Disk intensive task.*/
	void				Init(	VSTRING_CONST	pszDrives =
													NULL,
								HTREEITEM		hParent =
													TVI_ROOT)
	{
		VString strDrives;

		/* Get drive information if pszDrives is NULL.*/
		VSTRING pszLocalDrives =
			(pszDrives) ? (VSTRING)pszDrives : GetDrives(strDrives);

		VASSERT(VSTRLEN_CHECK(pszLocalDrives))

		if ( pszLocalDrives )
		{
			/* Loop until pszThisDrive is NULL, adding drives to tree.*/
			VSTRING pszThisDrive = pszLocalDrives;

			while ( pszThisDrive && pszThisDrive[0] != VTEXT('\0') )
			{
				/* Determine Drive Type.*/
				VUINT nDriveType = GetDriveType(pszThisDrive);

				/* Should we add this one to the tree?*/
				VBOOL bAdd = VTRUE;

				if (	nDriveType == DRIVE_REMOTE &&
						m_Options.IsNotSet(OPTION_DIR_TREE_DISPLAY_REMOTE) )
					bAdd = VFALSE;
				else if (	nDriveType == DRIVE_REMOVABLE &&
							m_Options.IsNotSet(
								OPTION_DIR_TREE_DISPLAY_REMOVABLE) )
					bAdd = VFALSE;
				else if (	nDriveType == DRIVE_FIXED &&
							m_Options.IsNotSet(OPTION_DIR_TREE_DISPLAY_FIXED) )
					bAdd = VFALSE;
				else if (	nDriveType == DRIVE_CDROM &&
							m_Options.IsNotSet(OPTION_DIR_TREE_DISPLAY_CDROM) )
					bAdd = VFALSE;
				else if (	nDriveType == DRIVE_RAMDISK &&
							m_Options.IsNotSet(
								OPTION_DIR_TREE_DISPLAY_RAMDISK) )
					bAdd = VFALSE;

				if ( bAdd )
				{
					VString sVolumeName;
					BuildVolumeDisplay(sVolumeName, pszThisDrive);

					/* On error, use DRIVE_REMOVABLE.*/
					if (	nDriveType < DRIVE_REMOVABLE ||
							nDriveType > DRIVE_RAMDISK )
						nDriveType = DRIVE_REMOVABLE;

					/* Add to tree at root.*/
					InternalAddElement(hParent, sVolumeName, nDriveType);
				}

				/* Offset the pointer.*/
				pszThisDrive += VSTRLEN(pszThisDrive) + 1;
			}
		}
	}

	/** Initialize this class, copying root items from an existing,
	initialized tree control. If the existing tree is not initialized, it
	will be initialized with Init() using default parameters.*/
	void				InitCopy(VDirectoryTreeView const& ExistingTreeView)
	{
		/* If the existing tree is not initialized, do that instead.*/
		if ( ExistingTreeView.GetCount() )
		{
			VCHAR sz[VWCL_MAX_SINGLE_TREE_ITEM_LENGTH];

			VDECLARE_TV_INSERTSTRUCT
			is.hParent =			NULL;
			is.hInsertAfter =		TVI_LAST;
			is.item.mask =			TVIF_HANDLE |
									TVIF_IMAGE |
									TVIF_SELECTEDIMAGE |
									TVIF_PARAM |
									TVIF_STATE |
									TVIF_TEXT;
			is.item.pszText =		sz;
			is.item.cchTextMax =	VARRAY_SIZE(sz);

			/* Copy each root item from ExistingTreeView to this tree.*/
			HTREEITEM hNext = ExistingTreeView.GetChild(NULL);
			VASSERT(hNext)

			while ( hNext )
			{
				is.item.hItem =	hNext;

				/* Get this items information from the existing tree. This
				is basically a deep copy, where we get all information from
				current item to insert into this tree to make an exact
				duplicate of the tree items.*/
				if ( ExistingTreeView.GetItem(is.item) )
					InternalAddElement(TVI_ROOT, sz, is.item.lParam);

				/* Get the next sibling.*/
				hNext = ExistingTreeView.GetNextSib(hNext);
			}
		}
		else
			Init();
	}

	/** Tests to see if hItem represents a file. Returns VTRUE if it does,
	VFALSE otherwise.*/
	VBOOL				IsItemFile(HTREEITEM hItem = NULL) const
		{ return (LParam(hItem) == TYPE_FILE) ? VTRUE : VFALSE; }

	/** Tests to see if hItem represents a directory. Returns VTRUE if it
	does, VFALSE otherwise.*/
	VBOOL				IsItemDirectory(HTREEITEM hItem = NULL) const
		{ return (LParam(hItem) == TYPE_DIRECTORY) ? VTRUE : VFALSE; }

	/** Used to add directory and file items to tree.*/
	HTREEITEM			InternalAddElement(	HTREEITEM		hParent,
											VSTRING_CONST	pszName,
											VLPARAM			nType)
	{
		VASSERT(hParent)
		VASSERT(VSTRLEN_CHECK(pszName))

		VDECLARE_TV_INSERTSTRUCT
		is.hParent =			hParent;
		is.hInsertAfter =		TVI_LAST;
		is.item.mask =			TVIF_TEXT |
								TVIF_IMAGE |
								TVIF_SELECTEDIMAGE |
								TVIF_PARAM;
		is.item.pszText =		(VSTRING)pszName;
		is.item.lParam =		nType;

		/* Image index is 1 more than drive type, except for directories
		and files.*/
		is.item.iImage =
			(nType == TYPE_DIRECTORY || nType == TYPE_FILE)
			? GetImageIndex(NULL, nType, (nType == TYPE_DIRECTORY)
			? VTRUE
			: VFALSE, VFALSE)
			: nType + 1;

		is.item.iSelectedImage = is.item.iImage;

		/* Should we show it in bold?*/
		if (	nType == TYPE_DIRECTORY &&
				m_Options.IsSet(OPTION_DISPLAY_FOLDERS_IN_BOLD_TYPE) )
		{
			is.item.mask |=	TVIF_STATE;
			is.item.state =	is.item.stateMask = TVIS_BOLD;
		}

		/* Insert the item now.*/
		HTREEITEM hItem = InsertItem(is);

		/* If we inserted anything other than a file, add an expand marker.*/
		if ( hItem && nType != TYPE_FILE )
			InsertExpandMarker(hItem);

		return hItem;
	}

	/** Same as base class, but finds drive item to use for hParent (Pass it
	a fully qualified path). hRootItem can be NULL to search whole tree root
	for staring item, or can be a known owner of the directory tree portion.
	This function also forces case insensitive searches since filenames on
	Win32 are not case sensitive. Since this tree view cannot deal with UNC
	networking naming right now, this function will check to make sure the
	path specified by pszPath contains a valid drive letter, followed by a
	colon.*/
	HTREEITEM			NavigateTo(	VSTRING_CONST	pszPath,
									HTREEITEM		hParent =
														NULL,
									VSTRING_CONST	pszSeparator =
														VFILE_PATH_SEP)
	{
		/* Copy pszString since we modify it.*/
		VString sPath(pszPath);

		if ( sPath.IsNotEmpty() )
		{
			/* Parse to find drive letter.*/
			VDelimitedString	ds(sPath, pszSeparator[0]);
			VSTRING				pszDrive = ds.Next();

			if ( pszDrive )
			{
				/* Get parent folder for this drive.*/
				HTREEITEM hRootParent = FindRootDirectoryParent(pszDrive);
				
				/* Call base class, with now known parent, to complete.*/
				if ( hRootParent )
				{
					/* Is there another string or is only a drive given?*/
					VSTRING pszPathStart = ds.Next();

					if ( pszPathStart )
					{
						ds.FixLast();
						hRootParent = VTreeView::NavigateTo(pszPathStart,
															hRootParent,
															pszSeparator,
															VFALSE);
					}
					else
					{
						/* The only navigational information is the drive,
						so complete and return.*/
						EnsureVisible(hRootParent);
						Selection(hRootParent);
					}

					return hRootParent;
				}
			}
		}

		return NULL;
	}

	/** Populate tree starting at hParent. This is to support delayed
	population of the tree.*/
	virtual void		Populate(HTREEITEM hParent)
	{
		/* Parent must be known.*/
		VASSERT(hParent)

		/* Skip if hParent is a file.*/
		if ( IsItemFile(hParent) )
			return;

		/* This can sometimes take a long time, so show the wait cursor.*/
		VWaitCursor wait;

		/* Get the path at this location.*/
		VString s;

		/* Call base class function to enumerate items in this directory.*/
		if ( GetPath(s, hParent) )
		{
			VRecursiveDirectoryEnumerator	re;
			VSortedStringLinkedList			listDir;
			VSortedStringLinkedList			listFiles;

			/* Should we show directories?*/
			if ( m_Options.IsSet(OPTION_DIR_TREE_DISPLAY_DIRECTORIES) )
				re.EnumDirectories(VREF_THIS, s, &listDir);

			/* Should we show files?*/
			if ( m_Options.IsSet(OPTION_DIR_TREE_DISPLAY_FILES) )
			{
				/* Determine if we should strip file extensions.*/
				VBOOL bStripFileExtension =
					m_Options.IsNotSet(OPTION_DIR_TREE_DISPLAY_FILE_EXTENSIONS);

				/* Did the caller give us a list of valid file extensions
				to show?*/
				if ( m_strFileExtensions.IsNotEmpty() )
				{
					VString sExtension;

					/* Walk each extension in delimeted string.*/
					VDelimitedString
						ds(m_strFileExtensions, VFILE_PATH_SEP_CHAR);

					VSTRING pszExtension = ds.Next();

					while ( pszExtension )
					{
						/* The first char should be a period!*/
						VASSERT(pszExtension[0] == VTEXT('.'))

						/* Build * and then extension string.*/
						sExtension = VTEXT('*');
						sExtension += pszExtension;

						/* Get files of this type.*/
						if ( sExtension.IsNotEmpty() )
							re.EnumFiles(	VREF_THIS,
											s,
											sExtension,
											(bStripFileExtension)
											? (VVOIDPTR)bStripFileExtension
											: NULL,
											&listFiles);

						/* Next legal extension.*/
						pszExtension = ds.Next();
					}
				}
				else
					re.EnumFiles(	VREF_THIS,
									s,
									NULL,
									(bStripFileExtension)
									? (VVOIDPTR)bStripFileExtension
									: NULL,
									&listFiles);
			}

			/* Add to list. Directories first, then files.*/
			VLinkedListStringItem* pItem = listDir.GetFirstItem();

			while ( pItem )
			{
				InternalAddElement(hParent, pItem->String(), TYPE_DIRECTORY);
				pItem = pItem->GetNextItem();
			}

			pItem = listFiles.GetFirstItem();

			while ( pItem )
			{
				InternalAddElement(hParent, pItem->String(), TYPE_FILE);
				pItem = pItem->GetNextItem();
			}
		}
	}

protected:
	/** Internal function to add a new directory or file to the tree. Not
	used internally, but here to support letting client code add items to
	the tree via AddFolder() or AddFile(). This function keeps the tree
	properly sorted but does not do it like the Populate() function does.
	This is better for adding one or two items to the tree, but is not well
	suited for mass population like Populate() is.*/
	HTREEITEM			AddFolderOrFile(	HTREEITEM		hParent,
											VSTRING_CONST	pszName,
											VBOOL			bDirectory)
	{
		VASSERT(hParent)
		VASSERT(VSTRLEN_CHECK(pszName))

		/* Get directory.*/
		hParent = GetDirectory(hParent);

		VASSERT(hParent)

		VDECLARE_TV_INSERTSTRUCT;
		is.hParent =			hParent;
		is.item.mask =			TVIF_TEXT |
								TVIF_IMAGE |
								TVIF_SELECTEDIMAGE |
								TVIF_PARAM;
		is.item.pszText =		(VSTRING)pszName;
		is.item.iImage =		GetImageIndex(NULL, (bDirectory)
								? TYPE_DIRECTORY
								: TYPE_FILE,
								bDirectory,
								VFALSE);
		is.item.iSelectedImage=	is.item.iImage;
		is.item.lParam =		(bDirectory) ? TYPE_DIRECTORY : TYPE_FILE;

		/* Should we show it in bold?*/
		if (	bDirectory &&
				m_Options.IsSet(OPTION_DISPLAY_FOLDERS_IN_BOLD_TYPE) )
		{
			is.item.mask |=	TVIF_STATE;
			is.item.state =	is.item.stateMask = TVIS_BOLD;
		}

		/* Sort items as they are added to tree.*/
		is.hInsertAfter = TVI_FIRST;

		if ( GetCount() > 1 )
		{
			HTREEITEM	hNextSib = GetChild(hParent);
			VString		strThisItem;

			while ( hNextSib )
			{
				/* Get this items type. It is already in the tree, so
				get its LParam().*/
				VLPARAM nType2 = LParam(hNextSib);

				/* See if directory needs to preceed a file.*/
				if (	is.item.lParam == TYPE_FILE &&
						nType2 == TYPE_DIRECTORY )
					is.hInsertAfter = hNextSib;
				else if ( is.item.lParam == nType2 )
				{
					/* Does the current item go after this one? If not,
					we can exit now.*/
					if (	GetItemText(strThisItem, hNextSib) &&
							strThisItem.Compare(is.item.pszText) > 0 )
						is.hInsertAfter = hNextSib;
					else
						break;
				}

				/* Continue enumeration.*/
				hNextSib = GetNextSib(hNextSib);
			}
		}

		/* Insert the item now.*/
		HTREEITEM hItem = InsertItem(is);

		/* If we added a folder add an expand marker.*/
		if ( hItem && bDirectory )
			InsertExpandMarker(hItem);

		return hItem;
	}

	/** Override verifies tree is populated to this point unless item
	is a file.*/
	virtual VLONG		OnDoubleClick(	NMHDR&		nmhdr,
										VLPARAM&	lCommonControlResult)
	{
		/* Hit-test to determine item that was double clicked.*/
		VPoint pt;
		pt.GetCursorPosition();
		ScreenToClient(pt);

		HTREEITEM hItem = HitTest(pt.GetX(), pt.GetY());

		/* Attempt populate if not a file.*/
		if ( hItem && !IsItemFile(hItem) )
			PopulateIfNeeded(hItem);

		return VTreeView::OnDoubleClick(nmhdr, lCommonControlResult);
	}

	/** Prevent auto-expanding feature of base class which toggles directory
	state for root items, which have children, but do not show an "open
	folder" when children are shown.*/
	virtual VLONG		OnItemExpanded(	NM_TREEVIEW&	nmTreeView,
										VLPARAM&		lCommonControlResult)
	{
		if (	m_Options.IsSet(OPTION_TOGGLE_IMAGE_ON_EXPAND) &&
				IsRootItem(nmTreeView.itemNew.hItem))
			return 0;

		return VTreeView::OnItemExpanded(nmTreeView, lCommonControlResult);
	}

	/** Called by VRecursiveDirectoryEnumerator during population.*/
	virtual VBOOL		OnRecursiveFound(
		VSTRING					pszName,
		VSTRING_CONST			pszMask,
		WIN32_FIND_DATA const&	FindData,
		VBOOL					bDirectory,
		VVOIDPTR				pVoid1,
		VVOIDPTR				pVoid2)
	{
		/* Add directories to VSortedStringLinkedList in pVoid1, files
		to pVoid2.*/
		if ( bDirectory )
		{
			VASSERT(pVoid1)

			((VSortedStringLinkedList*)pVoid1)->
				Add((VSTRING_CONST)FindData.cFileName);
		}
		else
		{
			VASSERT(pVoid2)

			VSTRING pszExtension = NULL;

			/* Should we remove the extension? Yes is pVoid1 is not NULL.*/
			if ( pVoid1 )
			{
				pszExtension = VSTRRCHR((VSTRING_CONST)FindData.cFileName, VTEXT('.'));

				if ( pszExtension )
					pszExtension[0] = VTEXT('\0');
			}

			((VSortedStringLinkedList*)pVoid2)->
				Add((VSTRING)FindData.cFileName);

			if ( pszExtension )
				pszExtension[0] = VTEXT('.');
		}

		/* Get the next item.*/
		return VTRUE;
	}

	/** Embedded Members.*/
	VString			m_strFileExtensions;
};

#endif /* VDIRECTORYTREEVIEW*/
