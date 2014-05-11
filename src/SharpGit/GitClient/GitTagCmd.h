#pragma once
#include "GitClientContext.h"
#include "GitCommitCmd.h"

namespace SharpGit {

    public ref class GitTagArgs : public GitCreateRefArgs
    {
        bool _overwrite;
        bool _noNormalize;
        bool _stripComments;
        bool _lightWeight;

    public:
        GitTagArgs()
        {
        }

        property bool OverwriteExisting
        {
            bool get()
            {
                return _overwrite;
            }
            void set(bool value)
            {
                _overwrite = value;
            }
        }

        property bool LightWeight
        {
            bool get()
            {
                return _lightWeight;
            }
            void set(bool value)
            {
                _lightWeight = value;
            }
        }
    };
}
