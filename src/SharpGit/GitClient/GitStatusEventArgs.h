#pragma once

namespace SharpGit {

    ref class GitStatusArgs;

    public enum class GitStatus
    {
        None,
        Normal,
        New,
        Deleted,
        Renamed,
        TypeChanged,

        Unreadable              = 0x10000
    };


    [FlagsAttribute]
    enum class GitConflicted
    {
        None,
        HasAncestor = 1,
        HasMine     = 2,
        HasTheirs   = 3,
    };

    [Flags]
    enum class GitStatusHas
    {
        None = 0,
        HeadToIndex       = 0x0001,
        IndexToWorking    = 0x0002,

        File        = 0x0010,
        Directory   = 0x0020,
        Dummy       = 0x0040,
    };

    public ref class GitStatusEventArgs : public GitClientEventArgs
    {
        initonly unsigned _status;
        initonly GitStatusHas _has;
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
                    _relPath = GitBase::Utf8_PtrToString(_path);

                return _relPath;
            }
        }

        property String^ FullPath
        {
            String^ get()
            {
                if (!_fullPath && _pool && _path && _wcPath)
                    _fullPath = GitBase::StringFromDirent(svn_dirent_join(_wcPath, _path, _pool->Handle), _pool);

                return _fullPath;
            }
        }

        property GitNodeKind NodeKind
        {
            GitNodeKind get()
            {
                if ((int)(_has & GitStatusHas::File))
                    return GitNodeKind::File;
                else if ((int)(_has & GitStatusHas::Directory))
                    return GitNodeKind::Directory;

                return GitNodeKind::Unknown;
            }
        }

    public:
        property GitStatus IndexStatus
        {
            GitStatus get()
            {
                if (! (int)(_has & GitStatusHas::HeadToIndex))
                    return GitStatus::None;

                switch (_status & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED | GIT_STATUS_INDEX_TYPECHANGE))
                {
                case GIT_STATUS_INDEX_NEW:
                    return GitStatus::New;
                case GIT_STATUS_INDEX_DELETED:
                    return GitStatus::Deleted;
                case GIT_STATUS_INDEX_RENAMED:
                    return GitStatus::Renamed;
                case GIT_STATUS_INDEX_TYPECHANGE:
                    return GitStatus::TypeChanged;
                default:
                    return GitStatus::Normal;
                }
            }
        }

        property bool IndexModified
        {
            bool get()
            {
                return (int)(_has & GitStatusHas::HeadToIndex) && (_status & GIT_STATUS_INDEX_MODIFIED);
            }
        }

        property GitStatus WorkingDirectoryStatus
        {
            GitStatus get()
            {
                if (! (int)(_has & GitStatusHas::IndexToWorking))
                    return GitStatus::None;

                switch (_status & (GIT_STATUS_WT_NEW | GIT_STATUS_WT_DELETED | GIT_STATUS_WT_TYPECHANGE | GIT_STATUS_WT_RENAMED | GIT_STATUS_WT_UNREADABLE))
                {
                case GIT_STATUS_WT_NEW:
                    return GitStatus::New;
                case GIT_STATUS_WT_DELETED:
                    return GitStatus::Deleted;
                case GIT_STATUS_WT_TYPECHANGE:
                    return GitStatus::Deleted;
                case GIT_STATUS_WT_RENAMED:
                    return GitStatus::Renamed;
                case GIT_STATUS_WT_UNREADABLE:
                    return GitStatus::Unreadable;
                default:
                    return GitStatus::Normal;
                }
            }
        }

        property bool WorkingDirectoryModified
        {
            bool get()
            {
                return (int)(_has & GitStatusHas::IndexToWorking) && (_status & GIT_STATUS_WT_MODIFIED);
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
