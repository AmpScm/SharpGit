#include "stdafx.h"
#include "GitClient.h"

#include "Args/GitPushArgs.h"

#include "Plumbing/GitRepository.h"
#include "Plumbing/GitRefSpec.h"
#include "Plumbing/GitRemote.h"

using namespace System;
using namespace SharpGit;
using namespace SharpGit::Plumbing;

bool GitClient::Push(String ^localRepository)
{
    return GitClient::Push(localRepository, gcnew GitPushArgs());
}

bool GitClient::Push(String ^localRepository, GitPushArgs ^args)
{
    GitRepository repo;

    if (!repo.Locate(localRepository, args))
        return false;

    switch(args->Mode)
    {
        case GitPushMode::All:
            return PushAll(%repo, args);


        case GitPushMode::Mirror:
            return PushMirror(%repo, args);

        case GitPushMode::Tags:
            return PushTags(%repo, args);

        default:
        case GitPushMode::Default:
            return PushDefault(%repo, args);
    }
}

bool GitClient::PushAll(GitRepository ^repo, GitPushArgs ^args)
{
    for each (GitRemote ^rm in repo->Remotes)
    {
        try
        {
            //rm->SetCallbacks(get_callbacks());
            //rm->Connect(true, args);
            //rm->Download(args);
            //rm->UpdateTips(args);
            //rm->Disconnect(args);
        }
        finally
        {
            delete rm;
        }
    }
    return true;
}

bool GitClient::PushMirror(GitRepository ^repo, GitPushArgs ^args)
{
    throw gcnew NotImplementedException();
}

bool GitClient::PushTags(GitRepository ^repo, GitPushArgs ^args)
{
    throw gcnew NotImplementedException();
}

bool GitClient::PushDefault(GitRepository ^repo, GitPushArgs ^args)
{
    throw gcnew NotImplementedException();
}
