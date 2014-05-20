using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using SharpGit;

namespace SharpGit.UI
{
    /// <summary>
    ///
    /// </summary>
    public static class GitUI
    {
        /// <summary>
        /// Binds the specified client to the user interface defined by args
        /// </summary>
        /// <param name="client">The client to bind.</param>
        /// <param name="args">The args.</param>
        public static void Bind(GitClient client, GitUIBindArgs args)
        {
            if (client == null)
                throw new ArgumentNullException("client");
            else if (args == null)
                throw new ArgumentNullException("args");

            GitClientUIHandler handler = new GitClientUIHandler(args.ParentWindow);

            handler.Bind(client, args);
        }

        /// <summary>
        /// Binds the specified client to the user interface defined by args
        /// </summary>
        /// <param name="client">The client to bind.</param>
        /// <param name="parentWindow">The parent window.</param>
        public static void Bind(GitClient client, IWin32Window parentWindow)
        {
            if (client == null)
                throw new ArgumentNullException("client");

            GitUIBindArgs args = new GitUIBindArgs();
            args.ParentWindow = parentWindow;

            Bind(client, args);
        }
    }
}
