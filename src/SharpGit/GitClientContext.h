#pragma once
#include <vcclr.h>

using namespace System;
#define UNUSED(x) (x)

namespace SharpGit {

    namespace Implementation {
        ref class GitPool;

        template<class T>
        struct GitRoot : gcroot<T>
        {
            apr_pool_t *_pool;
        public:
            GitRoot(T value)
                : gcroot(value)
            {
                _pool = nullptr;
            }

            GitRoot(T value, GitPool ^pool)
                : gcroot(value)
            {
                _pool = pool->Handle;
            }

            GitRoot(void *batonValue)
                : gcroot(*reinterpret_cast<GitRoot<T>*>(batonValue))
            {
                _pool = reinterpret_cast<GitRoot<T>*>(batonValue)->_pool;
            }

        public:
            void *GetBatonValue() const { return (void*)this; }
            apr_pool_t *GetPool() const { return _pool; }
        };

        public ref class GitBase : public System::MarshalByRefObject
        {
            static bool _aprInitialized;
            initonly static Object^ _ensurerLock = gcnew Object();

        internal:
            GitBase() { EnsureInitialized(); }
            static void EnsureInitialized();

        internal:
            static System::String^ Utf8_PtrToString(const char *ptr);
            static System::String^ Utf8_PtrToString(const char *ptr, int length);
            static System::String ^StringFromDirentNoPool(const char *dirent);
            static System::String ^StringFromDirent(const char *dirent, GitPool ^pool);

        internal:
            static int WrapError(Exception ^e)
            {
                return -1;
            }

            static array<char>^ _invalidCharMap;

            static property array<char>^ InvalidCharMap
            {
                array<char>^ get();
            }
            static void GenerateInvalidCharMap();
            static bool PathContainsInvalidChars(String^ path);
        };

        private ref class GitPool
        {
        private:
            apr_pool_t *_pool;
            GitPool ^_parent;

            static GitPool();

            ~GitPool();
            !GitPool();

        public:
            GitPool(GitPool ^parent)
            {
                _parent = parent;
                _pool = svn_pool_create(parent->Handle);
            }

            GitPool(GitPool %parent)
            {
                _parent = %parent;
                _pool = svn_pool_create(parent.Handle);
            }

            GitPool()
            {
                GitBase::EnsureInitialized();
                _pool = svn_pool_create(nullptr);
            }

            GitPool(apr_pool_t *parent)
            {
                _pool = svn_pool_create(parent);
            }

        public:
            property apr_pool_t *Handle
            {
                apr_pool_t *get()
                {
                    if (!_pool)
                        throw gcnew ObjectDisposedException("pool");
                    return _pool;
                }
            }

        public:
            char *Alloc(size_t sz);
            const char *AllocDirent(String ^path);
            const char *AllocRelpath(String ^relPath);
            const char *AllocString(String^ value);

            git_strarray* AllocStringArray(String ^path);
            git_strarray* AllocStringArray(IEnumerable<String^> ^path);
        };

        using System::Runtime::InteropServices::GCHandle;

        generic<typename T>
        ref class AprBaton : public IDisposable
        {
            GCHandle _handle;

        public:
            AprBaton(T value)
            {
                if (value == nullptr)
                    throw gcnew ArgumentNullException("value");

                _handle = GCHandle::Alloc(value, System::Runtime::InteropServices::GCHandleType::WeakTrackResurrection);
            }

        private:
            !AprBaton()
            {
                if (_handle.IsAllocated)
                    _handle.Free();
            }

            ~AprBaton()
            {
                if (_handle.IsAllocated)
                    _handle.Free();
            }
        public:
            static T Get(IntPtr value)
            {
                return (T)GCHandle::FromIntPtr(value).Target;
            }

            static T Get(void* ptr)
            {
                return (T)GCHandle::FromIntPtr((IntPtr)ptr).Target;
            }


            property void* Handle
            {
                [System::Diagnostics::DebuggerStepThrough]
                void *get()
                {
                    return (void *)GCHandle::ToIntPtr(_handle);
                }
            }
        };
    }

    ref class GitClientContext;

    public ref class GitEventArgs abstract : public EventArgs
    {

    protected public:
        virtual void Detach(bool keepValues)
        {
            UNUSED(keepValues);
        }
    public:
        void Detach()
        {
            Detach(true);
        }
    };



