﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NUnit.Framework;
using SharpGit.Plumbing;
using Assert = NUnit.Framework.Assert;

namespace SharpGit.Tests
{
    [TestClass]
    public class GitClientTests : TestBase
    {

        [TestMethod]
        public void MinVersion()
        {
            Assert.That(GitClient.Version, Is.GreaterThanOrEqualTo(new Version(0, 17)));

            Assert.That(GitClient.SharpGitVersion, Is.GreaterThanOrEqualTo(new Version(GitClient.Version.Major, 100 * GitClient.Version.Minor)));
            Assert.That(GitClient.AdministrativeDirectoryName, Is.EqualTo(".git"));
            Assert.That(GitClient.GitLibraries, Is.Not.Empty);
        }

        [TestMethod]
        public void UseGitClient()
        {
            GitCommitArgs ga = new GitCommitArgs();
            ga.Author.Name = "Tester";
            ga.Author.EmailAddress = "author@example.com";
            ga.Signature.Name = "Other";
            ga.Signature.EmailAddress = "committer@example.com";

            // Use stable time and offset to always produce the same hash
            DateTime ct = new DateTime(2002, 01, 01, 0, 0, 0, DateTimeKind.Utc);
            ga.Author.When = ct;
            ga.Author.TimeOffsetInMinutes = 120;
            ga.Signature.When = ct;
            ga.Signature.TimeOffsetInMinutes = 120;

            string repoDir = GetTempPath();
            string repo2Dir = GetTempPath();
            GitId firstResult;
            GitId lastCommit;
            using (GitRepository repo = GitRepository.Create(repoDir))
            using (GitClient git = new GitClient())
            {
                string ignoreFile = Path.Combine(repoDir, ".gitignore");
                string file = Path.Combine(repoDir, "newfile");
                string subDir = Path.Combine(repoDir, "dir");
                string fileInSubDir = Path.Combine(subDir, "file2");
                string file3 = Path.Combine(repoDir, "other");
                string file4 = Path.Combine(repoDir, "q.ignore");
                File.WriteAllText(file, "Some body");
                Directory.CreateDirectory(subDir);
                File.WriteAllText(fileInSubDir, "Some other body");
                File.WriteAllText(file3, "file3");

                File.WriteAllText(ignoreFile, "*.ignore\n");
                File.WriteAllText(file4, "file4");

                git.Add(ignoreFile);
                git.Add(file);
                git.Commit(repoDir, ga, out firstResult);

                git.Add(fileInSubDir);

                int ticked = 0;

                File.AppendAllText(file, "\nExtra Line");

                GitStatusArgs gsa = new GitStatusArgs();
                gsa.IncludeIgnored = true;
                gsa.IncludeUnmodified = true;

                Assert.That(git.Status(repoDir, gsa,
                    delegate(object sender, GitStatusEventArgs e)
                    {
                        switch (e.RelativePath)
                        {
                            case "newfile":
                                //Assert.That(e.IndexStatus, Is.EqualTo(GitStatus.Added));
                                Assert.That(e.IndexStatus, Is.EqualTo(GitStatus.Normal), "newfile index normal");
                                Assert.That(e.WorkingDirectoryStatus, Is.EqualTo(GitStatus.Normal), "newfile wc modified");
                                Assert.That(e.WorkingDirectoryModified);
                                Assert.That(e.Ignored, Is.False);
                                break;
                            case "dir/file2":
                                Assert.That(e.IndexStatus, Is.EqualTo(GitStatus.New), "file2 index added");
                                Assert.That(e.WorkingDirectoryStatus, Is.EqualTo(GitStatus.Normal), "file2 wc normal");
                                Assert.That(e.Ignored, Is.False);
                                break;
                            case "other":
                                Assert.That(e.IndexStatus, Is.EqualTo(GitStatus.None));
                                Assert.That(e.WorkingDirectoryStatus, Is.EqualTo(GitStatus.New));
                                Assert.That(e.Ignored, Is.False);
                                break;
                            case ".gitignore":
                                Assert.That(e.IndexStatus, Is.EqualTo(GitStatus.Normal));
                                Assert.That(e.WorkingDirectoryStatus, Is.EqualTo(GitStatus.Normal));
                                Assert.That(e.Ignored, Is.False);
                                break;
                            case "q.ignore":
                                // TODO: Make this ignored
                                Assert.That(e.IndexStatus, Is.EqualTo(GitStatus.None));
                                Assert.That(e.WorkingDirectoryStatus, Is.EqualTo(GitStatus.Normal));
                                Assert.That(e.Ignored, Is.True);
                                break;
                            default:
                                Assert.Fail("Invalid node found: {0}", e.RelativePath);
                                break;
                        }

                        Assert.That(e.FullPath, Is.EqualTo(Path.GetFullPath(Path.Combine(repoDir, e.RelativePath))));
                        ticked++;
                    }), Is.True);

                Assert.That(ticked, Is.EqualTo(5), "Ticked");

                ga.LogMessage = "Intermediate";
                git.Commit(repoDir, ga);

                Assert.That(git.Delete(fileInSubDir));
                Assert.That(git.Add(file));

                GitId commit;

                ga.LogMessage = "A log message to remember";

                // The passed path is currently just used to find the local repository
                lastCommit = new GitId("996cf198b49ed6fce3bcba232e2d88eb473560f9");

                Assert.That(git.Commit(repoDir, ga, out commit));
                Assert.That(commit, Is.EqualTo(lastCommit));

                File.Move(file, file + ".a");

                ticked = 0;
                gsa.IncludeIgnored = false;
                gsa.IncludeUnversioned = true;
                gsa.IncludeUnmodified = false;
                Assert.That(git.Status(repoDir, gsa,
                    delegate(object sender, GitStatusEventArgs e)
                    {
                        switch (e.RelativePath)
                        {
                            /*case "dir":
                                Assert.That(e.IndexStatus, Is.EqualTo(GitStatus.Normal), "dir index normal");
                                Assert.That(e.WorkingDirectoryStatus, Is.EqualTo(GitStatus.New), "dir wc normal");
                                break;*/
                            case "newfile":
                                Assert.That(e.IndexStatus, Is.EqualTo(GitStatus.None), "newfile index normal");
                                Assert.That(e.WorkingDirectoryStatus, Is.EqualTo(GitStatus.Deleted), "newfile wc deleted");
                                break;
                            case "newfile.a":
                                Assert.That(e.IndexStatus, Is.EqualTo(GitStatus.None), "newfile.a index normal");
                                Assert.That(e.WorkingDirectoryStatus, Is.EqualTo(GitStatus.New), "newfile.a wc new");
                                break;
                            case "other":
                                Assert.That(e.IndexStatus, Is.EqualTo(GitStatus.None), "other index normal");
                                Assert.That(e.WorkingDirectoryStatus, Is.EqualTo(GitStatus.New), "other wc normal");
                                break;
                            default:
                                Assert.Fail("Invalid node found: {0}", e.RelativePath);
                                break;
                        }

                        Assert.That(e.FullPath, Is.EqualTo(Path.GetFullPath(Path.Combine(repoDir, e.RelativePath))));
                        ticked++;
                    }), Is.True);

                Assert.That(ticked, Is.EqualTo(3));

                GitCloneArgs gc = new GitCloneArgs();
                gc.Synchronous = true;

                git.Clone(repoDir, repo2Dir, gc);

                GitCommit theCommit;
                Assert.That(repo.Lookup(commit, out theCommit));
                Assert.That(repo.Branches.Create(theCommit, "vNext"));
                Assert.That(repo.Branches, Is.Not.Empty);
            }

            using (GitRepository repo1 = new GitRepository(repoDir))
            using (GitRepository repo2 = new GitRepository(repo2Dir))
            {
                GitReference head = repo1.HeadReference;
                Assert.That(head, Is.Not.Null, "Has head");

                Assert.That(head.Name, Is.EqualTo("refs/heads/master"));
                //Assert.That(repo2.Head, Is.Not.Null);

                GitId headId;
                Assert.That(repo1.ResolveReference(repo1.HeadReference, out headId));
                Assert.That(headId, Is.EqualTo(lastCommit));
                GitCommit commit;

                Assert.That(repo1.Lookup(headId, out commit));
                Assert.That(commit, Is.Not.Null, "Have a commit");

                Assert.That(commit.Id, Is.EqualTo(lastCommit));
                Assert.That(commit.Ancestors, Is.Not.Empty);
                Assert.That(commit.Ancestor, Is.Not.Null);
                Assert.That(commit.Ancestor.Ancestor, Is.Not.Null);
                Assert.That(commit.Ancestor.Ancestor.Ancestor, Is.Null);
                Assert.That(commit.Ancestor.Ancestor.Id, Is.EqualTo(firstResult));

                Assert.That(commit.Author, Is.Not.Null);
                Assert.That(commit.Author.Name, Is.EqualTo("Tester"));
                Assert.That(commit.Author.EmailAddress, Is.EqualTo("author@example.com"));

                Assert.That(commit.Committer, Is.Not.Null);
                Assert.That(commit.Committer.Name, Is.EqualTo("Other"));
                Assert.That(commit.Committer.EmailAddress, Is.EqualTo("committer@example.com"));

                Assert.That(commit.Committer.TimeOffsetInMinutes, Is.EqualTo(120), "Time offset"); // CEST dependent
                Assert.That(commit.Committer.When, Is.EqualTo(ct), "Exact time");
                Assert.That(commit.LogMessage, Is.EqualTo("A log message to remember\n"));

                Assert.That(commit.Parents, Is.Not.Empty);
                Assert.That(commit.ParentIds, Is.Not.Empty);

                Assert.That(commit.Tree, Is.Not.Empty);
                Assert.That(commit.Tree.Count, Is.EqualTo(2));
                Assert.That(commit.Ancestor.Tree.Count, Is.EqualTo(3));
                Assert.That(commit.Ancestor.Ancestor.Tree.Count, Is.EqualTo(2));
                Assert.That(commit.Tree.Id, Is.Not.EqualTo(commit.Ancestor.Tree.Id));

                GitId id;
                Assert.That(repo1.LookupViaPrefix(commit.Id.ToString(), out id));
                Assert.That(id, Is.EqualTo(commit.Id));

                Assert.That(repo1.LookupViaPrefix(commit.Id.ToString().Substring(0, 10), out id));
                Assert.That(id, Is.EqualTo(commit.Id));

                Assert.That(commit.Peel<GitObject>().Id, Is.EqualTo(commit.Tree.Id));
                Assert.That(commit.Peel<GitTree>(), Is.EqualTo(commit.Tree)); // Compares members
                Assert.That(commit.Tree.Peel<GitObject>(), Is.Null);

                GitTagArgs ta = new GitTagArgs();
                ta.Signature.When = ct;
                ta.Signature.Name = "Me";
                ta.Signature.EmailAddress = "me@myself.and.I";
                ta.LogMessage = "Some message";
                ga.Author.TimeOffsetInMinutes = 120;
                Assert.That(commit.Tag("MyTag", ta, out id));
                Assert.That(id, Is.EqualTo(new GitId("db31f8333fc64d7e7921ea91f6e007b755dcfcbb")));

                GitTag tag;
                Assert.That(repo1.Lookup(id, out tag));
                Assert.That(tag, Is.Not.Null);
                Assert.That(tag.Name, Is.EqualTo("MyTag"));
                Assert.That(tag.LogMessage, Is.EqualTo("Some message\n"));
                Assert.That(tag.Tagger.Name, Is.EqualTo("Me"));

                Assert.That(tag.Target.Id, Is.EqualTo(commit.Id));
                Assert.That(tag.Peel<GitTree>(), Is.EqualTo(commit.Peel<GitTree>()));

                repo1.CheckOut(commit.Tree);

                //Console.WriteLine("1:");
                //foreach (GitTreeEntry e in commit.Tree)
                //{
                //    Console.WriteLine(string.Format("{0}: {1} ({2})", e.Name, e.Kind, e.Children.Count));
                //}

                //Console.WriteLine("2:");
                //foreach (GitTreeEntry e in commit.Ancestor.Tree)
                //{
                //    Console.WriteLine(string.Format("{0}: {1} ({2})", e.Name, e.Kind, e.Children.Count));
                //}

                //Console.WriteLine("3:");
                //foreach (GitTreeEntry e in commit.Ancestor.Ancestor.Tree)
                //{
                //    Console.WriteLine(string.Format("{0}: {1} ({2})", e.Name, e.Kind, e.Children.Count));
                //}
                //Console.WriteLine("-");
            }
        }

