#include "stdafx.h"

#include "GitRepository.h"
#include "GitObject.h"
#include "GitBlob.h"
#include "GitCommit.h"
#include "GitReference.h"
#include "GitTag.h"
#include "GitTree.h"
#include "GitBranch.h"

#include "../GitClient/GitCommitCmd.h"
#include "../GitClient/GitTagCmd.h"

#include "UnmanagedStructs.h"

using namespace SharpGit;
using namespace SharpGit::Implementation;
using namespace SharpGit::Plumbing;
using System::Collections::Generic::List;

GitSignature^ GitTag::Tagger::get()
{
    if (!_tagger && !IsDisposed)
        _tagger = gcnew GitSignature(git_tag_tagger(Handle));

    return _tagger;
}

GitObject^ GitTag::Target::get()
{
    if (!_target && !IsDisposed)
    {
        git_object *ob;

        if (0 == git_tag_target(&ob, Handle))
            _target = GitObject::Create(Repository, ob);
    }
    return _target;
}

GitId^ GitTag::TargetId::get()
{
    if (!_targetId && !IsDisposed)
        _targetId = gcnew GitId(git_tag_target_id(Handle));

    return _targetId;
}

bool GitObject::Tag(String ^tagName, GitTagArgs ^args)
{
    GitId ^ignored;
    return Tag(tagName, args, ignored);
}

bool GitObject::Tag(String ^tagName, GitTagArgs ^args, [Out] GitId ^%id)
{
    if (String::IsNullOrEmpty(tagName))
        throw gcnew ArgumentNullException("tagName");
    else if (! args)
        throw gcnew ArgumentNullException("args");

    GitPool pool;
    git_oid result;
    int r;
    if (!args->LightWeight)
    {
        r = git_tag_create(&result, Repository->Handle,
            pool.AllocString(tagName),
            Handle,
            args->Signature->Alloc(Repository, %pool),
            args->AllocLogMessage(%pool),
            args->OverwriteExisting);
    }
    else
    {
        r = git_tag_create_lightweight(&result, Repository->Handle,
            pool.AllocString(tagName),
            Handle,
            args->OverwriteExisting);
    }

    if (r == 0)
        id = gcnew GitId(result);
    else
        id = nullptr;

    return args->HandleGitError(this, r);
}

/*bool GitBranchCollection::Contains(GitReference ^reference)
{
if (_repository->IsDisposed)
return false;

int r = git_branch_lookup(
}*/
