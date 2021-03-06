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

#ifndef VSHELLREGISTRAR
#define VSHELLREGISTRAR

#include "../vstandard.h"
#include "../strings/vpathstring.hpp"
#include "../strings/vstring.hpp"
#include "vregistry.hpp"

/* Local string constant(s).*/
static VSTRING_CONST VSHELLREGISTRAR_STRING_SHELL_OPEN_COMMAND =
	VTEXT("shell\\open\\command");

/** VShellRegistrar is used when adding commands to, or removing them from,
the system shell (Explorer). It can also be used to read information stored
by other applications, or for asking the system to return file mapping
associations.*/
class VShellRegistrar
{
public:
	/** Given a file extension, including the leading period, get the path to
	the application that can open the file. On success, this path will be
	stored in the strBuffer string object and a pointer to it returned. On
	failure, NULL is returned and strBuffer is not modified. If the
	associated application is found in the registry, but does not exist at
	the specified location, this function will return NULL as if the
	association was not found at all. If this is not desired, pass VFALSE
	for bCheckPath.*/
	static VSTRING	GetApplicationFromFileExtension(
		VSTRING_CONST	pszDotFileExtension,
		VString&		strBuffer,
		VBOOL			bCheckPath =
							VTRUE)
	{
		VSTRING pszBuffer =
			InternalGetApplicationFromFileExtension(	pszDotFileExtension,
														strBuffer);

		/* Check the path?*/
		if ( pszBuffer && bCheckPath && !VDoesFileExist(pszBuffer) )
			pszBuffer = strBuffer.Empty();

		return pszBuffer;
	}

	/** Append an applications file extension and command with the system. To
	remove them during un-install, call RemoveAppendCommands().
	pszDotFileExtension is the file extension used when the system should
	launch your application when the user clicks on a file of your type.
	This must include the leading period. For instance, if your application
	deals with TXT files, pass .TXT for pszDotFileExtension. This function
	DOES not associate a file extension with your application. It is
	intended to allow the user to launch your application from the shell
	using a command that you specify. It differs from RegisterOpenCommand()
	by NOT associating a file extension to your program, but rather
	extending the commands that can be performed on a file type. For
	instance, if you are writing an HTML editor, you don't want to associate
	all HTML files with your editor, but do want to offer the user the
	ability to launch your program with an HTML file opened. The command you
	choose could be "Open with Killer Editor". This is what you would pass
	as the pszCommand parameter. This will add your command to the available
	commands Explorer shows the user when they select an HTML file. The
	application name is required, and if NULL, VGetAppTitle() will be used.
	Returns VTRUE on success, VFALSE on failure. This function will not
	fail if the file extension type is not already registered on the system.
	Instead the file type will be registered as pszFileType, if specified,
	and your command appended to the explorer options. If pszFileType is not
	known, the application name will be used.*/
	static VBOOL	AppendCommand(	VSTRING_CONST	pszDotFileExtension,
									VSTRING_CONST	pszCommand,
									VSTRING_CONST	pszFileType =
														NULL,
									VSTRING_CONST	pszAppTitle =
														NULL,
									HINSTANCE		hAltModuleHandle =
														NULL)
	{
		return InternalAppendCommand(	VTRUE,
										pszDotFileExtension,
										pszCommand,
										pszFileType,
										pszAppTitle,
										hAltModuleHandle);
	}