    public ref class GitCredentialEventArgs : public GitEventArgs
    {
        git_cred **_cred;
        initonly int _nr;
        initonly unsigned _allowed;
    internal:
        GitCredentialEventArgs(git_cred **cred, const char *url, const char *username_from_url, unsigned int allowed_types, int nr)
        {
            _allowed = allowed_types;
            _nr = nr;
            _cred = cred;
        }

    public:
        property int Invocation
        {
            int get()
            {
                return _nr;
            }
        }

    public:
        property bool AllowUsername
        {
            bool get()
            {
                return 0 != (_allowed & GIT_CREDTYPE_USERNAME);
            }
        }


        property bool AllowUsernamePassword
        {
            bool get()
            {
                return 0 != (_allowed & GIT_CREDTYPE_USERPASS_PLAINTEXT);
            }
        }

        property bool AllowSshKey
        {
            bool get()
            {
                return 0 != (_allowed & GIT_CREDTYPE_SSH_KEY);
            }
        }

        property bool AllowSshCustom
        {
            bool get()
            {
                return 0 != (_allowed & GIT_CREDTYPE_SSH_CUSTOM);
            }
        }

        property bool AllowDefault
        {
            bool get()
            {
                return 0 != (_allowed & GIT_CREDTYPE_DEFAULT);
            }
        }

        property bool AllowSshInteractive
        {
            bool get()
            {
                return 0 != (_allowed & GIT_CREDTYPE_SSH_INTERACTIVE);
            }
        }

        void SetUsernamePassword(String ^username, String ^password);
        void SetUsername(String ^username);
        void SetDefault();
    };

    public ref class GitAuthentication : public Implementation::GitBase
    {
        initonly GitClientContext ^_ctx;

    internal:
        GitAuthentication(GitClientContext ^ctx)
        {
            if (!ctx)
                throw gcnew ArgumentNullException("ctx");

            _ctx = ctx;
        }

    public:
        event System::EventHandler<GitCredentialEventArgs^>^ Credentials
        {
        public:
            [System::Runtime::CompilerServices::MethodImpl(System::Runtime::CompilerServices::MethodImplOptions::Synchronized)]
            void add(System::EventHandler<GitCredentialEventArgs^>^ value);

            [System::Runtime::CompilerServices::MethodImpl(System::Runtime::CompilerServices::MethodImplOptions::Synchronized)]
            void remove(System::EventHandler<GitCredentialEventArgs^>^ value);

        private:
            void raise(Object^ sender, GitCredentialEventArgs ^e)
            {
                throw gcnew NotImplementedException();
            }
        }
    };

    public ref class GitClientContext abstract : public Implementation::GitBase
    {
        initonly GitAuthentication ^_authentication;
    internal:
        SharpGit::Implementation::GitPool _pool;
        GitClientContext()
        {
            _authentication = gcnew GitAuthentication(this);
        }

    public:
        property GitAuthentication ^ Authentication
        {
            GitAuthentication ^ get()
            {
                return _authentication;
            }
        }

    internal:
        virtual void HookCredentials(bool add, System::EventHandler<GitCredentialEventArgs^> ^handler) = 0;
    };

    public ref class GitArgs abstract
    {
        static GitArgs()
        {}

    public:
        GitArgs()
        {
            GitBase::EnsureInitialized();
        }

    internal:
        literal int WrappedError = 1099001;
        Exception ^_ex;

        bool HandleGitError(Object^ q, int r)
        {
            if (_ex && (r == WrappedError || r == GIT_EUSER))
            {
                try
                {
                    throw gcnew Exception(String::Format("Wrapped Exception: {0}", _ex), _ex);
                }
                finally
                {
                    _ex = nullptr;
                }
            }

            UNUSED(q);
            _ex = nullptr;
            bool defined = false;

            switch (r)
            {
            case GIT_OK /* 0 */:
                return true;

            case GIT_ERROR:
            case GIT_ENOTFOUND:
            case GIT_EEXISTS:
            case GIT_EAMBIGUOUS:
            case GIT_EBUFS:

            case GIT_EUSER:
            case GIT_EBAREREPO:
            case GIT_EUNBORNBRANCH:
            case GIT_EUNMERGED:
            case GIT_ENONFASTFORWARD:
            case GIT_EINVALIDSPEC:
            case GIT_EMERGECONFLICT:
        case GIT_ELOCKED:

            case GIT_PASSTHROUGH:
            case GIT_ITEROVER:
                defined = true;
                // Fall through

            default:
                {
                    const git_error *info = giterr_last();

                    try
                    {
                        if (info)
                            throw GitException::Create(r, info);
                        else
                            throw gcnew GitException();
                    }
                    finally
                    {
                        if (info)
                            giterr_clear();
                    }
                }
            }

            return false;
        }

        bool HandleException(Exception ^e)
        {
            if (e)
                throw e;

            return true;
        }

        int WrapException(Exception ^e)
        {
            if (e)
            {
                System::Diagnostics::Debug::WriteLine(String::Format("Wrapping: {0}", e));
                _ex = e;
                return WrappedError;
            }

            return 0;
        }
    };

