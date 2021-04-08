#include "pch.h"

#include "GitRepository.h"
#include "GitBranch.h"
#include "GitReference.h"

using namespace SharpGit;
using namespace SharpGit::Implementation;
using namespace SharpGit::Plumbing;
using System::Collections::Generic::List;

struct git_reference {};

bool GitReference::IsDisposed::get()
{
    if (_repository->IsDisposed)
        return true;

    return ! (_reference || _name);
}

String^ GitReference::Name::get()
{
    if (!_name && _reference && !_repository->IsDisposed)
    {
        _name = GitBase::Utf8_PtrToString(git_reference_name(_reference));
    }

    return _name;
}

GitId ^ GitReference::TargetId::get()
{
    const git_oid *target = git_reference_target(Handle);

    if (target)
        return gcnew GitId(target);
    else
        return nullptr;
}

const git_reference* GitReference::Handle::get()
{
    if (IsDisposed)
        throw gcnew ObjectDisposedException("reference");

    // Not thread safe!
    if (!_reference && _name)
    {
        GitPool pool(_repository->Pool);
        git_reference *reference;
        int r = git_reference_lookup(&reference, _repository->Handle, pool.AllocString(Name));

        if (r == 0)
            _reference = reference;
    }

    return _reference;
}


bool GitReference::Delete()
{
    return Delete(gcnew GitNoArgs());
}

bool GitReference::Delete(GitArgs ^args)
{
    if (!args)
        throw gcnew ArgumentNullException("args");

    int r = git_reference_delete(const_cast<git_reference*>(Handle));

    try
    {
        return args->HandleGitError(this, r);
    }
    finally
    {
        delete this;
    }
}

bool GitReference::EnsureLog()
{
    GitPool pool(_repository->Pool);
    GIT_THROW(git_reference_ensure_log(_repository->Handle, pool.AllocString(Name)));
    return true;
}

bool GitReference::HasLog::get()
{
    GitPool pool(_repository->Pool);
    return git_reference_has_log(_repository->Handle, pool.AllocString(Name)) != 0;
}

GitBranch ^ GitReference::AsBranch()
{
    if (!IsBranch)
        throw gcnew InvalidOperationException();

    GitBranch ^branch;
    if (_repository->Branches->TryGet(Name, branch))
        return branch;

    return nullptr;
}

GitReferenceCollection^ GitRepository::References::get()
{
    if (! _references)
        gcnew GitReferenceCollection(this);

    return _references;
}

System::Collections::Generic::IEnumerator<GitReference^>^ GitReferenceCollection::GetEnumerator()
{
    return GetEnumerable()->GetEnumerator();
}

private ref class RefWalkInfo
{
public:
    List<GitReference^>^ references;
    GitRepository^ repository;
};

static int __cdecl for_reference(git_reference *reference, void *payload)
{
    GitRoot<RefWalkInfo^> root(payload);

    root->references->Add(gcnew GitReference(root->repository, reference));
    return 0;
}

System::Collections::Generic::IEnumerable<GitReference^>^ GitReferenceCollection::GetEnumerable()
{
    List<GitReference^> ^references = gcnew List<GitReference^>();
    if (_repository->IsDisposed)
        return references->AsReadOnly();

    GitRoot<RefWalkInfo^> root(gcnew RefWalkInfo());
    root->references = references;
    root->repository = _repository;

    int r = git_reference_foreach(_repository->Handle, for_reference, root.GetBatonValue());
    if (r != 0)
        references->Clear();

    return references->AsReadOnly();
}

/*bool GitReferenceCollection::Create(String ^name, GitId ^id, GitCreateRefArgs ^args)
{
    if (String::IsNullOrEmpty(name))
        throw gcnew ArgumentNullException("name");
    else if (!id)
        throw gcnew ArgumentNullException("id");

    //git_reference_symbolic_create()
    return false;
}
*/