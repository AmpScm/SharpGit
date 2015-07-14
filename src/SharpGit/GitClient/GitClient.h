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

      interface class IHasRemoteCallbacks
      {
          git_remote_callbacks *get_callbacks(GitPool ^pool);
      };


    }
    namespace Plumbing {
        ref class GitRepository;
    }

    using ::SharpGit::Implementation::GitLibrary;
    using ::SharpGit::Plumbing::GitRepository;
    using System::Collections::Generic::ICollection;

    public ref class GitProgressEventArgs : public GitEventArgs
    {
        initonly int _len;
        String ^_text;
        const char *_str;

    internal:
        GitProgressEventArgs(const char *str, int len)
        {
            _str = str;
            _len = len;
        }

    public:
        property String ^ Text
        {
            String ^ get()
            {
                if (!_text && _str)
                    _text = GitBase::Utf8_PtrToString(_str);

                return _text;
            }
        }

    protected public:
        virtual void Detach(bool keepValues) override
        {
            try
            {
                if (keepValues)
                {
                    GC::KeepAlive(Text);
                }
            }
            finally
            {
                _str = nullptr;
                __super::Detach(keepValues);
            }
        }
    };

    public enum class GitCompletedAction
    {
        Download = GIT_REMOTE_COMPLETION_DOWNLOAD,
        Indexing = GIT_REMOTE_COMPLETION_INDEXING,
        Error = GIT_REMOTE_COMPLETION_ERROR,
    };

    public ref class GitRemoteCompletedEventArgs : public GitEventArgs
    {
        initonly GitCompletedAction _action;
    internal:
        GitRemoteCompletedEventArgs(git_remote_completion_type completion)
        {
            _action = (GitCompletedAction)completion;
        }

    public:
        property GitCompletedAction Action
        {
            GitCompletedAction get()
            {
                return _action;
            }
        }
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

    public ref class GitCertificateEventArgs : public GitEventArgs
    {
    internal:
        GitCertificateEventArgs(git_cert *cert, int valid, const char *host)
        {
            UNUSED(cert);
            UNUSED(valid);
            UNUSED(host);
        }
    };


    public ref class GitTransferProgressEventArgs : public GitEventArgs
    {
        initonly unsigned int _total_objects;
        initonly unsigned int _indexed_objects;
        initonly unsigned int _received_objects;
        initonly unsigned int _local_objects;
        initonly unsigned int _total_deltas;
        initonly unsigned int _indexed_deltas;
        initonly size_t _received_bytes;
    internal:
        GitTransferProgressEventArgs(const git_transfer_progress &stats)
        {
            _total_objects =    stats.total_objects;
            _indexed_objects =  stats.indexed_objects;
            _received_objects = stats.received_objects;
            _local_objects =    stats.local_objects;
            _total_deltas =     stats.total_deltas;
            _indexed_deltas =   stats.indexed_deltas;
            _received_bytes =   stats.received_bytes;
        }

    protected public:
        virtual void Detach(bool keepValues) override
        {
            try
            {
                if (keepValues)
                {
                    //GC::KeepAlive(Text);
                }
            }
            finally
            {
                //_str = nullptr;
                __super::Detach(keepValues);
            }
        }
    };

    public ref class GitUpdateTipsEventArgs : public GitEventArgs
    {
        GitId ^_id1, ^_id2;
        const char *_pRefname;
        const git_oid *_a, *_b;
    internal:
        GitUpdateTipsEventArgs(const char *refname, const git_oid *a, const git_oid *b)
        {
            _pRefname = refname;
            _a = a;
            _b = b;
        }

    public:
        property GitId ^ Id1
        {
            GitId^ get()
            {
                if (!_id1 && _a)
                    _id1 = gcnew GitId(_a);
                return _id1;
            }
        }

        property GitId ^ Id2
        {
            GitId^ get()
            {
                if (!_id2 && _b)
                    _id2 = gcnew GitId(_b);
                return _id2;
            }
        }

    protected public:
        virtual void Detach(bool keepValues) override
        {
            try
            {
                if (keepValues)
                {
                    GC::KeepAlive(Id1);
                    GC::KeepAlive(Id2);
                }
            }
            finally
            {
                _pRefname = nullptr;
                _a = nullptr;
                _b = nullptr;
                __super::Detach(keepValues);
            }
        }
    };

    public ref class GitPackProgressEventArgs : public GitEventArgs
    {
    internal:
        GitPackProgressEventArgs(int stage, unsigned int current, unsigned int total)
        {
            UNUSED(stage);
            UNUSED(current);
            UNUSED(total);
        }
    };

    public ref class GitPushTransferProgressEventArgs : public GitEventArgs
    {
    internal:
        GitPushTransferProgressEventArgs(unsigned int current, unsigned int total, size_t bytes)
        {
            UNUSED(current);
            UNUSED(total);
            UNUSED(bytes);
        }
    };

    public ref class GitPushUpdateReferenceEventArgs : public GitEventArgs
    {
    internal:
        GitPushUpdateReferenceEventArgs(const char *ref, const char *msg)
        {
            UNUSED(ref);
            UNUSED(msg);
        }
    };

    /// <summary>Git client instance; main entrance to the SharpGit Client api</summary>
    /// <threadsafety static="true" instance="false"/>
    public ref class GitClient : GitClientContext, Implementation::IHasRemoteCallbacks
    {
        static ICollection<GitLibrary^>^ _gitLibraries;
        initonly Implementation::AprBaton<GitClient^> ^_clientBaton;
        initonly System::Collections::Generic::List<System::EventHandler<GitCredentialEventArgs^>^> ^_credentialHandlers;
        static initonly System::Version ^_version;
        int _nextCredentialHandler;
        static GitClient();

    public:
        GitClient();

    internal:
        virtual git_remote_callbacks *get_callbacks(GitPool ^pool) = Implementation::IHasRemoteCallbacks::get_callbacks;

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

        static property String ^VersionString
        {
            String^ get();
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
        DECLARE_EVENT(GitProgressEventArgs^, Progress)
        DECLARE_EVENT(GitRemoteCompletedEventArgs^, RemoteCompleted)
        DECLARE_EVENT(GitCertificateEventArgs^, Certificate)
        DECLARE_EVENT(GitTransferProgressEventArgs^, TransferProgress)
        DECLARE_EVENT(GitUpdateTipsEventArgs^, UpdateTips)
        DECLARE_EVENT(GitPackProgressEventArgs^, PackProgress)
        DECLARE_EVENT(GitPushTransferProgressEventArgs^, PushTransferProgress)
        DECLARE_EVENT(GitPushUpdateReferenceEventArgs^, PushUpdateReference)

    protected:
        virtual void OnProgress(GitProgressEventArgs ^e) { Progress(this, e); }
        virtual void OnRemoteCompleted(GitRemoteCompletedEventArgs ^e) { RemoteCompleted(this, e); }
        virtual void OnCredential(GitCredentialEventArgs ^e);
        virtual void OnCertificate(GitCertificateEventArgs ^e) { Certificate(this, e); }
        virtual void OnTransferProgress(GitTransferProgressEventArgs ^e) { TransferProgress(this, e); }
        virtual void OnUpdateTips(GitUpdateTipsEventArgs ^e) { UpdateTips(this, e); }
        virtual void OnPackProgress(GitPackProgressEventArgs ^e) { PackProgress(this, e); }
        virtual void OnPushTransferProgress(GitPushTransferProgressEventArgs ^e) { PushTransferProgress(this, e); }
        virtual void OnPushUpdateReference(GitPushUpdateReferenceEventArgs ^e) { PushUpdateReference(this, e); }

    internal:
        void InvokeProgress(GitProgressEventArgs ^e) { OnProgress(e); }
        void InvokeRemoteCompleted(GitRemoteCompletedEventArgs ^e) { OnRemoteCompleted(e); }
        void InvokeCredential(GitCredentialEventArgs ^e)
        {
            OnCredential(e);
        }
        void InvokeCertificate(GitCertificateEventArgs ^e) { OnCertificate(e); }
        void InvokeTransferProgress(GitTransferProgressEventArgs ^e) { OnTransferProgress(e); }
        void InvokeUpdateTips(GitUpdateTipsEventArgs ^e) { OnUpdateTips(e); }
        void InvokePackProgress(GitPackProgressEventArgs ^e) { OnPackProgress(e); }
        void InvokePushTransferProgress(GitPushTransferProgressEventArgs ^e) { OnPushTransferProgress(e); }
        void InvokePushUpdateReference(GitPushUpdateReferenceEventArgs ^e) { OnPushUpdateReference(e); }

        virtual void HookCredentials(bool add, System::EventHandler<GitCredentialEventArgs^> ^handler) override;
    private:
        ~GitClient();
    };

}
