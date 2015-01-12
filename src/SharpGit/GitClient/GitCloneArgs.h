#pragma once
#include "GitCheckOutArgs.h"

namespace SharpGit {

    ref class GitInitArgs;

    public ref class GitCloneArgs : public GitCheckOutArgsWithSignature
    {
        String ^_branchName;
        bool _asynchronous;
        GitInitArgs ^_initArgs;

    internal:
        const git_clone_options * MakeCloneOptions(const git_remote_callbacks *cb, GitPool ^pool);

    public:
        GitCloneArgs();

        property GitInitArgs ^InitArgs
        {
            GitInitArgs ^get()
            {
                return _initArgs;
            }
        }

        property String ^ BranchName
        {
            String ^ get()
            {
                return _branchName;
            }

            void set(String ^value)
            {
                _branchName = value;
            }
        }

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

}
