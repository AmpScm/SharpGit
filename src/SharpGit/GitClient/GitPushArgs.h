#pragma once
#include "GitClientContext.h"

namespace SharpGit {

    public enum class GitPushMode
    {
        Default = 0,
        All = 1,
        Mirror = 2,
        Tags = 3
    };

    public ref class GitPushArgs : public GitCreateRefArgs
    {
        GitPushMode _mode;

    internal:
        const git_push_options * AllocOptions(GitPool ^pool);

    public:
        property GitPushMode Mode
        {
            GitPushMode get()
            {
                return _mode;
            }

            void set(GitPushMode value)
            {
                _mode = value;
            }
        }
    };

}