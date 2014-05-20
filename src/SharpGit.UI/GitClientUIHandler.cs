using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SharpGit.UI
{
    class GitClientUIHandler
    {
        private System.Windows.Forms.IWin32Window win32Window;

        public GitClientUIHandler(System.Windows.Forms.IWin32Window win32Window)
        {
            // TODO: Complete member initialization
            this.win32Window = win32Window;
        }

        internal void Bind(GitClient client, GitUIBindArgs args)
        {
            //throw new NotImplementedException();
        }
    }
}
