#pragma once
#include "GitCheckOutArgs.h"

namespace SharpGit {

    public ref class GitCloneArgs : public GitCheckOutArgsWithSignature
    {
        bool _bare;
        String ^_branchName;
        bool _asynchronous;

    internal:
        const git_clone_options * MakeCloneOptions(const git_remote_callbacks *cb, GitPool ^pool);

    public:
        GitCloneArgs()
        {
        }

        property bool CreateBareRepository
        {
            bool get()
            {
                return _bare;
            }
            void set(bool value)
            {
                _bare = value;
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