	/** Register an applications file extension and type with the system. To
	remove them during un-install, call RemoveOpenCommand().
	pszDotFileExtension is the file extension used when the system should
	launch your application when the user clicks on a file of your type.
	This must include the leading period. For instance, if your application
	deals with TXT files, pass .TXT for pszDotFileExtension. pszFileType is
	the name of the file association the system displays for the
	pszDotFileExtension type. For TXT, the system displays "Text File". You
	can customize what the user see's with pszFileType. Returns VTRUE on
	success, VFALSE on failure.*/
	static VBOOL	OpenCommand(	VSTRING_CONST	pszDotFileExtension,
									VSTRING_CONST	pszFileType,
									HINSTANCE		hAltModuleHandle =
														NULL)
	{
		/* These must be known!*/
		VASSERT(VSTRLEN_CHECK(pszDotFileExtension))
		VASSERT(VSTRLEN_CHECK(pszFileType))

		/* Must start with period.*/
		VASSERT(pszDotFileExtension[0] == VTEXT('.'))

		if ( !pszDotFileExtension || !pszFileType )
			return VFALSE;

		VBOOL bSuccess = VFALSE;

		VRegistry reg;

		if ( reg.CreateKey(pszDotFileExtension, HKEY_CLASSES_ROOT) )
		{
			reg.WriteString((VSTRING_CONST)NULL, pszFileType);
			reg.Close();

			/* Set the file type so Explorer can display this instead
			of just "xxx file".*/
			if ( reg.CreateKey(pszFileType, HKEY_CLASSES_ROOT) )
			{
				reg.WriteString((VSTRING_CONST)NULL, pszFileType);

				/* Create the default icon key.*/
				VRegistry regDefaultIcon;

				if ( regDefaultIcon.CreateKey(	VTEXT("DefaultIcon"),
												reg.GetKey()) )
				{
					/* Get module path and append ,0 to it.*/
					VString s;
					VPathString ps(s);
					
					if (	ps.LoadModulePath(	VFALSE,
												VFALSE,
												hAltModuleHandle) &&
							s.Append(VTEXT(",0")) )
					{
						regDefaultIcon.WriteString((VSTRING_CONST)NULL, s);
						regDefaultIcon.Close();
					}
				}

				/* Create the command key.*/
				VRegistry regCommand;

				if (	regCommand.CreateKey(
							VSHELLREGISTRAR_STRING_SHELL_OPEN_COMMAND,
							reg.GetKey()) )
				{
					reg.Close();

					/* Get module path. Filename must be in quotes for
					long filenames to work.*/
					VString s(VTEXT("\""));
					VPathString ps(s);

					if (	ps.AppendModulePath(VFALSE,
												VFALSE,
												hAltModuleHandle) &&
							s.Append(VTEXT("\" \"%1\"")) )
					{
						/* Write this key.*/
						regCommand.WriteString((VSTRING_CONST)NULL, s);

						/* Success!*/
						bSuccess = VTRUE;
					}
				}
			}
		}

		return bSuccess;
	}

	/** Remove appended shell commands installed by the AppendCommands()
	functions.*/
	static VBOOL	RemoveAppendCommand(
		VSTRING_CONST	pszDotFileExtension,
		VSTRING_CONST	pszAppTitle =
							NULL,
		HINSTANCE		hAltModuleHandle =
							NULL)
	{
		return InternalAppendCommand(	VFALSE,
										pszDotFileExtension,
										NULL,
										NULL,
										pszAppTitle,
										hAltModuleHandle);
	}

	/** Remove shell open commands installed by OpenCommand() function(s).*/
	static void		RemoveOpenCommand(
		VSTRING_CONST pszDotFileExtension,
		VSTRING_CONST pszFileType)
	{
		/* These must be known!*/
		VASSERT(VSTRLEN_CHECK(pszDotFileExtension))
		VASSERT(VSTRLEN_CHECK(pszFileType))

		/* Must start with .*/
		VASSERT(pszDotFileExtension[0] == VTEXT('.'))

		if (	pszDotFileExtension &&
				pszFileType &&
				pszDotFileExtension[0] == VTEXT('.') )
		{
			/* Remove shell type.*/
			VRegistry::DeleteKey(HKEY_CLASSES_ROOT, pszDotFileExtension);

			/* Remove shell open command.*/
			VRegistry::DeleteKey(HKEY_CLASSES_ROOT, pszFileType);
		}
	}

protected:
	/** Internal version of GetApplicationFromFileExtension() that does not
	validate the path unless required.*/
	static VSTRING	InternalGetApplicationFromFileExtension(
		VSTRING_CONST	pszDotFileExtension,
		VString&		strBuffer)
	{
		/* This must be known!*/
		VASSERT(VSTRLEN_CHECK(pszDotFileExtension))

		/* Must start with period.*/
		VASSERT(pszDotFileExtension[0] == VTEXT('.'))

		if ( !pszDotFileExtension )
			return NULL;

		VRegistry reg;

		/* Open this key.*/
		if ( reg.OpenKey(	pszDotFileExtension,
							HKEY_CLASSES_ROOT,
							KEY_QUERY_VALUE | KEY_READ) )
		{
			VString strRoot;

			/* Get the file type from this key.*/
			if ( reg.ReadString((VSTRING_CONST)NULL, strRoot) )
			{
				/* Close the current key.*/
				reg.Close();

				VString strKey(strRoot);

				/* Now that we have the file type, build the key to
				the open shell command.*/
				strKey += VTEXT('\\');
				strKey += VSHELLREGISTRAR_STRING_SHELL_OPEN_COMMAND;

				/* Open this key.*/
				if ( reg.OpenKey(	strKey,
									HKEY_CLASSES_ROOT,
									KEY_QUERY_VALUE | KEY_READ) )
				{
					VString strPath;

					/* Get the path to the program that opens this
					file type.*/
					if ( reg.ReadString((VSTRING_CONST)NULL, strPath) )
					{
						/* Convert to lowercase.*/
						strPath.LowerCase();

						/* Find where the filename part ends.*/

						/* If it begins with ", look for trailing " and
						terminate there.*/
						if ( strPath.CharAt(0) == VTEXT('"') )
						{
							VSTRING pszEnd =
								VSTRCHR(strPath.String() + 1, VTEXT('"'));

							/* The string started with " but was never
							ended with one!*/
							VASSERT(pszEnd)

							if ( pszEnd )
								pszEnd[0] = VTEXT('\0');

							strBuffer = strPath.Mid(1);

							return strBuffer;
						}

						/* It appears that the path is not enclosed in "
						characters.*/

						/* See if we can find a % in the string. That will
						mark the start of parameters.*/
						VSTRING pszTerm =
							VSTRCHR((VSTRING_CONST)strPath, VTEXT('%'));

						/* Can we find a %?*/
						if ( pszTerm )
							pszTerm--;
						else
							pszTerm = strPath.String() + strPath.GetLength();

						/* Parse backward until we get a valid file name.*/
						while ( pszTerm > strPath.String() )
						{
							if ( VDoesFileExist(strPath) )
							{
								/* The string could still contain white
								space or extra characters. Get rid of it.*/
								while (	pszTerm[0] == VTEXT(' ') ||
										!VISPRINT(pszTerm[0]) )
								{
									pszTerm[0] = VTEXT('\0');
									pszTerm--;
								}

								return strBuffer.String(strPath);
							}

							pszTerm[0] = VTEXT('\0');
							pszTerm--;
						}
					}
				}
			}
		}

		return NULL;
	}

