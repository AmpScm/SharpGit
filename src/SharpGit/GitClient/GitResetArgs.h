#pragma once
#include "GitClientContext.h"

namespace SharpGit {

    public enum class GitResetMode
    {
        Default = 0,
        Mixed = 1,
        Hard = 2,
    };

    public ref class GitResetArgs : public GitCreateRefArgs
    {
        GitResetMode _mode;
    public:
        GitResetArgs()
        {}

        property GitResetMode Mode
        {
            GitResetMode get()
            {
                return _mode;
            }

            void set(GitResetMode value)
            {
                _mode = value;
            }
        }
    };

}