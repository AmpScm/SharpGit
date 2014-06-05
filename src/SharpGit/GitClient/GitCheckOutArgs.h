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

    [Flags]
    public enum class GitCheckoutNotify
    {
        None = GIT_CHECKOUT_NOTIFY_NONE,
        Conflict = GIT_CHECKOUT_NOTIFY_CONFLICT,
        Dirty = GIT_CHECKOUT_NOTIFY_DIRTY,
        Updated = GIT_CHECKOUT_NOTIFY_UPDATED,
        Untracked = GIT_CHECKOUT_NOTIFY_UNTRACKED,
        Ignored =GIT_CHECKOUT_NOTIFY_IGNORED,
    };

    public ref class GitCheckOutNotifyEventArgs : public GitEventArgs
    {
        const char *_pPath;
        String ^_relPath;
        GitCheckoutNotify _notify;

    internal:
        GitCheckOutNotifyEventArgs(git_checkout_notify_t why, const char *path, const git_diff_file *baseline, const git_diff_file *target, const git_diff_file *workdir, GitPool ^pool)
        {
            _pPath = path;
            _notify = (GitCheckoutNotify)(int)why;
        }

    public:
        property String ^ RelativePath
        {
            String ^ get()
            {
                if (!_relPath && _pPath)
                    _relPath = GitBase::Utf8_PtrToString(_pPath);

                return _relPath;
            }
        }

        property GitCheckoutNotify Notify
        {
            GitCheckoutNotify get()
            {
                return _notify;
            }
        }

    protected public:
        virtual void Detach(bool keepValues) override
        {
            try
            {
                if (keepValues)
                {
                    GC::KeepAlive(RelativePath);
                }
            }
            finally
            {
                _pPath = nullptr;

                __super::Detach(keepValues);
            }
        }
    };

    public ref class GitCheckOutProgressEventArgs : public GitEventArgs
    {
        const char *_pPath;
        String ^_relPath;
        initonly __int64 _completedSteps;
        initonly __int64 _totalSteps;

    internal:
        GitCheckOutProgressEventArgs(const char *path, __int64 completed_steps, __int64 total_steps)
        {
            _pPath = path;
            _completedSteps = completed_steps;
            _totalSteps = total_steps;
        }

    public:
        property String ^ RelativePath
        {
            String ^ get()
            {
                if (!_relPath && _pPath)
                    _relPath = GitBase::Utf8_PtrToString(_pPath);

                return _relPath;
            }
        }

        property __int64 CompletedSteps
        {
            __int64 get()
            {
                return _completedSteps;
            }
        }

        property __int64 TotalSteps
        {
            __int64 get()
            {
                return _totalSteps;
            }
        }

    protected public:
        virtual void Detach(bool keepValues) override
        {
            try
            {
                if (keepValues)
                {
                    GC::KeepAlive(RelativePath);
                }
            }
            finally
            {
                _pPath = nullptr;

                __super::Detach(keepValues);
            }
        }
    };

    /// <summary>
    /// Enum specifying what content checkout should write to disk
    /// for conflicts.
    /// </summary>
    public enum class GitCheckOutConflictStrategy
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
        git_checkout_options * AllocCheckOutOptions(GitPool ^pool);

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
        GitCheckOutConflictStrategy _conflictStrategy;

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

        property GitCheckOutConflictStrategy ConflictStrategy
        {
            GitCheckOutConflictStrategy get()
            {
                return _conflictStrategy;
            }
            void set(GitCheckOutConflictStrategy value)
            {
                _conflictStrategy = value;
            }
        }

    public:
        DECLARE_EVENT(GitCheckOutNotifyEventArgs ^, CheckOutNotify)
        DECLARE_EVENT(GitCheckOutProgressEventArgs ^, CheckOutProgress)

    protected:
        virtual void OnCheckOutNotify(GitCheckOutNotifyEventArgs ^e)
        {
            CheckOutNotify(this, e);
        }

        virtual void OnCheckOutProgress(GitCheckOutProgressEventArgs ^e)
        {
            CheckOutProgress(this, e);
        }

    internal:
        void InvokeCheckOutNotify(GitCheckOutNotifyEventArgs ^e)
        {
            OnCheckOutNotify(e);
        }

        void InvokeCheckOutProgress(GitCheckOutProgressEventArgs ^e)
        {
            OnCheckOutProgress(e);
        }
    };
};