	/** Internal function to add or append shell commands.*/
	static VBOOL	InternalAppendCommand(
		VBOOL			bAppend,
		VSTRING_CONST	pszDotFileExtension,
		VSTRING_CONST	pszCommand,
		VSTRING_CONST	pszFileType,
		VSTRING_CONST	pszAppTitle = NULL,
		HINSTANCE		hAltModuleHandle = NULL)
	{
		/* This must be known!*/
		VASSERT(VSTRLEN_CHECK(pszDotFileExtension))

		/* Must start with .*/
		VASSERT(pszDotFileExtension[0] == VTEXT('.'))

		/* Assume failure.*/
		VBOOL bSuccess = VFALSE;

		if (	pszDotFileExtension &&
				pszDotFileExtension[0] == VTEXT('.')	)
		{
			/* If the application name is not given, get it.*/
			if ( !pszAppTitle )
				pszAppTitle = VGetAppTitle();

			VRegistry	reg;
			VString		strRoot;

			/* If we fail to open the key, and we are appending, create it.*/
			if (	!reg.OpenKey(pszDotFileExtension, HKEY_CLASSES_ROOT) &&
					bAppend )
			{
				/* Set root to known value now.*/
				strRoot = (pszFileType) ? pszFileType : pszAppTitle;

				/* If we create the key, and know the file type, set it.
				If the file type is not given, use application name.*/
				if ( reg.CreateKey(pszDotFileExtension, HKEY_CLASSES_ROOT) )
					reg.WriteString((VSTRING_CONST)NULL, strRoot);
			}

			/* Did we open or create the registry key?*/
			if ( reg.GetKey() )
			{
				/* Get the file type from this key, unless already known.*/
				if (	strRoot.IsNotEmpty() ||
						reg.ReadString((VSTRING_CONST)NULL, strRoot) )
				{
					/* Close the current key.*/
					reg.Close();

					/* Build string to the shell part of the tree and
					append the application name.*/
					strRoot += VTEXT('\\');
					strRoot += VTEXT("shell");
					strRoot += VTEXT('\\');
					strRoot += pszAppTitle;

					if ( bAppend )
					{
						/* This must be known!*/
						VASSERT(VSTRLEN_CHECK(pszCommand))

						/* Open this key, for writing this time.*/
						if (	pszCommand &&
								reg.CreateKey(strRoot, HKEY_CLASSES_ROOT) )
						{
							/* The the command to show to the user as
							the key value.*/
							reg.WriteString((VSTRING_CONST)NULL, pszCommand);

							/* Close this key.*/
							reg.Close();

							/* Append the command string.*/
							strRoot += VTEXT('\\');
							strRoot += VTEXT("command");

							/* Open this key.*/
							if ( reg.CreateKey(strRoot, HKEY_CLASSES_ROOT) )
							{
								/* Get module path. Filename must be
								in quotes for long filenames to work.*/
								VString s(VTEXT("\""));
								VPathString ps(s);

								if ( ps.AppendModulePath(
										VFALSE,
										VFALSE,
										hAltModuleHandle) &&
										s.Append(VTEXT("\" \"%1\"")) )
								{
									/* Write this value.*/
									reg.WriteString((VSTRING_CONST)NULL, s);

									/* Success!*/
									bSuccess = VTRUE;
								}
							}
						}
					}
					else
						bSuccess = VRegistry::DeleteKey(HKEY_CLASSES_ROOT,
														strRoot);
				}
			}
		}

		return bSuccess;
	}

	/** Embedded Members.*/
	HKEY			m_hKey;
};

#endif /* VSHELLREGISTRAR*/
