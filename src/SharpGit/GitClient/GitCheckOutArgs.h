#pragma once

#include "GitClientContext.h"

namespace SharpGit {

    public ref class GitClientRemoteArgs abstract : public GitClientArgs
    {
        bool _asynchronous;

    public:
        property bool Synchronous
        {
            bool get()
            {
                return !_asynchronous;
            }
            void set(bool value)
            {
                _asynchronous = !value;
            }
        }
    };

    /// <summary>
    /// Enum specifying what content checkout should write to disk
    /// for conflicts.
    /// </summary>
    public enum class GitCheckoutConflictStrategy
    {
        /// <summary>
        /// Use the default behavior for handling file conflicts. This is
        /// controlled by the merge.conflictstyle config option, and is "Merge"
        /// if no option is explicitly set.
        /// </summary>
        Default,

        /// <summary>
        /// For conflicting files, checkout the "ours" (stage 2)  version of
        /// the file from the index.
        /// </summary>
        UseOurs,

        /// <summary>
        /// For conflicting files, checkout the "theirs" (stage 3) version of
        /// the file from the index.
        /// </summary>
        UseTheirs,

        /// <summary>
        /// Write normal merge files for conflicts.
        /// </summary>
        Merge,

        /// <summary>
        /// Write diff3 formated files for conflicts.
        /// </summary>
        Diff3
    };

    public ref class GitCheckOutArgs : public GitClientArgs
    {
    internal:
        git_checkout_options * MakeCheckOutOptions(GitPool ^pool);

    private:
        bool _dryRun;
        bool _noCreateMissing;
        bool _overwrite;
        bool _failOnConflicts;
        bool _removeUnversioned;
        bool _removeIgnored;
        bool _updateOnly;
        bool _noUpdateCache;
        bool _noRefresh;
        GitCheckoutConflictStrategy _conflictStrategy;

    public:
        property bool DryRun
        {
            bool get()
            {
                return _dryRun;
            }
            void set(bool value)
            {
                _dryRun = value;
            }
        }

        /// <summary>If true, don't create missing files during checkout</summary>
        property bool SkipMissingFiles
        {
            bool get()
            {
                return _noCreateMissing;
            }
            void set(bool value)
            {
                _noCreateMissing = value;
            }
        }

        property bool OverwriteExisting
        {
            bool get()
            {
                return _overwrite;
            }
            void set(bool value)
            {
                _overwrite = value;
            }
        }

        property bool FailOnConflicts
        {
            bool get()
            {
                return _failOnConflicts;
            }
            void set(bool value)
            {
                _failOnConflicts = value;
            }
        }

        property bool RemoveUnversioned
        {
            bool get()
            {
                return _removeUnversioned;
            }
            void set(bool value)
            {
                _removeUnversioned = value;
            }
        }

        property bool RemoveIgnored
        {
            bool get()
            {
                return _removeIgnored;
            }
            void set(bool value)
            {
                _removeIgnored = value;
            }
        }

        property bool UpdateOnly
        {
            bool get()
            {
                return _updateOnly;
            }
            void set(bool value)
            {
                _updateOnly = value;
            }
        }

        property bool NoRefresh
        {
            bool get()
            {
                return _noRefresh;
            }
            void set(bool value)
            {
                _noRefresh = value;
            }
        }

        property bool NoCacheUpdate
        {
            bool get()
            {
                return _noUpdateCache;
            }
            void set(bool value)
            {
                _noUpdateCache = value;
            }
        }

        property GitCheckoutConflictStrategy ConflictStrategy
        {
            GitCheckoutConflictStrategy get()
            {
                return _conflictStrategy;
            }
            void set(GitCheckoutConflictStrategy value)
            {
                _conflictStrategy = value;
            }
        }
    };
};
