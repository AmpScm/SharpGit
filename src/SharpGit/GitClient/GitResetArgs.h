#pragma once
#include "GitClientContext.h"
#include "GitCheckOutArgs.h"

namespace SharpGit {

    public enum class GitResetMode
    {
        Default = 0,
        Mixed = 1,
        Hard = 2,
    };

    public ref class GitResetArgs : public GitCheckOutArgs
    {
        GitResetMode _mode;
        String^ _refLogMessage;
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

        property String ^ RefLogMessage
        {
            String ^get()
            {
                return _refLogMessage;
            }
            void set(String ^value)
            {
                _refLogMessage = value;
            }
        }
    };

}