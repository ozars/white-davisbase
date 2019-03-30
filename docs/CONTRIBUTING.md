# Contributing

**Note: white-davisbase is a class project for the Univesity of Texas at Dallas
CS 6360 course. So, the main audience of this document is currently group
members.**

## Git Workflow

The project will be managed using [GitHub Standard Fork & Pull Request
Workflow][git workflow]. Some notes and exceptions:

- Make sure you [set up your SSH keys for GitHub][ssh keys] if you haven't
done already to be able to push your own repo.
- Use interactive rebase to merge changes from upstream/master so that git
history doesn't get bloated with merge commits. More on interactive rebasing is
below.

Main branch is currently `ozars/master`. All features will be
merged to this branch. It is a [protected branch], meaning all other features
will be merged through GitHub Pull Requests after they pass checks. List of
checks currently active:

- TravisCI: Runs tests and checks code formatting.
- Codecov: Tracks changes in coverage of tests.
- Codacy: Runs linters to ensure good practices.

Since there may be multiple features being worked on concurrently by different
people, it is likely that your branch needs to be rebased on top of up-to-date
upstream master. [Interactive rebase] is particularly useful in these
circumstances. It is also useful in tidying up git history before merge.

Lastly, please follow [general good practices][git message] in git commit
messages for well-being of other fellow developers and future self.

[git workflow]: ./GitHub-Forking.md
[ssh keys]: https://help.github.com/en/articles/adding-a-new-ssh-key-to-your-github-account
[protected branch]: https://help.github.com/en/articles/about-protected-branches
[Interactive rebase]: https://thoughtbot.com/blog/git-interactive-rebase-squash-amend-rewriting-history
[git message]: https://chris.beams.io/posts/git-commit/

## Communication Channels

Use GitHub issues and pull requests wherever possible. Use good old WhatsApp
group chat and phone calls for more interactive and informal communication.

## Coding Style and Editor Configuration

Coding conventions are based on [Mozilla's style] with minor changes. If you'd
like to see what's different, you can type:

```shell
diff --color .clang-format <(clang-format -style=mozilla -dump-config)
```

in the project root. However this should not be necessary if you use automated
tools for formatting. So, it is highly recommended using an editor:

- Utilizing `editor-config`, which takes care of basic editor settings like tab
  spacing and format.
- Utilizing `clang-format`, which takes care of C++ related formatting
  conventions. Make sure you have `clang-format` installed if your editor or
  editor plugin requires (usually they do).

Alternatively, `clang-format` comes with a git extension enabling `git
clang-format` command, which formats the code in the working directory.

If you realize `clang-format` makes some portion of your code look ugly, you
can turn-off formatting temporarily for that portion of your code as in [this
example][ugly format].

[Mozilla's style]: https://developer.mozilla.org/en-US/docs/Mozilla/Developer_guide/Coding_Style
[ugly format]: https://github.com/ozars/white-davisbase/blob/84e1a5/src/parser.cpp#L91-L98

## Extra Testing Configuration

By default a testing binary named `tests` will be placed in `bin` directory
after build. CTest is actually frontend to this binary for running tests.
However, this binary is useful in accessing Catch's command line interface,
which CTest doesn't offer. Some useful commands for example:

```
bin/tests -h         # Prints help
bin/tests -s         # Print output for successful tests
bin/tests -t         # List available tags
bin/tests '[parser]' # Run tests tagged as [parser]
```

See [Catch's documentation] for more details.

[Catch's documentation]: https://github.com/catchorg/Catch2/blob/v2.7.0/docs/Readme.md

## And of course...

Don't forget to most important part of the project: **Having fun!** 🕺🎉💃