        [TestMethod]
        public void CreateGitRepository()
        {
            string dir = GetTempPath();
            string file = Path.Combine(dir, "file");
            using (GitRepository repo = GitRepository.Create(dir))
            {
                Assert.That(repo, Is.Not.Null);
                Assert.That(repo.IsEmpty, Is.True);
                Assert.That(repo.IsBare, Is.False);
                Assert.That(repo.RepositoryDirectory, Is.EqualTo(Path.Combine(dir, GitClient.AdministrativeDirectoryName)));
                Assert.That(repo.WorkingCopyDirectory, Is.EqualTo(dir));

                GitConfiguration config = repo.Configuration;

                Assert.That(config, Is.Not.Null);

                GitIndex index = repo.Index;
                Assert.That(index, Is.Not.Null);
                //repo.SetIndex(index);

                GitObjectDatabase odb = repo.ObjectDatabase;
                Assert.That(odb, Is.Not.Null);
                //repo.SetObjectDatabase(odb);

                File.WriteAllText(file, "qqq");

                repo.Index.Add("file");

                Assert.That(repo.Configuration.Set(GitConfigurationLevel.Repository, GitConfigurationKeys.UserName, "Tester"));
                Assert.That(repo.Configuration.Set(GitConfigurationLevel.Repository, GitConfigurationKeys.UserEmail, "me@myself.and.i"));

                string v;
                Assert.That(repo.Configuration.TryGetString(GitConfigurationKeys.UserName, out v));
                Assert.That(v, Is.EqualTo("Tester"));

                Assert.That(repo.Configuration.TryGetString(GitConfigurationKeys.UserEmail, out v));
                Assert.That(v, Is.EqualTo("me@myself.and.i"));

                Assert.That(index.Contains("file"));
                GitIndexEntry entry = repo.Index["file"];

                Assert.That(entry, Is.Not.Null);
                Assert.That(entry.FileSize, Is.EqualTo(3));
                Assert.That(entry.Id, Is.EqualTo(new GitId("E5A49F32170B89EE4425C4AB09E70DFCDB93E174")));

                index.Reload();

                Assert.That(index.Contains("file"), Is.False);
            }
        }

