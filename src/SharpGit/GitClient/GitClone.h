#pragma once
#include "GitCheckOut.h"

namespace SharpGit {

    public ref class GitCloneArgs : public GitCheckOutArgs
    {
        GitSignature ^_signature;
        bool _bare;
        bool _ignoreCertErrors;
        String ^_remoteName;
        String ^_branchName;

    internal:
        const git_clone_options * MakeCloneOptions(const git_remote_callbacks *cb, GitPool ^pool);

    public:
        GitCloneArgs()
        {
            _signature = gcnew GitSignature();
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

        property GitSignature ^ Signature
        {
            GitSignature ^ get()
            {
                return _signature;
            }
        }

        property bool IgnoreCertificateErrors
        {
            bool get()
            {
                return _ignoreCertErrors;
            }
            void set(bool value)
            {
                _ignoreCertErrors = value;
            }
        }

        property String ^ RemoteName
        {
            String ^ get()
            {
                return _remoteName;
            }

            void set(String ^value)
            {
                _remoteName = value;
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
    };

}
