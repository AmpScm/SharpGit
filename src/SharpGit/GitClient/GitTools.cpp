// Copyright 2007-2008 The SharpGit Project
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

#include <malloc.h>

#include "GitTools.h"
#include "GitClient.h"

#pragma warning(disable: 6255) // warning C6255: _alloca indicates failure by raising a stack overflow exception. Consider using _malloca instead

#ifdef GetEnvironmentVariable
#undef GetEnvironmentVariable
#endif

using namespace SharpGit;
using namespace SharpGit::Implementation;
using System::Text::StringBuilder;
using namespace System::IO;

static String^ StripLongPrefix(String ^path)
{
    if (path->StartsWith("\\\\?\\", StringComparison::Ordinal))
    {
        if (path->StartsWith("\\\\?\\UNC\\", StringComparison::Ordinal))
            path = L'\\' + path->Substring(7);
        else
            path = path->Substring(4);
    }

    return path;
}

static String^ LongGetFullPath(String^ path)
{
    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");

    path = StripLongPrefix(path);

    pin_ptr<const wchar_t> pPath = PtrToStringChars(path);
    wchar_t rPath[1024];
    wchar_t *pPathBuf;

    ZeroMemory(rPath, sizeof(rPath));
    const int sz = (sizeof(rPath) / sizeof(rPath[0]))-1;

    unsigned c = GetFullPathNameW((LPCWSTR)pPath, sz, rPath, nullptr);

    if (c == 0)
        throw gcnew PathTooLongException("GetFullPath for long paths failed");
    else if (c > sz)
    {
        pPathBuf = (wchar_t*)_alloca(sizeof(wchar_t) * (sz + 1));
        c = GetFullPathNameW((LPCWSTR)pPath, sz, pPathBuf, nullptr);
    }
    else
        pPathBuf = rPath;

    path = gcnew String(pPathBuf, 0, c);

    return StripLongPrefix(path);
}

/*static bool ContainsRelative(String^ path)
{
    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");

    int nxt = 0;
    int n;

    while (0 <= (n = path->IndexOf("\\.", nxt)))
    {
        nxt+=2;

        if (nxt >= path->Length || !Char::IsLetterOrDigit(path[nxt]))
            return true;
    }

    return false;
}*/

static String^ GetPathRootPart(String^ path)
{
    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");

    if (path->Length >= 3 && path[1] == ':' && path[2] == '\\')
    {
        if (path[0] >= 'a' && path[0] <= 'z')
            return Char::ToUpperInvariant(path[0]) + ":\\";
        else
            return path->Substring(0, 3);
    }

    if (!path->StartsWith("\\\\"))
        return nullptr;

    int nSlash = path->IndexOf('\\', 2);

    if (nSlash <= 2)
        return nullptr; // No hostname

    int nEnd = path->IndexOf('\\', nSlash+1);

    if (nEnd < 0)
        nEnd = path->Length;

    return "\\\\" + path->Substring(2, nSlash-2)->ToLowerInvariant() + path->Substring(nSlash, nEnd-nSlash);
}

static inline bool IsSeparator(String^ v, int index)
{
    wchar_t c = v[index];

    return (c == '\\') || (c == '/');
}

static inline bool IsDirSeparator(String^ v, int index)
{
    return (v[index] == '\\');
}

static bool IsInvalid(String^ v, int index)
{
    unsigned short c = v[index]; // .Net handles index checking
    array<char>^ invalidChars = GitBase::InvalidCharMap;

    if (c < invalidChars->Length)
        return (0 != invalidChars[c]);

    return false;
}

array<char>^ GitBase::InvalidCharMap::get()
{
    if (!_invalidCharMap)
        GenerateInvalidCharMap();

    return _invalidCharMap;
}

void GitBase::GenerateInvalidCharMap()
{
    List<char> ^invalid = gcnew List<char>(128); // Typical required: 124

    for each (Char c in Path::GetInvalidPathChars())
    {
        unsigned short cs = c;

        while (cs >= invalid->Count)
            invalid->Add(0);

        invalid[cs] = 1;
    }
    _invalidCharMap = invalid->ToArray();
}

bool GitBase::PathContainsInvalidChars(String^ path)
{
    array<char>^ invalidChars = InvalidCharMap;

    for each (Char c in path)
    {
        unsigned short cs = c;

        if (cs < invalidChars->Length
            && invalidChars[cs] != 0)
        {
            return false;
        }
    }

    return false;
}

