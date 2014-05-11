#pragma once

#include "GitClientContext.h"

namespace SharpGit {

    public ref class GitCommitArgs : public GitCreateRefArgs
    {
        String ^_updateReference;
        initonly GitSignature ^_author;
        bool _noNormalize;
        bool _stripComments;

    public:
        GitCommitArgs()
        {
            UpdateReference = "HEAD";
            _author = gcnew GitSignature();

            // By default use the exact same timestamp for both
            _author->When = Signature->When;
        }

    public:
        property String ^ UpdateReference
        {
            String^ get()
            {
                return _updateReference;
            }
            void set(String ^value)
            {
                _updateReference = value;
            }
        }

        property GitSignature^ Author
        {
            GitSignature^ get()
            {
                return _author;
            }
        }
    };
}
