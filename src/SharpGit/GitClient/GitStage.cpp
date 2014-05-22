#include "stdafx.h"
#include "GitClient.h"
#include "GitStageArgs.h"

#include "Plumbing/GitRepository.h"
#include "Plumbing/GitIndex.h"

using namespace SharpGit;
using namespace SharpGit::Plumbing;

void GitClient::AssertNotBare(GitRepository ^repository)
{
    if (! repository)
        throw gcnew ArgumentNullException("repository");

    if (repository->IsBare)
        throw gcnew InvalidOperationException("Not valid on a bare repository");
}

void GitClient::AssertNotBare(GitRepository %repository)
{
    AssertNotBare(%repository);
}

bool GitClient::Add(String ^path)
{
    return Stage(path, gcnew GitStageArgs());
}

bool GitClient::Add(String ^path, GitStageArgs ^args)
{
    return Stage(path, args);
}

bool GitClient::Stage(String ^path)
{
	return Stage(path, gcnew GitStageArgs());
}

bool GitClient::Stage(String ^path, GitStageArgs^ args)
{
    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");

	return Stage(gcnew array<String^> { path }, args);
}

bool GitClient::Stage(IEnumerable<String^>^ paths)
{
	return Stage(paths, gcnew GitStageArgs());
}

bool GitClient::Stage(IEnumerable<String^>^ paths, GitStageArgs^ args)
{
    if (! paths)
        throw gcnew ArgumentNullException("path");
    else if (! args)
        throw gcnew ArgumentNullException("args");

    GitRepository repo;

    for each (String ^ p in paths)
    {
        if (!repo.Locate(p, args))
            return false;
    }

    if (repo.IsDisposed)
        return false;

    AssertNotBare(repo);

    GitPool pool(_pool);

    for each (String ^path in paths)
    {
        if (!repo.Index->Add(repo.MakeRelpath(path, %pool), args, %pool))
            return false;
    }
    
    return repo.Index->Write(args);
}