String^ GitTools::GetTruePath(String^ path)
{
    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");

    // Keep original behavior as default
    return GetTruePath(path, false);
}

String^ GitTools::GetTruePath(String^ path, bool bestEffort)
{
    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");

    path = GetNormalizedFullPath(path);
    String^ root = GetPathRootPart(path);

    return FindTruePath(path, root, bestEffort);
}


String^ GitTools::FindTruePath(String^ path, String^ root, bool bestEffort)
{
    // Okay, now we have a normalized path and it's root in normal form. Now we need to find the exact casing of the next parts
    StringBuilder^ result = gcnew StringBuilder(root, path->Length + root->Length + 4);

    pin_ptr<const wchar_t> pChars = PtrToStringChars(path);
    size_t len = (path->Length+1+(4+4)); // 4+4 is long + UNC prefix
    wchar_t* pSec = (wchar_t*)_malloca(len * sizeof(wchar_t));
    int nMax;
    int nStart = 0;

    wchar_t *pTxt;
    if (!path->StartsWith("\\\\", StringComparison::OrdinalIgnoreCase))
    {
        if (wcscpy_s(pSec, len, L"\\\\?\\"))
            return nullptr;
        if (wcscat_s(pSec, len, pChars))
            return nullptr;

        pTxt = &pSec[4]; // strlen(@"\\?\") = 4
        nMax = path->Length;
        nStart = root->Length;
    }
    else
    {
        if (wcscpy_s(pSec, len, L"\\\\?\\UNC"))
            return nullptr;
        if (wcscat_s(pSec, len, pChars+1))
            return nullptr;

        pTxt = &pSec[root->Length + 6 + 1]; // strlen('?\UNC\')=6 + 1 for '\'
        nMax = path->Length - root->Length - 1;

        if (nMax <= 0) // "\\PC\C$" -> nMax = -1
            return result->ToString();
        result->Append(L'\\');
    }

    bool isFirst = true;

    //assert(wcslen(pTxt) == nMax);

    while (nStart < nMax)
    {
        WIN32_FIND_DATAW filedata;

        wchar_t *pNext = wcschr(&pTxt[nStart], '\\');

        if (pNext)
        {
            nStart = (int)(((INT_PTR)pNext - (INT_PTR)pTxt) / sizeof(wchar_t))+1;
            *pNext = 0; // Temporarily replace '\' with 0
        }

        HANDLE hSearch = FindFirstFileW(pSec, &filedata);

        if (hSearch == INVALID_HANDLE_VALUE)
        {
            if (!bestEffort)
                return nullptr;

            if (!isFirst)
                result->Append(L'\\');

            const wchar_t *pFileName = wcsrchr(pSec, L'\\');

            if (!pFileName)
            {
                // Should be impossible to reach for valid paths as we
                // only search for paths, not for roots
                return nullptr;
            }

            result->Append(gcnew String(pFileName+1));

            if (pNext)
            {
                *pNext = L'\\';
                result->Append(gcnew String(pNext));
            }

            return result->ToString();
        }

        if (!isFirst)
            result->Append(L'\\');

        result->Append(gcnew String(filedata.cFileName));

        ::FindClose(hSearch); // Close search request

        if (!pNext)
            break;
        else
            *pNext = '\\'; // Revert 0 to '\'

        isFirst= false;
    }

    return result->ToString();
}

