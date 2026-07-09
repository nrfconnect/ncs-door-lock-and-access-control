# Generate Commit Message

Generate a git commit message for the currently **staged** files, matching this
repository's conventions and passing the Zephyr `gitlint` compliance check used
in `.github/workflows/compliance.yaml`.

## Steps

1. Determine what to commit:
   - Run `git diff --cached --stat` to check for staged changes.
   - **If there are staged changes**, use only those.
   - **If nothing is staged**, interactively ask the user which files to add:
     - Run `git status --porcelain` to list all changed and untracked files.
       Expand any untracked directories (e.g. run `git status --porcelain
       --untracked-files=all`) so each individual file is listed separately,
       not just the parent folder.
     - Present the files as a **multi-select checkbox pop-up** using the
       `AskQuestion` tool with `allow_multiple: true`, so each file appears as
       its own option with a tick/checkbox next to it that the user can toggle.
       Do **not** just print the list as text.
       - Prefix each option label with its git status (e.g. `M `, `A `, `??`,
         `D `, `R `) so the user can see what kind of change each file is.
       - Consider adding an explicit **"Select all"** option at the top for
         convenience.
     - After the user selects, `git add` exactly the chosen files and continue.
     - If the user selects nothing, stop and tell the user that nothing was
       staged, so nothing will be committed.
   - If there are no changes at all (working tree clean), stop and tell the
     user there is nothing to commit.
2. Inspect the changes to be committed:
   - Run `git diff --cached --stat` for an overview.
   - Run `git diff --cached` to understand what actually changed.
3. Determine the author identity for the sign-off line:
   - Run `git config user.name` and `git config user.email`.
4. Look at recent history for tone/subsystem prefixes if unsure:
   - `git log --no-merges -n 20 --format='%s'`
5. Write the commit message following the **Format** and **Rules** below.
6. Present the message inside a single ```` ```text ```` block so the user can
   copy it. Then ask the user what to do using a **clickable multiple-choice
   prompt** (the `AskQuestion` tool) — do **not** require free-form typing just
   to confirm. Offer these options:
   - **Commit** — commit with the message as shown.
   - **Edit message** — the user wants to change the message.
   - **Cancel** — abort, commit nothing.

   Handle the choice as follows:
   - **Commit**: run the commit with the approved message:

     ```bash
     git commit -m "<title>" -m "<body>" -m "Signed-off-by: <Name> <email>"
     ```

     then show `git status` and `git log -1 --oneline` to confirm.
   - **Edit message**: ask the user (free-form) what they'd like to change,
     apply the changes, then return to the start of this step — re-display the
     revised message and re-prompt with the same **Commit / Edit message /
     Cancel** options. Repeat this loop as many times as the user wants, so the
     user can **always** re-edit before committing.
   - **Cancel**: do nothing and confirm nothing was committed.

   Only run the commit after the user explicitly selects **Commit**.

## Format

```text
<subsystem>: <Short capitalized subject>

<Body: what changed and why. Wrap lines at 72 (hard max 120).
Use "- " bullets for multiple distinct changes, as seen in history.>

Signed-off-by: <Full Name> <email>
```

Example (matches existing history):

```text
uwb: Move localization zone logs

Move logs.

Signed-off-by: Adrian Gielniewski <adrian.gielniewski@nordicsemi.no>
```

## Rules (must pass gitlint)

The rules below are reproduced in full here on purpose: the message **must** be
crafted to satisfy them directly, **without running any external script**. In
CI these are enforced by `check_compliance.py -m gitlint`, but that script is
only present after `west update` and may be unavailable locally — do **not**
depend on it. Always apply the rules from this document instead.

- **Title prefix**: title must be `area: Subject` and must match
  `^(?!subsys:)(?!treewide:)(([^:]+):)(\s([^:]+):)*\s(.+)$`.
  - Pick the subsystem/area from the changed paths (e.g. `uwb`, `doc`, `ci`,
    `manifest`, `matter_access`, `access_manager`, `tests`, `aliro`, ...).
    Prefer prefixes already used in `git log`.
  - The title must **not** start with the literal words `subsys:` or `treewide:`.
  - Nested prefixes like `subsys: uwb: Subject` are allowed (only the *literal*
    leading `subsys:`/`treewide:` is rejected).
- **Title length**: ≤ 120 characters (aim for ≤ 72). No trailing whitespace.
- **Title must not contain the word** `wip`.
- **Blank line** required between title and body.
- **Body is required**: at least 1 non-empty line that is not the
  `Signed-off-by` line. Never produce a body-less commit.
- **Body line length**: ≤ 120 characters per line (URLs and `Signed-off-by`/
  `Co-authored-by` lines are exempt). No trailing whitespace.
- **No blocked tags**: do not include a `Change-Id:` line.
- **Sign-off required**: last line must be
  `Signed-off-by: <First> <Last> <email>` — the name must be a real full name
  (at least two words), taken from `git config user.name` /
  `git config user.email`. If `user.name` is not a full name, ask the user for
  their full name rather than guessing.

## Style guidance (from this repo's history)

- Subjects are concise and start with a capital letter, imperative or
  descriptive ("Add ...", "Fix ...", "Move ...", "Remove ...", "Improve ...").
- Bodies often use `- ` bullet lists when there are several logical changes,
  and a single sentence for small changes.
- Keep the body focused on *what* and *why*, not a line-by-line restatement of
  the diff.