        [TestMethod]
        public void StatusTest()
        {
            string[] A = new string[] { "A/a", "A/b", "A/c", "A/d" };
            string[] B = new string[] { "B/a", "B/b", "B/c", "B/d" };
            string[] C = new string[] { "C/a", "C/b", "C/c", "C/d" };

            List<string> all = new List<string>();
            all.AddRange(A);
            all.AddRange(B);
            all.AddRange(C);

            using (GitRepository repo = GitRepository.Create(GetTempPath()))
            {
                foreach (string d in all)
                {
                    string fp = Path.Combine(repo.WorkingCopyDirectory, d);
                    Directory.CreateDirectory(Path.GetDirectoryName(fp));

                    File.WriteAllText(fp, string.Format("This is {0}\n", fp));
                    repo.Index.Add(d);
                }
                repo.Index.Write();

                using (GitClient git = new GitClient())
                {
                    List<string> found = new List<string>();

                    GitStatusArgs sa = new GitStatusArgs();

                    git.Status(Path.Combine(repo.WorkingCopyDirectory, "B"),
                        delegate(object sender, GitStatusEventArgs e)
                        {
                            found.Add(e.RelativePath);
                        });

                    Assert.That(found.Count, Is.EqualTo(4));
                    Assert.That(found, Is.All.GreaterThan("B"));
                    Assert.That(found, Is.All.LessThan("C"));

                    found.Clear();
                    git.Status(Path.Combine(repo.WorkingCopyDirectory, "B/c"),
                        delegate(object sender, GitStatusEventArgs e)
                        {
                            found.Add(e.RelativePath);
                        });
                    Assert.That(found.Count, Is.EqualTo(1));
                    Assert.That(found, Is.All.Contains("B/c"));
                }
            }
        }