// Optimized version of Directory::Exists() with several security checks removed
inline bool IsDirectory(String^ path)
{
    if (path->Length >= (MAX_PATH-4))
        path = "\\\\?\\" + GitTools::GetNormalizedFullPath(path);

    pin_ptr<const wchar_t> p = PtrToStringChars(path);
    DWORD r = ::GetFileAttributesW(p);

    if (r == INVALID_FILE_ATTRIBUTES)
        return false;

    return (r & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool GitTools::IsManagedPath(String^ path)
{
    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");

    if (IsSeparator(path, path->Length-1))
        path += GitClient::AdministrativeDirectoryName;
    else
        path += "\\" + GitClient::AdministrativeDirectoryName;

    return IsDirectory(path->Replace('/', '\\'));
}

bool GitTools::IsBelowManagedPath(String^ path)
{
    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");

    // We search from the root, instead of the other way around to optimize the disk cache
    // (We probably never need to access the disk when called a few times in a row)

    path = GetNormalizedFullPath(path);

    String^ root = GetPathRootPart(path);

    if (!root)
        return false;

    String^ admDir = GitClient::AdministrativeDirectoryName;
    int nStart = root->Length;
    int nEnd = path->Length - 1;
    int i;

    while (nStart <= (i = path->LastIndexOf('\\', nEnd)))
    {
        int len = nEnd - i;

        if (len == admDir->Length &&
            (0 == String::Compare(path, i+1, admDir, 0, len, StringComparison::OrdinalIgnoreCase)))
        {
            // The .git directory can't contain a working copy..

            nStart = nEnd+2; // start looking one character after the '\' if there is one.

            if (nStart >= path->Length)
                return false;

            break;
        }

        nEnd = i - 1;
    }

    while (0 <= (i = path->IndexOf('\\', nStart)))
    {
        if (IsManagedPath(path->Substring(0, i)))
            return true;

        nStart = i+1;
    }

    if (nStart >= path->Length)
        return false;
    else
        return IsManagedPath(path);
}

String^ GitTools::GetNormalizedFullPath(String^ path)
{
    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");

    if (path[0] == L'\\')
        path = StripLongPrefix(path);

    if (IsNormalizedFullPath(path))
        return path; // Just pass through; no allocations
    else if (PathContainsInvalidChars(path) || path->LastIndexOf(':') >= 2)
        throw gcnew ArgumentException(String::Format("Path '{0}' contains invalid characters.", path), "path");

    bool retry = true;

    if(path->Length < MAX_PATH)
    {
        try
        {
            path = Path::GetFullPath(path);
            retry = false;
        }
        catch(PathTooLongException^) // Path grew by getting full path
        {
            // Use the retry
        }
        catch(NotSupportedException^) // Something fishy is going on
        {
            // Use the retry
        }
    }

    if (retry)
    {
        path = LongGetFullPath(path);

        if (!GetPathRootPart(path))
            throw gcnew PathTooLongException(String::Format("Paths with a length above MAX_PATH (like '{0}') must be rooted.", path));
    }

    if (path->Length >= 2 && path[1] == ':')
    {
        wchar_t c = path[0];

        if ((c >= 'a') && (c <= 'z'))
            path = Char::ToUpperInvariant(c) + path->Substring(1);

        String^ r = path->TrimEnd('\\');

        if (r->Length > 3)
            return r;
        else
            return path->Substring(0, 3);
    }
    else if (path->StartsWith("\\\\", StringComparison::OrdinalIgnoreCase))
    {
        String^ root = GetPathRootPart(path);

        if (root && !path->StartsWith(root, StringComparison::Ordinal))
            path = root + path->Substring(root->Length)->TrimEnd('\\');
        else
            path = path->TrimEnd('\\');
    }
    else
        path = path->TrimEnd('\\');

    return path;
}

bool GitTools::IsAbsolutePath(String^ path)
{
    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");

    if (path->Length < 3)
        return false;

    int i, n;
    if (IsSeparator(path, 0))
    {
        if (!IsSeparator(path, 1))
            return false;

        for (i = 2; i < path->Length; i++)
        {
            if (!Char::IsLetterOrDigit(path, i) && 0 > _hostChars->IndexOf(path[i]))
                break;
        }

        if (i == 2 || i == path->Length || !IsSeparator(path, i))
            return false;

        i++;

        n = i;

        for (; i < path->Length; i++)
        {
            if (!Char::IsLetterOrDigit(path, i) && 0 > _shareChars->IndexOf(path[i]))
                break;
        }

        if (i == path->Length)
            return (i != n);
        else if (i == n || !IsSeparator(path, i))
            return false;

        i++;

        if (i == path->Length)
            return false; // "\\server\share\"
    }
    else if ((path[1] != ':') || !IsSeparator(path, 2))
        return false;
    else if (!(((path[0] >= 'A') && (path[0] <= 'Z')) || ((path[0] >= 'a') && (path[0] <= 'z'))))
        return false;
    else
        i = 3;

    while (i < path->Length)
    {
        if (IsSeparator(path, i))
            return false; // '\'-s behind each other

        if (path[i] == '.')
        {
            int j = i;
            j++;

            if (j < path->Length && path[j] == '.')
                j++;

            if (j >= path->Length || IsSeparator(path, j))
                return false; // '\'-s behind each other
        }

        n = i;

        while (i < path->Length && !IsInvalid(path, i) && !IsSeparator(path, i))
            i++;

        if (n == i)
            return false;
        else if (i == path->Length)
            return true;
        else if (!IsSeparator(path, i++))
            return false;

        if (i == path->Length)
            return false; // We don't like paths with a '\' at the end
    }

    return true;
}

bool GitTools::IsNormalizedFullPath(String^ path)
{
    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");

    int c = path->Length;

    if (path->Length < 3)
        return false;

    int i, n;
    if (IsDirSeparator(path, 0))
    {
        if (!IsDirSeparator(path, 1))
            return false;

        for (i = 2; i < path->Length; i++)
        {
            wchar_t cc = path[i];
            // Check hostname rules

            if (! ((cc >= 'a' && cc <= 'z') ||
                       (cc >= '0' && cc <= '9') ||
                       0 <= _hostChars->IndexOf(cc)))
                break;
        }

        if (i == 2 || !IsDirSeparator(path, i))
            return false;

        i++;

        n = i;

        for (; i < path->Length; i++)
        {
            // Check share name rules
            if (!Char::IsLetterOrDigit(path, i) && (0 > _shareChars->IndexOf(path[i])))
                break;
        }

        if (i == n)
            return false; // "\\server\"
        else if (i == c)
            return true; // "\\server\path"
        else if (c > i && !IsDirSeparator(path, i))
            return false;
        else if (c == i+1)
            return false; // "\\server\path\"

        i++;
    }
    else if ((path[1] != ':') || !IsDirSeparator(path, 2))
        return false;
    else if (!((path[0] >= 'A') && (path[0] <= 'Z')))
        return false;
    else
        i = 3;

    array<char>^ invalidMap = InvalidCharMap;

    while (i < c)
    {
        if (i >= c && IsDirSeparator(path, i))
            return false; // '\'-s behind each other

        if (i < c && path[i] == '.')
        {
            int j = i;

            while (j < c && path[j] == '.')
                j++;

            if (j < path->Length && IsSeparator(path, j) || j >= c)
                return false; // Relative path
        }

        n = i;

        while (i < c)
        {
            unsigned short cc = path[i];

            if (cc < invalidMap->Length && 0 != invalidMap[cc])
                return false;
            else if (cc == '\\' || cc == '/' || cc == ':')
                break;

            i++;
        }

        if (n == i)
            return false;
        else if (i == c)
            return true;
        else if (!IsDirSeparator(path, i++))
            return false;

        if (i == c)
            return false; // We don't like paths with a '\' at the end
    }

    return true;
}

String^ GitTools::PathCombine(String^ path1, String^ path2)
{
    if (!path1)
        throw gcnew ArgumentNullException("path1");
    else if (!path2)
        throw gcnew ArgumentNullException("path2");

    try
    {
        return Path::Combine(path1, path2);
    }
    catch (PathTooLongException^)
    {
        if (GetPathRootPart(path2))
        {
            // Handle large absolute paths in path2, including UNC paths
            return GitTools::GetNormalizedFullPath(path2);
        }

        // The next code is fall back code that is only really supported
        // for appending simple relative filenames after a parent path

        // This all to avoid exceptions when creating repository local paths that
        // trip the MAX_PATH limit

        path1 = GitTools::GetNormalizedFullPath(path1);

        if (IsSeparator(path2, 0)) // "\d\e\f"
        {
            // BH: This is what Path::Combine does in .Net 2.0!!!

            String^ root = GetPathRootPart(Environment::CurrentDirectory);

            if(root)
                path1 = root;
        }

        if (!IsSeparator(path1, path1->Length-1))
            path1 += '\\';

        return GitTools::GetNormalizedFullPath(path1 +
            path2->Replace('/', '\\')->TrimStart('\\'));
    }
}

String^ GitTools::GetNormalizedDirectoryName(String^ path)
{
    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");

    path = GetNormalizedFullPath(path);

    String^ root = GetPathRootPart(path);
    if (!root)
        return nullptr;

    int nLs = path->LastIndexOf('\\');

    if (nLs > root->Length)
        return path->Substring(0, nLs);
    else if (nLs == root->Length || (nLs < root->Length && path->Length > root->Length))
        return root;
    else
        return nullptr;
}

String^ GitTools::GetPathRoot(String^ path)
{
    if (String::IsNullOrEmpty(path))
                throw gcnew ArgumentNullException("path");

    return GetPathRootPart(path);
}
