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
    ref class GitInitArgs;
    ref class GitResetArgs;
    ref class GitRevertCommitArgs;
    ref class GitStashArgs;

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
        initonly System::Collections::Generic::List<System::EventHandler<GitCredentialEventArgs^>^> ^_credentialHandlers;
    public:
        GitClient();

    internal:
        git_remote_callbacks *get_callbacks(GitPool ^pool);

    private:
        static void AssertNotBare(GitRepository ^repository);
        static void AssertNotBare(GitRepository %repository);

    public:
        bool Init(String ^localRepository);
        bool Init(String ^localRepository, GitInitArgs ^args);

        bool Status(String ^path, EventHandler<GitStatusEventArgs^>^ status);
        bool Status(String ^path, GitStatusArgs ^args, EventHandler<GitStatusEventArgs^>^ status);

        /// <summary>Alias for .Stage()</summary>
        bool Add(String ^path);
        /// <summary>Alias for .Stage()</summary>
        bool Add(String ^path, GitStageArgs ^args);

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

        bool Stash(String ^localRepository);
        bool Stash(String ^localRepository, GitStashArgs ^args);
        bool Stash(String ^localRepository, [Out] GitId ^%stashId);
        bool Stash(String ^localRepository, GitStashArgs ^args, [Out] GitId ^%stashId);

        bool Reset(String ^localRepository);
        bool Reset(String ^localRepository, GitResetArgs ^args);

        bool RevertCommit(String ^localRepository, GitId ^commitId);
        bool RevertCommit(String ^localRepository, GitId ^commitId, GitRevertCommitArgs ^args);

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
        virtual void OnCredential(GitCredentialEventArgs ^e);
        virtual void OnTransferProgress(GitTransferProgressEventArgs ^e) {}
        virtual void OnUpdateTips(GitUpdateTipsEventArgs ^e) {}

    internal:
        void InvokeProgress(GitProgressEventArgs ^e) { OnProgress(e); }
        void InvokeRemoteCompleted(GitRemoteCompletedEventArgs ^e) { OnRemoteCompleted(e); }
        void InvokeCredential(GitCredentialEventArgs ^e)
        {
            OnCredential(e);
        }
        void InvokeTransferProgress(GitTransferProgressEventArgs ^e) { OnTransferProgress(e); }
        void InvokeUpdateTips(GitUpdateTipsEventArgs ^e) { OnUpdateTips(e); }

        virtual void HookCredentials(bool add, System::EventHandler<GitCredentialEventArgs^> ^handler) override;
    private:
        ~GitClient();
    };

}