        [TestMethod]
        [TestCategory("NeedsNetwork")]
        public void ClonePublic()
        {
            string repos = GetTempPath();
            using (GitClient git = new GitClient())
            {
                GitCloneArgs ca = new GitCloneArgs();
                ca.Synchronous = true;

                git.Clone(new Uri("https://github.com/libgit2/TestGitRepository.git"), repos, ca);

                List<string> found = new List<string>();

                GitStatusArgs sa = new GitStatusArgs();
                sa.IncludeUnmodified = true;
                sa.UseGlobPath = true;

                git.Status(repos, sa,
                    delegate(object sender, GitStatusEventArgs e)
                    {
                        found.Add(e.RelativePath);
                    });

                Assert.That(found.Count, Is.EqualTo(8));
                Assert.That(found, Is.All.Not.Null);
                Assert.That(found, Is.Unique);

                found.Clear();

                git.Status(repos + "/a/*", sa,
                    delegate(object sender, GitStatusEventArgs e)
                    {
                        found.Add(e.RelativePath);
                    });

                Assert.That(found.Count, Is.EqualTo(3));
                Assert.That(found, Is.All.Not.Null);
                Assert.That(found, Is.Unique);

                GitFetchArgs fa = new GitFetchArgs();
                fa.All = true;
                git.Fetch(repos, fa);

                using (GitRepository repo = new GitRepository(repos))
                {
                    Assert.That(repo.HeadBranch, Is.Not.Null);

                    Assert.That(repo.HeadBranch.Name, Is.EqualTo("refs/heads/master"));
                    Assert.That(repo.HeadBranch.UpstreamReference, Is.Not.Null);
                    Assert.That(repo.HeadBranch.UpstreamReference.Name, Is.EqualTo("refs/remotes/origin/master"));
                    Assert.That(repo.HeadBranch.IsHead, "Head knows that it is head");
                    //Assert.That(repo.HeadBranch.TrackedBranch, Is.Null);

                    Assert.That(repo.HeadBranch.Name, Is.EqualTo("refs/heads/master"));
                    Assert.That(repo.HeadBranch.IsRemote, Is.False, "Local branch");
                    Assert.That(repo.HeadBranch.RemoteName, Is.EqualTo("origin"));

                    foreach (GitBranch b in repo.Branches.Remote)
                    {
                        Assert.That(b.IsRemote, "Remote branch");
                        Assert.That(b.IsLocal, Is.False, "Not local");
                        Assert.That(b.Name, Is.SubPathOf("refs/remotes/origin/"));
                        Assert.That(b.IsHead, Is.False);
                        Assert.That(b.UpstreamReference, Is.Null);
                        Assert.That(b.LocalUpstreamName, Is.Null);
                        Assert.That(b.RemoteName, Is.EqualTo("origin"));
                    }

                    foreach (GitBranch b in repo.Branches.Local)
                    {
                        Assert.That(b.IsLocal, "Local branch");
                        Assert.That(b.IsRemote, Is.False, "Not remote");
                        Assert.That(b.Name, Is.SubPathOf("refs/").Or.EqualTo("master"));
                        Assert.That(b.IsHead, Is.EqualTo(b.ShortName == "master"));
                        Assert.That(b.RemoteName, Is.EqualTo("origin"));
                        if (!b.IsHead)
                        {
                            Assert.That(b.LocalUpstreamName, Is.Not.Null);

                            GitBranch tracked = b.TrackedBranch;
                            Assert.That(tracked, Is.Not.Null, "Have tracked");

                            Assert.That(b.RemoteName, Is.Not.Null);
                        }
                    }

                    foreach (GitRemote r in repo.Remotes)
                    {
                        Assert.That(r.Name, Is.Not.Null);
                        Assert.That(r.TagSynchronize, Is.EqualTo(GitTagSynchronize.Auto));
                        //Assert.That(r.Save(new GitFetchArgs()));

                        foreach (GitRefSpec rs in r.FetchRefSpecs)
                        {

                        }

                        foreach (GitRefSpec rs in r.PushRefSpecs)
                        {

                        }
                    }
                }

                git.Pull(repos);

                // libgit2's local push code only supports bare repositories at
                // this time, so we use a few more clones to test the real push

                string cloneDir = GetTempPath();
                GitCloneArgs cca = new GitCloneArgs();
                cca.InitArgs.CreateBareRepository = true;
                git.Clone(repos, cloneDir, cca);

                string clone2Dir = GetTempPath();

                git.Clone(cloneDir, clone2Dir);

                GitPushArgs pa = new GitPushArgs();
                pa.Mode = GitPushMode.All;
                git.Push(clone2Dir, pa);
            }
        }

