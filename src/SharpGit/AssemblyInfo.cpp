// Copyright 2007-2009 The SharpSvn Project
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include "pch.h"
#include "GitLibraryAttribute.h"

#pragma warning(disable: 4635)
#pragma warning(disable: 4634) // XML document comment: cannot be applied:  Discarding XML document comment for invalid target.

#include <apr_version.h>
#include <apu_version.h>
#include <openssl/opensslv.h>
#include <zlib.h>
#include <expat.h>
#include <svn_version.h>
#include "../libsvn_subr/utf8proc/utf8proc_internal.h"

using namespace System;
using namespace System::Reflection;
using namespace System::Resources;
using namespace System::Runtime::CompilerServices;
using namespace System::Runtime::InteropServices;
using namespace System::Security::Permissions;
using SharpGit::Implementation::GitLibraryAttribute;

#define EXPAT_VERSION APR_STRINGIFY(XML_MAJOR_VERSION) "." APR_STRINGIFY(XML_MINOR_VERSION) "." APR_STRINGIFY(XML_MICRO_VERSION)
#define UTF8PROC_VERSION APR_STRINGIFY(UTF8PROC_VERSION_MAJOR) "." APR_STRINGIFY(UTF8PROC_VERSION_MINOR) "." APR_STRINGIFY(UTF8PROC_VERSION_PATCH)


// General Information about an assembly is controlled through the following
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
//
[assembly:AssemblyTitleAttribute("SharpGit - Git for .Net 2.0-3.5 and 4.0")];
[assembly:AssemblyDescriptionAttribute("SharpGit, compiled statically with libgit2 " LIBGIT2_VERSION
                                       ", apr " APR_VERSION_STRING
                                       ", apr-util " APU_VERSION_STRING
                                       ", eXpat " EXPAT_VERSION
                                       ", utf8proc " UTF8PROC_VERSION
                                       ", LibSSH2 " LIBSSH2_VERSION
                                       ", " OPENSSL_VERSION_TEXT
                                       ", ZLib " ZLIB_VERSION
                                       " and some platform support libraries of Subversion " SVN_VER_NUMBER)];
[assembly:AssemblyConfigurationAttribute("")];
[assembly:AssemblyCompanyAttribute("SharpSvn Project")];
[assembly:AssemblyProductAttribute("SharpGit")];
[assembly:AssemblyCopyrightAttribute("Copyright (c) SharpSvn Project 2013-2015")];
[assembly:AssemblyTrademarkAttribute("")];
[assembly:AssemblyCultureAttribute("")];

[assembly:GitLibrary("Libgit2", LIBGIT2_VERSION)]
[assembly:GitLibrary("Apr", APR_VERSION_STRING)];
[assembly:GitLibrary("Apr-Util", APU_VERSION_STRING)];
[assembly:GitLibrary("eXpat", EXPAT_VERSION)];
[assembly:GitLibrary("LibSSH2", LIBSSH2_VERSION)];
[assembly:GitLibrary("OpenSSL", OPENSSL_VERSION_TEXT, SkipPrefix = true)];
[assembly:GitLibrary("Subversion", SVN_VER_NUMBER)];
[assembly:GitLibrary("Utf8proc", UTF8PROC_VERSION)];
[assembly:GitLibrary("ZLib", ZLIB_VERSION)];


//
// Version information for an assembly consists of the following four values:
//
//      Major Version
//      Minor Version
//      Build Number
//      Revision
//
// You can specify all the value or you can default the Revision and Build Numbers
// by using the '*' as shown below:

[assembly:NeutralResourcesLanguageAttribute("en-US")];

[assembly:ComVisible(false)];

[assembly:CLSCompliantAttribute(true)];

#if __CLR_VER < 40000000
[assembly:SecurityPermission(SecurityAction::RequestMinimum, UnmanagedCode = true)];
#endif

//[assembly:RuntimeCompatibility(WrapNonExceptionThrows = true)];
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "winhttp.lib")

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Rpcrt4.lib")

//#include "../../imports/release/include/sharpsvn-imports.h"
[assembly:AssemblyVersionAttribute("1.20000.*")]; 

#pragma comment(lib, "git2.lib")
#pragma comment(lib, "http_parser.lib")
#ifdef _DEBUG
#pragma comment(lib, "libexpatdMD.lib")
#pragma comment(lib, "pcred.lib")
#pragma comment(lib, "zlibd.lib")
#else
#pragma comment(lib, "libexpatMD.lib")
#pragma comment(lib, "pcre.lib")
#pragma comment(lib, "zlib.lib")
#endif
#pragma comment(lib, "libssh2.lib")
#pragma comment(lib, "libcrypto.lib")



#if (APR_MAJOR_VERSION == 0)
#  pragma comment(lib, "apr.lib")
#  pragma comment(lib, "aprutil.lib")
#elif (APR_MAJOR_VERSION == 1)
#  pragma comment(lib, "apr-1.lib")
#  pragma comment(lib, "aprutil-1.lib")
#elif (APR_MAJOR_VERSION == 2)
#  pragma comment(lib, "apr-2.lib")
#  pragma comment(lib, "aprutil-2.lib")
#else
#  error Only apr 0.9.* and 1.* are supported at this time
#endif

