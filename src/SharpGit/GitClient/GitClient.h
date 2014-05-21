#pragma once

#include "GitClientContext.h"
#include "GitClientEventArgs.h"

namespace SharpGit {

    ref class GitStatusArgs;
    ref class GitAddArgs;
    ref class GitDeleteArgs;
    ref class GitCloneArgs;
    ref class GitCommitArgs;
    ref class GitStageArgs;
    ref class GitUnstageArgs;
    ref class GitFetchArgs;
    ref class GitPushArgs;
    ref class GitMergeArgs;
    ref class GitPullArgs;

    namespace Implementation {
        ref class GitLibrary;
    }
    namespace Plumbing {
        ref class GitRepository;
    }

    using ::SharpGit::Implementation::GitLibrary;
    using ::SharpGit::Plumbing::GitRepository;
    using System::Collections::Generic::ICollection;

    public ref class GitProgressEventArgs : public GitEventArgs
    {
    internal:
        GitProgressEventArgs(const char *str, int len)
        {}
    };

    public ref class GitRemoteCompletedEventArgs : public GitEventArgs
    {
    internal:
        GitRemoteCompletedEventArgs(git_remote_completion_type completion)
        {}
    };

    [Flags]
    public enum class GitCredentialTypes
    {
        None,
        Simple = GIT_CREDTYPE_USERPASS_PLAINTEXT,
        SshKey = GIT_CREDTYPE_SSH_KEY,
        SshCustom = GIT_CREDTYPE_SSH_CUSTOM,
        DefaultCredentials = GIT_CREDTYPE_DEFAULT,
        SshInteractive = GIT_CREDTYPE_SSH_INTERACTIVE,
    };

    public ref class GitCredentialsEventArgs : public GitEventArgs
    {
    internal:
        GitCredentialsEventArgs(const char *url, const char *username_from_url, unsigned int allowed_types)
        {}
    };

    public ref class GitTransferProgressEventArgs : public GitEventArgs
    {
    internal:
        GitTransferProgressEventArgs(const git_transfer_progress &stats)
        {}
    };

    public ref class GitUpdateTipsEventArgs : public GitEventArgs
    {
    internal:
        GitUpdateTipsEventArgs(const char *refname, const git_oid *a, const git_oid *b)
        {}
    };

    /// <summary>Git client instance; main entrance to the SharpGit Client api</summary>
    /// <threadsafety static="true" instance="false"/>
    public ref class GitClient : GitClientContext
    {
        static ICollection<GitLibrary^>^ _gitLibraries;
        initonly Implementation::AprBaton<GitClient^> ^_clientBaton;
        git_remote_callbacks *_callbacks;
    public:
        GitClient();

    internal:
        git_remote_callbacks *get_callbacks();

    private:
        static void AssertNotBare(GitRepository ^repository);
        static void AssertNotBare(GitRepository %repository);

    public:
        bool Status(String ^path, EventHandler<GitStatusEventArgs^>^ status);
        bool Status(String ^path, GitStatusArgs ^args, EventHandler<GitStatusEventArgs^>^ status);

        bool Add(String ^path);
        bool Add(String ^path, GitAddArgs ^args);

        bool Delete(String ^path);
        bool Delete(String ^path, GitDeleteArgs ^args);

        bool Clone(System::Uri ^remoteRepository, String ^path);
        bool Clone(String ^localRepository, String ^path);

        bool Clone(System::Uri ^remoteRepository, String ^path, GitCloneArgs ^args);
        bool Clone(String ^localRepository, String ^path, GitCloneArgs ^args);

        bool Commit(String ^path);
        bool Commit(System::Collections::Generic::ICollection<String^> ^paths);
        bool Commit(String ^path, GitCommitArgs ^args);
        bool Commit(System::Collections::Generic::ICollection<String^> ^paths, GitCommitArgs ^args);

        bool Commit(String ^path, [Out] GitId ^%commitId);
        bool Commit(System::Collections::Generic::ICollection<String^> ^paths, [Out] GitId ^%commitId);
        bool Commit(String ^path, GitCommitArgs ^args, [Out] GitId ^%commitId);
        bool Commit(System::Collections::Generic::ICollection<String^> ^paths, GitCommitArgs ^args, [Out] GitId ^%commitId);

        bool Stage(String ^path);
        bool Stage(String ^path, GitStageArgs^ args);

        bool Stage(IEnumerable<String^>^ paths);
        bool Stage(IEnumerable<String^>^ paths, GitStageArgs^ args);

        bool Unstage(String ^path);
        bool Unstage(String ^path, GitUnstageArgs^ args);

        bool Unstage(IEnumerable<String^>^ paths);
        bool Unstage(IEnumerable<String^>^ paths, GitUnstageArgs^ args);

        bool Fetch(String ^localRepository);
        bool Fetch(String ^localRepository, GitFetchArgs ^args);

        bool Push(String ^localRepository);
        bool Push(String ^localRepository, GitPushArgs ^args);

        bool Pull(String ^localRepository);
        bool Pull(String ^localRepository, GitPullArgs ^args);

    private:
        bool PushAll(GitRepository ^repo, GitPushArgs ^args);
        bool PushMirror(GitRepository ^repo, GitPushArgs ^args);
        bool PushTags(GitRepository ^repo, GitPushArgs ^args);
        bool PushDefault(GitRepository ^repo, GitPushArgs ^args);
        
    private:
        bool CloneInternal(const char *rawRepository, String ^path, GitCloneArgs ^args, GitPool ^pool);
    public:
        /// <summary>Gets the libgit2 version</summary>
        static property System::Version ^Version
        {
            System::Version^ get();
        }

        /// <summary>Gets the SharpGit version</summary>
        static property System::Version ^SharpGitVersion
        {
            System::Version^ get();
        }

        property static System::Collections::Generic::ICollection<GitLibrary^>^ GitLibraries
        {
            System::Collections::Generic::ICollection<GitLibrary^>^ get();
        }

        static property String^ AdministrativeDirectoryName
        {
            String^ get()
            {
                return ".git";
            }
        }

    protected:
        virtual void OnProgress(GitProgressEventArgs ^e) {}
        virtual void OnRemoteCompleted(GitRemoteCompletedEventArgs ^e) {}
        virtual void OnCredentials(GitCredentialsEventArgs ^e) {}
        virtual void OnTransferProgress(GitTransferProgressEventArgs ^e) {}
        virtual void OnUpdateTips(GitUpdateTipsEventArgs ^e) {}

    internal:
        void InvokeProgress(GitProgressEventArgs ^e) { OnProgress(e); }
        void InvokeRemoteCompleted(GitRemoteCompletedEventArgs ^e) { OnRemoteCompleted(e); }
        void InvokeCredentials(git_cred*& creds, GitCredentialsEventArgs ^e)
        {
            OnCredentials(e);
            // ### TODO: Fill creds
        }
        void InvokeTransferProgress(GitTransferProgressEventArgs ^e) { OnTransferProgress(e); }
        void InvokeUpdateTips(GitUpdateTipsEventArgs ^e) { OnUpdateTips(e); }
    private:
        ~GitClient();
    };

}
