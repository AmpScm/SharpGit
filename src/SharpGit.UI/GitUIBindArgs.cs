using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using System.Windows.Forms.Design;

namespace SharpGit.UI
{
    /// <summary>
    /// Argument container for <see cref="GitUI.Bind(GitClient, GitUIBindArgs)"/>
    /// </summary>
    public class GitUIBindArgs
    {
        IWin32Window _parentWindow;
        ISynchronizeInvoke _synchronizer;
        Image _headerImage;
        IUIService _uiService;
        Size _autoScaleBaseSize;

        /// <summary>
        /// Initializes a new instance of the <see cref="GitUIBindArgs"/> class.
        /// </summary>
        public GitUIBindArgs()
        {
        }

        /// <summary>
        /// Gets or sets the parent window.
        /// </summary>
        /// <value>The parent window.</value>
        public IWin32Window ParentWindow
        {
            get { return _parentWindow; }
            set { _parentWindow = value; }
        }

        /// <summary>
        /// Gets or sets the synchronizer.
        /// </summary>
        /// <value>The synchronizer.</value>
        public ISynchronizeInvoke Synchronizer
        {
            get { return _synchronizer; }
            set { _synchronizer = value; }
        }

        /// <summary>
        /// Gets or sets the header image.
        /// </summary>
        /// <value>The header image.</value>
        public Image HeaderImage
        {
            get { return _headerImage; }
            set { _headerImage = value; }
        }

        /// <summary>
        /// Gets or sets the UI service.
        /// </summary>
        /// <remarks>The UI service allows redirecting dialog processing to the specific UI service.
        /// (You should use this within .Net development environments (Like MS Visual Studio) implementing this interface)</remarks>
        /// <value>The UI service.</value>
        public IUIService UIService
        {
            get { return _uiService; }
            set { _uiService = value; }
        }

        /// <summary>
        /// Gets or sets the size of the auto scale base.
        /// </summary>
        /// <value>The size of the auto scale base.</value>
        public Size AutoScaleBaseSize
        {
            get { return _autoScaleBaseSize; }
            set { _autoScaleBaseSize = value; }
        }
    }
}
