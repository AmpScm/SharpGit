using System;
using System.Collections.Generic;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace SharpGit.Tests
{
    [TestClass]
    public class StatusTests : TestBase
    {
        [TestMethod]
        public void FullStatus()
        {
            using(GitClient client = new GitClient())
            {
                string dir = GitTools.GetTruePath(GetTempPath(), true);
                client.Init(dir);

                GitStatusArgs sa = new GitStatusArgs();
                sa.GenerateVersionedDirs = true;
                sa.IncludeUnmodified = true;
                sa.IncludeIgnored = true; // Directories with 0 files are ignored
                sa.IncludeDirectories = true;

                BuildGreek(dir);

                {
                    var paths = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                    int n = 0;
                    foreach (string[] s in GreekTree())
                        paths.Add(GitTools.GetNormalizedFullPath(Path.Combine(dir, s[0])));

                    paths.Add(dir);

                    n = 0;
                    client.Status(dir, sa,
                        delegate(object sender, GitStatusEventArgs e)
                        {
                            Assert.IsTrue(GitTools.IsNormalizedFullPath(e.FullPath));
                            Assert.IsTrue(paths.Remove(e.FullPath), "Can remove {0}", e.FullPath);
                            if (e.NodeKind == GitNodeKind.File)
                            {
                                Assert.AreEqual(GitStatus.None, e.IndexStatus, "Invalid index status on {0}", e.RelativePath);
                                Assert.AreEqual(GitStatus.New, e.WorkingDirectoryStatus, "Invalid working status on {0}", e.RelativePath);
                                Assert.IsFalse(e.Ignored);
                            }
                            else if (e.FullPath == dir)
                            {
                                Assert.AreEqual(GitStatus.None, e.WorkingDirectoryStatus);
                                Assert.IsFalse(e.Ignored);
                            }
                            else if (Path.GetFileName(e.FullPath) == "F" || Path.GetFileName(e.FullPath) == "C")
                            {   // Empty directory
                                Assert.AreEqual(GitStatus.Normal, e.WorkingDirectoryStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.AreEqual(GitStatus.None, e.IndexStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.IsTrue(e.Ignored);
                            }
                            else
                            {
                                Assert.AreEqual(GitStatus.None, e.IndexStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.AreEqual(GitStatus.None, e.WorkingDirectoryStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.IsFalse(e.Ignored);
                            }
                            n++;
                        });
                    Assert.AreEqual(19, n);
                    Assert.AreEqual(2, paths.Count);
                }

                foreach (string[] k in GreekTree())
                {
                    string p = Path.Combine(dir, k[0]);
                    if (File.Exists(p))
                        client.Stage(p);
                }

                string A_dir = Path.Combine(dir, "A\\B");

                {
                    int n = 0;
                    client.Status(A_dir, sa,
                        delegate(object sender, GitStatusEventArgs e)
                        {
                            Assert.IsTrue(GitTools.IsNormalizedFullPath(e.FullPath));
                            Assert.IsTrue(e.FullPath.StartsWith(A_dir + "\\") || e.FullPath == A_dir);
                            n++;
                        });
                    Assert.AreEqual(5, n);
                }

                {
                    var paths = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                    int n = 0;
                    foreach (string[] s in GreekTree())
                        paths.Add(GitTools.GetNormalizedFullPath(Path.Combine(dir, s[0])));

                    paths.Add(dir);

                    n = 0;
                    client.Status(dir, sa,
                        delegate(object sender, GitStatusEventArgs e)
                        {
                            Assert.IsTrue(Path.IsPathRooted(e.FullPath));
                            Assert.IsTrue(paths.Remove(e.FullPath), "Can remove {0}", e.FullPath);
                            if (e.NodeKind == GitNodeKind.File)
                            {
                                Assert.AreEqual(GitStatus.New, e.IndexStatus, "Invalid index status on {0}", e.RelativePath);
                                Assert.AreEqual(GitStatus.Normal, e.WorkingDirectoryStatus, "Invalid working status on {0}", e.RelativePath);
                                Assert.IsFalse(e.Ignored);
                            }
                            else if (e.FullPath == dir)
                            {
                                Assert.AreEqual(GitStatus.None, e.WorkingDirectoryStatus);
                                Assert.IsFalse(e.Ignored);
                            }
                            else if (Path.GetFileName(e.FullPath) == "F" || Path.GetFileName(e.FullPath) == "C")
                            {   // Empty directory
                                Assert.AreEqual(GitStatus.Normal, e.WorkingDirectoryStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.AreEqual(GitStatus.None, e.IndexStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.IsTrue(e.Ignored);
                            }
                            else
                            {
                                Assert.AreEqual(GitStatus.None, e.IndexStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.AreEqual(GitStatus.None, e.WorkingDirectoryStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.IsFalse(e.Ignored);
                            }
                            n++;
                        });
                    Assert.AreEqual(19, n);
                    Assert.AreEqual(2, paths.Count);
                }

                client.Commit(dir);

                {
                    List<string> paths = new List<string>();
                    int n = 0;
                    foreach (string[] s in GreekTree())
                        paths.Add(GitTools.GetNormalizedFullPath(Path.Combine(dir, s[0])));

                    paths.Add(dir);

                    n = 0;
                    client.Status(dir, sa,
                        delegate(object sender, GitStatusEventArgs e)
                        {
                            Assert.IsTrue(GitTools.IsNormalizedFullPath(e.FullPath));
                            Assert.IsTrue(paths.Remove(e.FullPath), "Can remove {0}", e.FullPath);
                            Assert.AreEqual(e.NodeKind, File.Exists(e.FullPath) ? GitNodeKind.File : GitNodeKind.Directory);
                            Assert.IsFalse(e.IndexModified);
                            Assert.IsFalse(e.WorkingDirectoryModified);
                            n++;
                        });
                    Assert.AreEqual(19, n);
                    Assert.AreEqual(2, paths.Count);
                }
            }
        }

        [TestMethod]
        public void TreeReplacements()
        {
            using (GitClient client = new GitClient())
            {
                string dir = GitTools.GetTruePath(GetTempPath(), true);
                client.Init(dir);

                GitStatusArgs sa = new GitStatusArgs();
                sa.GenerateVersionedDirs = true;
                sa.IncludeUnmodified = true;
                sa.IncludeIgnored = true; // Directories with 0 files are ignored
                sa.IncludeUnversioned = true;

                BuildGreek(dir);

                foreach (string[] k in GreekTree())
                {
                    string p = Path.Combine(dir, k[0]);
                    if (File.Exists(p))
                        client.Stage(p);
                }
                client.Commit(dir);

                Directory.Delete(Path.Combine(dir, "A"), true);
                File.WriteAllText(Path.Combine(dir, "A"), "'A' file");

                File.Delete(Path.Combine(dir, "mu"));
                Directory.CreateDirectory(Path.Combine(dir, "mu"));
                File.WriteAllText(Path.Combine(dir, "mu/AAAA"), "AAAA file");


                List<string> paths = new List<string>();
                int n = 0;
                foreach (string[] s in GreekTree())
                    paths.Add(GitTools.GetNormalizedFullPath(Path.Combine(dir, s[0])));

                paths.Add(dir);
                paths.Add(Path.Combine(dir, "mu"));
                paths.Add(Path.Combine(dir, "mu\\AAAA"));

                /* Empty dirs are not versioned, so not reported deleted */
                paths.Remove(Path.Combine(dir, "A\\C"));
                paths.Remove(Path.Combine(dir, "A\\B\\F"));

                List<string> paths2 = new List<string>(paths);

                n = 0;
                client.Status(dir, sa,
                    delegate(object sender, GitStatusEventArgs e)
                    {
                        Assert.IsTrue(GitTools.IsNormalizedFullPath(e.FullPath));
                        Assert.IsTrue(paths.Remove(e.FullPath), "Can remove {0}", e.FullPath);
                        Assert.IsFalse(e.IndexModified);
                        Assert.IsFalse(e.WorkingDirectoryModified);
                        n++;
                    });
                Assert.AreEqual(21, n);
                Assert.AreEqual(0, paths.Count);

                client.Status(dir, sa,
                    delegate(object sender, GitStatusEventArgs e)
                    {
                        switch (e.WorkingDirectoryStatus)
                        {
                            case GitStatus.New:
                            case GitStatus.Deleted:
                            case GitStatus.TypeChanged:
                                client.Stage(e.FullPath);
                                break;
                            default:
                                if (e.WorkingDirectoryModified)
                                    goto case GitStatus.New;
                                break;
                        }
                    });

                paths = new List<string>(paths2);
                n = 0;
                client.Status(dir, sa,
                    delegate(object sender, GitStatusEventArgs e)
                    {
                        Assert.IsTrue(GitTools.IsNormalizedFullPath(e.FullPath));
                        Assert.IsTrue(paths.Remove(e.FullPath), "Can remove {0}", e.FullPath);
                        Assert.IsFalse(e.IndexModified);
                        Assert.IsFalse(e.WorkingDirectoryModified);
                        n++;
                    });
                Assert.AreEqual(21, n);
                Assert.AreEqual(0, paths.Count);


                client.Status(dir, sa,
                    delegate(object sender, GitStatusEventArgs e)
                    {
                        switch (e.IndexStatus)
                        {
                            case GitStatus.New:
                            case GitStatus.Deleted:
                            case GitStatus.TypeChanged:
                                client.Unstage(e.FullPath);
                                break;
                        }
                    });
            }
        }
    }
}
