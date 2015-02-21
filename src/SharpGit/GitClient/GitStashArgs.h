#pragma once
#include "GitClientContext.h"
#include "GitCommitArgs.h"

namespace SharpGit {

    public ref class GitStashArgs : public GitCreateRefArgs
    {
        bool _keepChanges;
        bool _includeUntracked;
        bool _includeIgnored;
    public:
        GitStashArgs()
        {
        }

    public:
        property bool KeepChanges
        {
            bool get()
            {
                return _keepChanges;
            }
            void set(bool value)
            {
                _keepChanges = value;
            }
        }

        property bool IncludeUntracked
        {
            bool get()
            {
                return _includeUntracked;
            }
            void set(bool value)
            {
                _includeUntracked = value;
            }
        }

        property bool IncludeIgnored
        {
            bool get()
            {
                return _includeIgnored;
            }
            void set(bool value)
            {
                _includeIgnored = value;
            }
        }
    };
}
