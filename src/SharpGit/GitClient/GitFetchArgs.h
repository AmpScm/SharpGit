#pragma once
#include "GitClientContext.h"

namespace SharpGit {

    public ref class GitFetchArgs : public GitCreateRefArgs
    {
        bool _all;

    public:
        property bool All
        {
            bool get()
            {
                return _all;
            }

            void set(bool value)
            {
                _all = value;
            }
        }
    };
}

