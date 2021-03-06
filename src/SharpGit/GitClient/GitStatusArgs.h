#pragma once

namespace SharpGit {
    using System::ComponentModel::DefaultValueAttribute;
    namespace Implementation {
        ref class GitPool;
    };

    public ref class GitStatusArgs : public GitClientArgs
    {
        bool _noWorkDir;
        bool _noIndex;
        bool _noUnversioned;
        bool _showIgnored;
        bool _showUnmodified;
        bool _noDirs;
        bool _noSubmodules;
        bool _caseInsensitive;
        bool _noRefresh;
        bool _updateCache;
        bool _noIncludeUnreadable;
        bool _findRenames;
        bool _noCollapseReplacements;

        bool _includeVersionedDirs;
        bool _globPath;

    public:
        DECLARE_EVENT(GitStatusEventArgs^, Status)

    public:
        [DefaultValue(true)]
        property bool WorkingDirectoryStatus
        {
            bool get()
            {
                return !_noWorkDir;
            }
            void set(bool value)
            {
                _noWorkDir = !value;
            }
        }

        property bool IndexStatus
        {
            bool get()
            {
                return !_noIndex;
            }
            void set(bool value)
            {
                _noIndex = !value;
            }
        }

        [DefaultValue(true)]
        property bool IncludeUnversioned
        {
            bool get()
            {
                return !_noUnversioned;
            }
            void set(bool value)
            {
                _noUnversioned = !value;
            }
        }

        [DefaultValue(false)]
        property bool IncludeIgnored
        {
            bool get()
            {
                return _showIgnored;
            }
            void set(bool value)
            {
                _showIgnored = value;
            }
        }

        [DefaultValue(false)]
        property bool IncludeUnmodified
        {
            bool get()
            {
                return _showUnmodified;
            }
            void set(bool value)
            {
                _showUnmodified = value;
            }
        }

        [DefaultValue(true)]
        property bool IncludeSubmodules
        {
            bool get()
            {
                return !_noSubmodules;
            }
            void set(bool value)
            {
                _noSubmodules = !value;
            }
        }

        [DefaultValue(true)]
        property bool IncludeDirectories
        {
            bool get()
            {
                return !_noDirs;
            }
            void set(bool value)
            {
                _noDirs = !value;
            }
        }

        property bool SortCaseInsensitive
        {
            bool get()
            {
                return _caseInsensitive;
            }
            void set(bool value)
            {
                _caseInsensitive = value;
            }
        }

        [DefaultValue(false)]
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

        [DefaultValue(true)]
        property bool NoCacheUpdate
        {
            bool get()
            {
                return !_updateCache;
            }
            void set(bool value)
            {
                _updateCache = !value;
            }
        }

        [DefaultValue(true)]
        property bool IncludeUnreadable
        {
            bool get()
            {
                return !_noIncludeUnreadable;
            }
            void set(bool value)
            {
                _noIncludeUnreadable = !value;
            }
        }

        [DefaultValue(false)]
        property bool FindRenames
        {
            bool get()
            {
                return _findRenames;
            }
            void set(bool value)
            {
                _findRenames = value;
            }
        }

        [DefaultValue(true)]
        property bool CollapseReplacements
        {
            bool get()
            {
                return !_noCollapseReplacements;
            }
            void set(bool value)
            {
                _noCollapseReplacements = value;
            }
        }

    public:
        /// <summary>Generates reports for versioned directories</summary>
        [DefaultValue(false)]
        property bool GenerateVersionedDirs
        {
            bool get()
            {
                return _includeVersionedDirs;
            }
            void set(bool value)
            {
                _includeVersionedDirs = value;
            }
        }

        [DefaultValue(false)]
        property bool UseGlobPath
        {
            bool get()
            {
                return _globPath;
            }
            void set(bool value)
            {
                _globPath = value;
            }
        }


    protected public:
        virtual void OnStatus(GitStatusEventArgs ^e)
        {
            Status(this, e);
        }

    internal:
        const git_status_options *MakeOptions(String^ path, Implementation::GitPool ^pool);
    };
}
