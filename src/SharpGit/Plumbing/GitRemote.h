#pragma once
#include "GitClientContext.h"
#include "GitRepository.h"

namespace SharpGit {
    namespace Plumbing {

        public ref class GitRemote : public Implementation::GitBase
        {
            initonly GitRepository ^_repository;
            git_remote *_remote;
            String^ _name;

        internal:
            GitRemote(GitRepository ^repository, git_remote *remote);

        private:
            ~GitRemote();
            !GitRemote();

        internal:
            void SetCallbacks(const git_remote_callbacks *callbacks);
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

        public:
            property String^ Name
            {
                String ^ get();
            }
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