        [TestMethod]
        public void PushChanges()
        {
            string master = GetTempPath();
            using (GitClient git = new GitClient())
            {
                GitInitArgs ia = new GitInitArgs();
                ia.CreateBareRepository = true;
                ia.Description = "Harry & Sally root";

                git.Init(master, ia);
            }

            Uri masterUri = new Uri(master);

            string harry = GetTempPath();
            string sally = GetTempPath();

            using (GitClient git = new GitClient()) // Harry
            {
                git.Clone(masterUri, harry);

                using (GitRepository harryRepo = new GitRepository(harry))
                {
                    harryRepo.Configuration.Set(GitConfigurationLevel.Repository, "user.name", "Harry");
                    harryRepo.Configuration.Set(GitConfigurationLevel.Repository, "user.email", "harry@example.com");
                }

                string src = Path.Combine(harry, "src");
                Directory.CreateDirectory(src);

                string index = Path.Combine(src, "index.txt");

                File.WriteAllText(index, "This is index.txt\n");

                string appCode = Path.Combine(harry, "app.c");
                File.WriteAllText(appCode, @"
#include <stdio.h>

int main(int argc, const char **argv)
{
    printf(""hello world\n"");
    return 0;
}
");

                git.Add(index);
                git.Add(appCode);
                GitId result;
                GitCommitArgs cma = new GitCommitArgs();
                cma.Signature.When = new DateTime(2000, 1, 1, 0, 0, 0, DateTimeKind.Utc);
                cma.Author.When = new DateTime(2000, 1, 1, 0, 0, 0, DateTimeKind.Utc);
                git.Commit(harry, cma, out result);

                using (GitRepository harryRepo = new GitRepository(harry))
                {
                    GitCommit commit;
                    Assert.That(harryRepo.Lookup(result, out commit));

                    Assert.That(commit.Author.Name, Is.EqualTo("Harry"));
                }
            }

            using (GitClient git = new GitClient()) // Sally
            {
                git.Clone(masterUri, sally);

                using (GitRepository sallyRepo = new GitRepository(sally))
                {
                    sallyRepo.Configuration.Set(GitConfigurationLevel.Repository, "user.name", "Sally");
                    sallyRepo.Configuration.Set(GitConfigurationLevel.Repository, "user.email", "Sally@my.domain");
                }

                string src = Path.Combine(sally, "src");
                string index = Path.Combine(src, "index.txt");

                Assert.That(File.Exists(index), Is.False);
            }

            using (GitClient git = new GitClient()) // Harry
            {
                GitPushArgs pa = new GitPushArgs();
                pa.Mode = GitPushMode.All;
                git.Push(harry, pa);
            }

            using (GitClient git = new GitClient()) // Sally
            {
                GitPullArgs pa = new GitPullArgs();
                pa.FetchArgs.All = true;

                git.Pull(sally, pa);

                string src = Path.Combine(sally, "src");
                string index = Path.Combine(src, "index.txt");

                Assert.That(File.Exists(index), Is.True);

                string appCode = Path.Combine(sally, "app.c");
                File.WriteAllText(appCode, @"
#include <stdio.h>

int main(int argc, const char **argv)
{
    int i;

    if (argc != 1)
    {
        fprintf(stderr, ""Usage %s <int>\n"", argv[0]);
        return 1;
    }
    for (i = 0; i < atoi(argv[1]); i++
        printf(""hello world %d\n"", i);

    return 0;
}
");
                git.Add(appCode);

                GitId result;
                GitCommitArgs cma = new GitCommitArgs();
                cma.Signature.When = new DateTime(2000, 1, 2, 0, 0, 0, DateTimeKind.Utc);
                cma.Author.When = new DateTime(2000, 1, 2, 0, 0, 0, DateTimeKind.Utc);
                git.Commit(sally, cma, out result);

                GitPushArgs ph = new GitPushArgs();
                ph.Mode = GitPushMode.All;
                git.Push(sally, ph);
            }

            using (GitClient git = new GitClient()) // Harry
            {
                string appCode = Path.Combine(harry, "app.c");
                File.WriteAllText(appCode, @"
#include <stdio.h>

int main(int argc, const char **argv)
{
    if (argc > 0 && strcmp(argv[1], ""-V"")
    {
        printf(""%s version 1.0 (c) QQn\n"");
        return 0;
    }
    printf(""hello world\n"");
    return 0;
}
");

                git.Add(appCode);

                GitId result;
                GitCommitArgs cma = new GitCommitArgs();
                cma.Signature.When = new DateTime(2000, 1, 3, 0, 0, 0, DateTimeKind.Utc);
                cma.Author.When = new DateTime(2000, 1, 3, 0, 0, 0, DateTimeKind.Utc);
                git.Commit(harry, cma, out result); // Local commit will succeed

                GitPushArgs ph = new GitPushArgs();
                ph.Mode = GitPushMode.All;
                try
                {
                    git.Push(harry, ph); // But push fails, as it conflicts
                    Assert.Fail("Should have failed"); // ###
                }
                catch(GitException ge)
                {
                    Assert.That(ge.Message, Contains.Substring(/*C*/"annot push"));
                }

                GitPullArgs pa = new GitPullArgs();
                pa.FetchArgs.All = true;

                git.Pull(harry, pa);

                bool gotConflict = false;
                git.Status(harry,
                    delegate(object sender, GitStatusEventArgs e)
                    {
                        switch (e.RelativePath)
                        {
                            case "app.c":
                                gotConflict = true;
                                Assert.That(e.WorkingDirectoryStatus, Is.EqualTo(GitStatus.Normal));
                                Assert.That(e.IndexStatus, Is.EqualTo(GitStatus.Normal));
                                Assert.That(e.Conflicted, Is.True, "Conflicted");
                                break;
                            default:
                                Assert.Fail("Unexpected path: {0}", e.RelativePath);
                                break;
                        }
                    });

                Assert.That(gotConflict, "Found conflict status");

                try
                {
                    git.Push(harry, ph); // But push fails, as it conflicts
                    Assert.Fail("Should still fail");
                }
                catch (GitException ge)
                {
                    Assert.That(ge.Message, Contains.Substring(/*C*/"annot push"));
                }

                GitResetArgs ra = new GitResetArgs();
                ra.Mode = GitResetMode.Hard;
                git.Reset(harry, ra);
            }
        }

        [TestMethod]
        public void DualStart()
        {
            string master = GetTempPath();
            using (GitClient git = new GitClient())
            {
                GitInitArgs ia = new GitInitArgs();
                ia.CreateBareRepository = true;
                ia.Description = "Harry & Sally root";

                git.Init(master, ia);
            }

            Uri masterUri = new Uri(master);

            string harry = GetTempPath();
            string sally = GetTempPath();

            using (GitClient git = new GitClient()) // Harry
            {
                git.Clone(masterUri, harry);

                using (GitRepository harryRepo = new GitRepository(harry))
                {
                    harryRepo.Configuration.Set(GitConfigurationLevel.Repository, "user.name", "Harry");
                    harryRepo.Configuration.Set(GitConfigurationLevel.Repository, "user.email", "harry@example.com");
                }

                string src = Path.Combine(harry, "src");
                Directory.CreateDirectory(src);

                string index = Path.Combine(src, "index.txt");

                File.WriteAllText(index, "This is index.txt\n");

                git.Add(index);
                GitId result;
                GitCommitArgs cma = new GitCommitArgs();
                cma.Signature.When = new DateTime(2000, 1, 1, 0, 0, 0, DateTimeKind.Utc);
                cma.Author.When = new DateTime(2000, 1, 1, 0, 0, 0, DateTimeKind.Utc);
                git.Commit(harry, cma, out result);

                using (GitRepository harryRepo = new GitRepository(harry))
                {
                    GitCommit commit;
                    Assert.That(harryRepo.Lookup(result, out commit));

                    Assert.That(commit.Author.Name, Is.EqualTo("Harry"));
                }
            }

            using (GitClient git = new GitClient()) // Sally
            {
                git.Clone(masterUri, sally);

                using (GitRepository sallyRepo = new GitRepository(sally))
                {
                    sallyRepo.Configuration.Set(GitConfigurationLevel.Repository, "user.name", "Sally");
                    sallyRepo.Configuration.Set(GitConfigurationLevel.Repository, "user.email", "Sally@my.domain");
                }

                string iota = Path.Combine(sally, "iota.txt");

                File.WriteAllText(iota, "This is iota\n");

                git.Stage(iota);

                GitCommitArgs cma = new GitCommitArgs();
                cma.Signature.When = new DateTime(2001, 1, 1, 0, 0, 0, DateTimeKind.Utc);
                cma.Author.When = new DateTime(2001, 1, 1, 0, 0, 0, DateTimeKind.Utc);
                git.Commit(sally, cma);

                string src = Path.Combine(sally, "src");
                string index = Path.Combine(src, "index.txt");

                Assert.That(File.Exists(index), Is.False);
            }

            using (GitClient git = new GitClient()) // Harry
            {
                GitPushArgs pa = new GitPushArgs();
                pa.Mode = GitPushMode.All;
                git.Push(harry, pa);
            }

            using (GitClient git = new GitClient()) // Sally
            {
                GitPullArgs pa = new GitPullArgs();
                pa.FetchArgs.All = true;

                git.Pull(sally, pa);

                string src = Path.Combine(sally, "src");
                string index = Path.Combine(src, "index.txt");

                Assert.That(File.Exists(index), Is.True);
            }
        }
    }
}
