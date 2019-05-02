# Contributing

**Please carefully read those guidelines before contributing**

You are expected to **respect** and **enforce** all the rules listed here. Please do so kindly, respectfully, without making a fuss.

Those rules are not meant to be definitive. If you feel uncomfortable with some of them, please consider asking for a change. In the meantime, please continue respecting and enforcing them in their current state while providing construcive feedback.

Remember: the main reason for a style guide, is to provide your fellow developpers with a consistent experience. Consistency improves readability. This is why typographic rules are enforced in books, newspapers and magazines. You are expected to take those rules seriously even if they seem spurious to you, or even moronic. Good code is code that is not only understandable by a machine, but also by your fellow human beings, and consistency is one of the key factor in readability.

In short:

* Be respectful
* Be consistent
* Be smart
* Be COOL :)


## Committing rules

1. You shall not commit directly to master. Open a pull request first. You might want to check out the [Hub project](https://github.com/github/hub) to open pull requests from the command line.

    To do so, you must create a branch. The branch name:

    * MUST respect the format `<changeset-type>/<changeset-description>`
    * `changeset-type` MUST be either be `feature`, `fix` or `enhancement` (singular every time)
    * `changeset-description` MUST be written in `lower-snake-case` and should not exceed 20 characters
    * SHOULD express the intent of the changes, not the technical details

2. You shall write properly formatted commit messages, in English. The format is as follows:
        Short commit message
        [blank line]
        Long commit message (optional)
    1. Short commit messages:
        * MUST not exceed the canonical 50 characters length
        * MUST be written in imperative mode : "Add something", not "Added something" or "Adding something"
        * MUST begin by a capital letter
        * MUST not end with a dot (`.`)
        * SHOULD express the intent of the changes, not its gritty technical details
        * CAN use github markdown styling
    2. Long commit messages:
      * Are optional
      * MUST not exceed a 72 characters line-length
      * MUST be written using markdown syntax for links, lists, bold, and other fancy formatting
      * CAN and SHOULD be a bit more involved than the short commit message, explaining technical details as needed

3. If 1. and 2. were followed, you are allowed to open a pull request, which begins the code review process.

## Tagging rules

If you want to tag a release, please respect the following:

* Tag name MUST be in the format `v${VERSION_NUMBER}`, e.g. `v0.1.0`
* `VERSION_NUMBER` MUST follow the [semver specifications](https://semver.org/), e.g `0.1.0`

Instructions for tagging with git:

* Do not create an annotated tag
* Be careful before pushing (i.e don't push non-conforming tags)
* Use the following commands
        git tag v0.1.1
        git push --tags

## Code style guide

### General

* Respect the LLVM style guide (https://llvm.org/docs/CodingStandards.html)

* The file format is UNIX. Lines are terminated by a single `\n` character. If you are running windows (which terminate lines with `\r\n`), make sure to configure your editor with UNIX style end of lines.

* The indent rules are: two spaces, no tabs.

* Avoid comments. Comments usually means your code could be more expressive all by itself. So, if you feel you need comments:
  
  1. Try naming your variables, functions, components to express your intent more clearly.
  2. Try extracting your code into smaller variables, functions, components, that express their intent clearly.
  3. If you still feel your code could benefit a comment, you must be in one of those two cases:
    * you are writing a very obscure algorithm
    * you are interacting with a very obscure API
    In those case, your comment should express _what_ and _why_ you are trying to achieve, not _how_ your are achieving it.


