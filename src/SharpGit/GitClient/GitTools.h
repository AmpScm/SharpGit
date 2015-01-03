#pragma once

#include "GitClientContext.h"

namespace SharpGit {
    using namespace SharpGit::Implementation;
    using namespace System::Collections::Generic;
    using System::String;

    public ref class GitTools sealed : Implementation::GitBase
    {
    private:
        GitTools()
        {} // Static class

        literal String^ _hostChars = "._-";
        literal String^ _shareChars = "._-$ ";

        static String^ FindTruePath(String^ path, String^ root, bool bestEffort);

    public:
        /// <summary>Gets the absolute pathname exactly like it is on disk (fixing casing); returns NULL for non existing paths</summary>
        static String^ GetTruePath(String^ path);

        /// <summary>Gets the absolute pathname exactly like it is on disk (fixing casing).
        /// For not existing paths, if bestEffort is TRUE, returns a path based on existing parents. Otherwise return NULL for not existing paths</summary>
        static String^ GetTruePath(String^ path, bool bestEffort);

        /// <summary>Gets a boolean indicating whether the path could contain a Subversion Working Copy</summary>
        /// <remarks>Assumes path is a directory</remarks>
        static bool IsManagedPath(String^ path);

        /// <summary>Gets a boolean indicating whether at least one of the
        /// parent paths or the path (file/directory) itself is a working copy.
        /// </summary>
        static bool IsBelowManagedPath(String^ path);

        /// <summary>
        /// Normalizes the path to a full path
        /// </summary>
        /// <summary>This normalizes drive letters to upper case and hostnames to lowercase to match Subversion 1.6 behavior</summary>
        static String^ GetNormalizedFullPath(String^ path);

        /// <summary>
        /// Checks whether normalization is required
        /// </summary>
        /// <remarks>This method does not verify the casing of invariant parts</remarks>
        static bool IsNormalizedFullPath(String^ path);

        /// <summary>
        /// Checks whether the specified path is an absolute path that doesn't end in an unneeded '\'
        /// </summary>
        static bool IsAbsolutePath(String^ path);

    public:

        /// <summary>Gets the normalized directory name of path (Long path enabled version of <see cref="System::IO::Path::GetDirectoryName" />, always returning full paths)</summary>
        /// <returns>Directory information for path, or null if path denotes a root directory or is null. Returns String.Empty if path does not contain directory information.</returns>
        static String^ GetNormalizedDirectoryName(String^ path);

        static String^ GetPathRoot(String^ path);

    internal:
        /// <summary>Long path capable version of <see cref="System::IO::Path::Combine(String^, String^)" /></summary>
        static String^ PathCombine(String^ path1, String^ path2);
    };
}
