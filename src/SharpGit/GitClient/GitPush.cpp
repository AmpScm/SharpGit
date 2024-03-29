#include "pch.h"
#include "GitClient.h"

#include "GitPushArgs.h"

#include "Plumbing/GitRepository.h"
#include "Plumbing/GitRefSpec.h"
#include "Plumbing/GitBranch.h"
#include "Plumbing/GitReference.h"
#include "Plumbing/GitRemote.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace SharpGit;
using namespace SharpGit::Plumbing;
using namespace SharpGit::Implementation;

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
    Dictionary<String^, GitRemote^> ^remotes = gcnew Dictionary<String^, GitRemote^>();
    Dictionary<String^, List<GitRefSpec^>^> ^refSpecs = gcnew Dictionary<String^, List<GitRefSpec^>^>();

    //if (!args->HasTarget)
    {
        for each (GitBranch ^branch in repo->Branches)
        {
            GitRemote ^rm;
            List<GitRefSpec^> ^rs;

            GitBranch ^tracked = branch->TrackedBranch;
            if (!tracked)
                continue;

            if (!remotes->ContainsKey(tracked->RemoteName))
            {
                if (repo->Remotes->TryGet(tracked->RemoteName, rm))
                {
                    remotes->Add(tracked->RemoteName, rm);
                    refSpecs->Add(tracked->RemoteName, rs = gcnew List<GitRefSpec^>());
                }
                else
                    throw gcnew InvalidOperationException();
            }
            else
                rs = refSpecs[tracked->RemoteName];

            rs->Add(branch->AsRefSpec());
        }
    }
    /* else: set from target */

    GitPool pool;

    for each (KeyValuePair<String^, GitRemote^>^ kv in remotes)
    {
        GitRemote ^rm = kv->Value;

        rm->Push(refSpecs[rm->Name], args, this);

        delete rm;
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