    public ref class GitSignature : public GitBase
    {
        literal __int64 DELTA_EPOCH_AS_FILETIME = 116444736000000000i64;
        DateTime _when;
        String ^_name;
        String ^_email;
        bool _readOnly;
        int _offset;

    internal:
        git_signature * Alloc(GitRepository^ repository, GitPool ^pool);

    internal:
        GitSignature(const git_signature *from)
        {
            _name = from->name ? Utf8_PtrToString(from->name) : nullptr;
            _email = from->email ? Utf8_PtrToString(from->email) : nullptr;

            __int64 when = from->when.time;

            _when = DateTime::FromFileTimeUtc(when * 10000000i64 + DELTA_EPOCH_AS_FILETIME);
            _offset = from->when.offset;
            _readOnly = true;
        }

        GitSignature()
        {
            When = DateTime::UtcNow;
        }

    public:
        /// <summary>When the signature was added (as/auto-converted as GMT DateTime)</summary>
        property DateTime When
        {
            DateTime get()
            {
                return _when;
            }
            void set(DateTime value)
            {
                if (_readOnly)
                    throw gcnew InvalidOperationException();

                // Always store as UTC
                if (value.Kind != System::DateTimeKind::Unspecified)
                    _when = value.ToUniversalTime();
                else
                    _when = DateTime(value.Ticks, System::DateTimeKind::Utc);

                // But also store the current offset at the time of setting
                _offset = (int)Math::Round(TimeSpan(value.Ticks - _when.Ticks).TotalMinutes);
            }
        }

        /// <summary>The user name. When using for commit preparation NULL represents the configured value</summary>
        property String^ Name
        {
            String^ get()
            {
                return _name;
            }
            void set(String ^value)
            {
                if (_readOnly)
                    throw gcnew InvalidOperationException();

                _name = value;
            }
        }

        /// <summary>The email address. When using for commit preparation NULL represents the configured value</summary>
        property String^ EmailAddress
        {
            String^ get()
            {
                return _email;
            }
            void set(String ^value)
            {
                if (_readOnly)
                    throw gcnew InvalidOperationException();
                _email = value;
            }
        }

        property int TimeOffsetInMinutes
        {
            int get()
            {
                return _offset;
            }
            void set(int value)
            {
                if (_readOnly)
                    throw gcnew InvalidOperationException();

                // Not strictly OK, but should avoid most problems
                if ((value / 60) * 60 != value)
                    throw gcnew InvalidOperationException();

                _offset = value;
            }
        }

        virtual String^ ToString() override
        {
            System::Text::StringBuilder ^sb = gcnew System::Text::StringBuilder();

            if (Name)
                sb->Append(Name);

            if (EmailAddress)
            {
                if (Name)
                    sb->Append(' ');

                sb->Append('<');
                sb->Append(EmailAddress);
                sb->Append('>');
            }
            if (Name || EmailAddress)
                sb->Append(' ');

            sb->Append(When);

            return sb->ToString();
        }
    };


    public ref class GitClientArgs abstract : public GitArgs
    {
    };

    ref class GitSignature;

    public ref class GitCreateRefArgs abstract : public GitClientArgs
    {
        String ^_logMessage;
        initonly GitSignature ^_committer;
        bool _noNormalize;
        bool _stripComments;

    protected public:
        GitCreateRefArgs()
        {
            _committer = gcnew GitSignature();

            // By default use the exact same timestamp for both
            DateTime now = DateTime::UtcNow;
            _committer->When = now;
        }

    internal:
        const char * AllocLogMessage(GitPool ^pool);

    public:
        property String ^ LogMessage
        {
            String ^get()
            {
                return _logMessage;
            }
            void set(String ^value)
            {
                _logMessage = value;
            }
        }

        property GitSignature^ Signature
        {
            GitSignature^ get()
            {
                return _committer;
            }
        }

        property bool NormalizeLogMessage
        {
            bool get()
            {
                return !_noNormalize;
            }
            void set(bool value)
            {
                _noNormalize = !value;
            }
        }

        property bool StripLogMessageComments
        {
            bool get()
            {
                return _stripComments;
            }
            void set(bool value)
            {
                _stripComments = value;
            }
        }
    };

    private ref class GitNoArgs sealed : GitCreateRefArgs
    {
    };

}
