# CLAUDE.md

A modern, cross-platform desktop frontend for nmap. Built in C++/Qt.
Goal: everything Zenmap should be but isn't — clean, fast, genuinely open source,
runs identically on **Windows and Linux**.

This file is the contract. Follow it on every change. When something here conflicts
with what seems clever or fast, this file wins.

---

## 1. Core philosophy — anti-vibe-code

The single rule: **write the simplest code that solves the actual problem, and
write it so a human can read it without you explaining it.** No cleverness, no
guessing, no code you don't understand.

Concretely:

- **Think first, then write.** Before touching a file, state in one or two plain
  sentences what the change does and why. If you can't explain the approach
  simply, you don't understand it yet — stop and reason it out.
- **The boring solution wins.** A plain `for` loop beats a clever one-liner. An
  `if` beats a template trick. Reach for the obvious version first, every time.
- **No abstraction without three real cases.** No interfaces, factories,
  managers, or base classes "for later." Build for what exists now. YAGNI is law.
- **One function, one job.** A function fits on one screen and does one thing. If
  you can't give it a clear name, it's doing too much — split it.
- **No dependency you can avoid.** Qt and the C++ standard library first. Adding
  a third-party library requires a real reason and must be flagged, not slipped in.
- **Explicit over magic.** No hidden control flow, no macro tricks, no template
  metaprogramming, no operator overloading surprises. The reader should never
  have to guess what runs.
- **Comments explain *why*, never *what*.** The code already says what. A comment
  justifies a non-obvious decision or warns about a trap.
- **Errors are loud and handled where they happen.** No empty catch blocks, no
  swallowed errors, no returning a silent default on failure. If nmap isn't
  found, the user sees exactly that.
- **Delete what you don't need.** Dead code, commented-out blocks, and unused
  parameters are bugs. Remove them.
- **Never write code you can't explain line by line.** If you copied a pattern
  you don't fully understand, don't use it — work it out or ask.

If a change makes the codebase harder to read, it's wrong even if it works.

---

## 2. How you (Claude Code) must work

- **Plan before coding.** For any non-trivial change, list the files you'll touch
  and the approach in a few lines *before* writing. Wait for confirmation on
  anything that adds a feature or changes architecture.
- **Small, single-purpose changes.** One logical change per step. Keep diffs
  small enough to review in one sitting.
- **Do only what was asked.** No bonus features, no "while I was in here" edits,
  no refactoring unrelated code. Scope creep is a failure, not a favor.
- **Ask instead of guessing.** If a design decision is genuinely open (naming,
  UX, where a responsibility lives), ask one clear question rather than building
  a large thing in the wrong direction.
- **Don't invent APIs.** If you're unsure a Qt method or signature exists, say so
  and check — never write a plausible-looking call from memory and hope.
- **Match the existing style.** Follow the formatting and naming already in the
  file. Don't reformat code you didn't need to change.
- **State how you verified.** After a change, say whether it compiles and what you
  checked. "It should work" is not verification.

---

## 3. Fixed technical decisions

These are settled. Don't relitigate them mid-task.

- **Language:** C++17. **Toolkit:** Qt6 **Widgets** (not QML/Qt Quick).
- **Build:** **CMake** only. Never qmake, never a hand-written Makefile.
- **Running nmap:** launch via `QProcess`. Write results to a temp file with
  `-oX`, and parse it only after the `finished` signal, using `QXmlStreamReader`.
  For live output, buffer stdout in a `QByteArray` and split on newlines.
- **Results view:** `QTableView` backed by a **custom `QAbstractTableModel`**
  (never `QTableWidget`). Sorting and filtering via `QSortFilterProxyModel`.
- **Persistence:** window/splitter/settings state via `QSettings`.
- **Command transparency:** always display the exact nmap command being run, and
  keep it editable. Never hide what the tool does.
- **License:** GPL-3.0. **Never copy any code from Zenmap** (its license forbids
  it). We only consume nmap's documented XML output — that's fine and intended.
- **Naming:** nmap is a trademark. Nothing with `nmap` or `-map` in the app name.

---

## 4. Cross-platform rules (Windows + Linux)

The C++/Qt code is ~95% identical on both. The differences live in two places
only: finding nmap, and privileges. Keep them there.

- **No hardcoded paths.** Find the nmap binary in this order: user override from
  `QSettings` → `QStandardPaths::findExecutable("nmap")` → known per-OS fallback
  locations. If none found, tell the user and let them pick the path.
- **Use Qt's path APIs** — `QDir`, `QFileInfo`, `QStandardPaths`. Never build
  paths with raw `/` or `\` or assume a separator.
- **Platform-specific code only behind `#ifdef Q_OS_WIN` / `Q_OS_LINUX`,** and
  only inside small, isolated helper functions. No `#ifdef` scattered through UI
  or logic code.
- **Privileges:** SYN scan (`-sS`), OS detection, and some techniques need root
  (Linux) or Administrator + Npcap (Windows). Detect whether the app has the
  needed rights; if not, grey out those options with a clear explanation. Do
  **not** try to auto-elevate in v1 — just inform the user.
- **CI must prove both.** A GitHub Actions matrix (`windows-latest` +
  `ubuntu-latest`) has to build successfully before anything merges. If a change
  can't build on both, it isn't done.

---

## 5. v1 scope — keep it tight

**In scope (build these, polish them, stop):**
- Target input (host, range, CIDR)
- Option builder for the common, useful flags — not all ~150
- Run a scan with live output
- Parsed host/port table: filterable, sortable, colour-coded by port state
- Save and load a scan (its XML)
- Diff two scans (what ports/hosts changed)
- Settings: nmap path, light/dark theme

**Out of scope (v2+, do not build now even if easy):**
- Topology / network graph view
- NSE script manager UI
- Scheduled / recurring scans
- PDF or fancy report export
- Remote / distributed scanning

A small, polished v1 attracts contributors. A big half-finished one repels them.

---

## 6. Definition of done

A change is finished only when all of these hold:

1. It compiles cleanly on **both** Windows and Linux, with no new warnings.
2. It's the simplest version that satisfies the request.
3. You can explain every line you wrote.
4. No unrelated files were touched.
5. If the UI changed, the README screenshot/GIF is updated to match.
