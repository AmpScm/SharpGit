#pragma once
#include "GitClientContext.h"
#include "GitRepository.h"

namespace SharpGit {
    ref class GitPushArgs;
    using System::Collections::Generic::IEnumerable;

    namespace Plumbing {

        ref class GitRefSpec;

        public enum class GitTagSynchronize
        {
            Auto = GIT_REMOTE_DOWNLOAD_TAGS_AUTO,
            None = GIT_REMOTE_DOWNLOAD_TAGS_NONE,
            All = GIT_REMOTE_DOWNLOAD_TAGS_ALL
        };

        typedef int (__cdecl *git_push_status)(const char *ref, const char *msg, void *payload);

        public ref class GitRemoteHead : public Implementation::GitBase
        {
            initonly bool _local;
            initonly GitId ^_oid;
            initonly GitId ^_loid;
            initonly String ^_name;
            initonly String ^_symrefTarget;

        internal:
            GitRemoteHead(const git_remote_head *init)
            {
                _local = (init->local != 0);
                _oid = gcnew GitId(init->oid);
                _loid = gcnew GitId(init->loid);
                _name = init->name ? GitBase::Utf8_PtrToString(init->name) : nullptr;
                _symrefTarget = init->symref_target ? GitBase::Utf8_PtrToString(init->symref_target) : nullptr;
            }

        public:
            property bool Local
            {
                bool get()
                {
                    return _local;
                }
            }

            property GitId ^ Id
            {
                GitId ^ get()
                {
                    return _oid;
                }
            }

            property GitId ^ LocalId
            {
                GitId ^ get()
                {
                    return _loid;
                }
            }

            property String ^ Name
            {
                String ^ get()
                {
                    return _name;
                }
            }

            property String ^ SymbolicReferenceTarget
            {
                String ^ get()
                {
                    return _symrefTarget;
                }
            }
        };

        public ref class GitRemote : public Implementation::GitBase
        {
            initonly GitRepository ^_repository;
            git_remote *_remote;
            String^ _name;

            git_packbuilder_progress _pack_progress_cb;
            git_push_transfer_progress _transfer_progress_cb;
            git_push_status _push_status_cb;
            void *_push_payload;
            bool _connected;

        internal:
            GitRemote(GitRepository ^repository, git_remote *remote);

        private:
            ~GitRemote();
            !GitRemote();

        internal:
            void SetCallbacks(const git_remote_callbacks *callbacks);
            void SetPushCallbacks(git_packbuilder_progress pack_progress_cb,
                                  git_push_transfer_progress transfer_progress_cb,
                                  git_push_status _git_push_status_cb,
                                  void *push_payload);

            property git_remote * Handle
            {
                git_remote * get()
                {
                    if (!_remote)
                        throw gcnew ObjectDisposedException("GitRemote");

                    return _remote;
                }
            }

        public:
            bool Connect(bool forFetch, GitArgs ^args);
            bool Download(GitArgs ^args);
            bool Disconnect(GitArgs ^args);
            bool UpdateTips(GitCreateRefArgs ^args);

            bool Save(GitArgs ^args);

            void Stop(GitArgs ^args);

            bool Delete();

        public:
            bool Push(IEnumerable<GitRefSpec^>^ references, GitPushArgs ^args);

        internal:
            String ^FetchSpecTransformToSource(String ^spec);

        public:
            property String^ Name
            {
                String ^ get();
            }

            property GitTagSynchronize TagSynchronize
            {
                GitTagSynchronize get()
                {
                    return (GitTagSynchronize)git_remote_autotag(Handle);
                }
                void set(GitTagSynchronize value)
                {
                    git_remote_set_autotag(Handle, (git_remote_autotag_option_t)value);
                }
            }

            property bool FetchHead
            {
                bool get()
                {
                    return git_remote_update_fetchhead(Handle) != 0;
                }
                void set(bool value)
                {
                    git_remote_set_update_fetchhead(Handle, value);
                }
            }

            property System::Uri ^ Uri
            {
                System::Uri ^ get();
                void set(System::Uri ^value);
            }

            property System::Uri ^ PushUri
            {
                System::Uri ^ get();
                void set(System::Uri ^value);
            }

            /// <summary>When connected: Provides the default branch</summary>
            property String ^ DefaultBranch
            {
                String ^ get();
            }

            property IEnumerable<GitRefSpec^>^ FetchRefSpecs
            {
                IEnumerable<GitRefSpec^>^ get();
                void set(IEnumerable<GitRefSpec^>^ value);
            }

            property IEnumerable<GitRefSpec^>^ PushRefSpecs
            {
                IEnumerable<GitRefSpec^>^ get();
                void set(IEnumerable<GitRefSpec^>^ value);
            }

            IList<GitRemoteHead^>^ GetHeads();
        };

        public ref class GitRemoteCollection sealed : public Implementation::GitBase,
                                                      public System::Collections::Generic::IEnumerable<GitRemote^>
        {
            initonly GitRepository ^_repository;

        internal:
            GitRemoteCollection(GitRepository^ repository)
            {
                if (!repository)
                    throw gcnew ArgumentNullException("repository");

                _repository = repository;
            }

        public:
            virtual System::Collections::Generic::IEnumerator<GitRemote^>^ GetEnumerator();

            bool TryGet(String ^name, [Out] GitRemote ^%value);

            GitRemote ^ CreateAnonymous(Uri ^remoteRepository);

        private:
            virtual System::Collections::IEnumerator^ GetObjectEnumerator() sealed
                = System::Collections::IEnumerable::GetEnumerator
            {
                return GetEnumerator();
            }
        };
    }
}