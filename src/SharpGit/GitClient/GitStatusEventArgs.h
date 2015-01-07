#pragma once

namespace SharpGit {

    ref class GitStatusArgs;

    [FlagsAttribute]
    public enum class GitStatus
    {
        Normal = 0,

        New                     =     0x0001,
        Added                   =     0x0002,

        Modified                =     0x0010,
        Renamed                 =     0x0020,
        TypeChange              =     0x0040,

        Unreadable              = 0x00100000,
        Deleted                 = 0x01000000,

        Ignored                 = 0x10000000,
    };


    [FlagsAttribute]
    enum class GitConflicted
    {
        None,
        HasAncestor = 1,
        HasMine     = 2,
        HasTheirs   = 3,
    };

    public enum class GitNodeKind
    {
        None = 0,
        File = 1,
        Directory = 2,
        Unknown = 3,
        /*SymbolicLink = 4,*/
    };

    public ref class GitStatusEventArgs : public GitClientEventArgs
    {
        initonly unsigned _status;
        String^ _relPath;
        String^ _fullPath;
        bool _directory;
        GitPool^ _pool;
        const char *_path;
        const char *_wcPath;
        const git_status_entry *_entry;
        GitConflicted _conflicted;

    internal:
        GitStatusEventArgs(const char *path, const char *wcPath, bool directory, const git_status_entry *entry, const git_index_entry *conflict_stages[3], GitStatusArgs ^args, Implementation::GitPool ^pool);

    public:
        property String^ RelativePath
        {
            String^ get()
            {
                if (!_relPath && _path)
                {
                    int n = (int)strlen(_path);

                    if (n > 0 && _path[n-1] == '/')
                        n--;

                    _relPath = GitBase::Utf8_PtrToString(_path, n);
                }

                return _relPath;
            }
        }

        property String^ FullPath
        {
            String^ get()
            {
                if (!_fullPath && _pool && _path && _wcPath)
                {
                    _fullPath = GitBase::StringFromDirent(svn_dirent_join(_wcPath, _path, _pool->Handle), _pool);
                }

                return _fullPath;
            }
        }

        property GitNodeKind NodeKind
        {
            GitNodeKind get()
            {
                return _directory ? GitNodeKind::Directory : GitNodeKind::File;
            }
        }

    public:
        property GitStatus IndexStatus
        {
            GitStatus get()
            {
                switch (_status & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED | GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED | GIT_STATUS_INDEX_TYPECHANGE))
                {
                case 0:
                    return GitStatus::Normal;
                case GIT_STATUS_INDEX_NEW:
                    return GitStatus::Added;
                case GIT_STATUS_INDEX_MODIFIED:
                    return GitStatus::Modified;
                case GIT_STATUS_INDEX_DELETED:
                    return GitStatus::Deleted;
                default:
                    assert(false);
                    return GitStatus::Modified;
                }
            }
        }

        property GitStatus WorkingDirectoryStatus
        {
            GitStatus get()
            {
                switch (_status & (GIT_STATUS_WT_NEW | GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED | GIT_STATUS_WT_TYPECHANGE | GIT_STATUS_WT_RENAMED | GIT_STATUS_WT_UNREADABLE))
                {
                case 0:
                    return GitStatus::Normal;
                case GIT_STATUS_WT_NEW:
                    return GitStatus::New;
                case GIT_STATUS_WT_MODIFIED:
                    return GitStatus::Modified;
                case GIT_STATUS_WT_DELETED:
                    return GitStatus::Deleted;
                case GIT_STATUS_WT_UNREADABLE:
                    return GitStatus::Unreadable;
                default:
                    assert(false);
                    return GitStatus::Modified;
                }
            }
        }

        property bool Ignored
        {
            bool get()
            {
                return (_status & GIT_STATUS_IGNORED) != 0;
            }
        }

        property bool Conflicted
        {
            bool get()
            {
                return (_conflicted != GitConflicted::None);
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
                    GC::KeepAlive(FullPath);
                }
            }
            finally
            {
                _pool = nullptr;
                _path = nullptr;
                _wcPath = nullptr;
                _entry = nullptr;

                __super::Detach(keepValues);
            }
        }
    };
}
