#include "stdafx.h"

#include "GitRepository.h"
#include "GitRemote.h"

#include "UnmanagedStructs.h"

using namespace SharpGit;
using namespace SharpGit::Implementation;
using namespace SharpGit::Plumbing;
using System::Collections::Generic::List;

GitRemoteCollection^ GitRepository::Remotes::get()
{
    if (!_remotes)
    {
        if (IsDisposed)
              throw gcnew ObjectDisposedException("repository");

        _remotes = gcnew GitRemoteCollection(this);
    }

    return _remotes;
}

System::Collections::Generic::IEnumerator<GitRemote^>^ GitRemoteCollection::GetEnumerator()
{
    throw gcnew NotImplementedException();
}
