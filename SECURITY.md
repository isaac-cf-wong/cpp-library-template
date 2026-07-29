# Security Policy

## Supported versions

<!-- prettier-ignore-start -->

| Version  | Supported          | Notes                                     |
| -------- | ------------------ | ----------------------------------------- |
| Latest   | :white_check_mark: | Fixes land on `main` and in a new release |
| `< 1.0`  | :x:                | Pre-1.0; upgrade to the latest release    |

<!-- prettier-ignore-end -->

Update this table when you reach 1.0 and start maintaining release branches.

## Reporting a vulnerability

**Do not open a public issue for a security problem.**

Use GitHub's private vulnerability reporting:

<!-- prettier-ignore-start -->

1. Go to
   <https://github.com/isaac-cf-wong/cpp-library-template/security>
2. Click **Report a vulnerability**
3. Describe the issue and how to reproduce it

<!-- prettier-ignore-end -->

Please include:

- The affected version, or the commit hash
- Your compiler, its version, and the standard library implementation
  (libstdc++, libc++, MSVC STL)
- Your operating system and architecture
- Whether the build was static or shared, and which sanitizers, if any, were
  enabled
- A minimal reproducer: the smallest program and build command that shows the
  problem
- The impact as you see it, and any suggested fix

For memory-safety reports, a sanitizer log (ASan, UBSan, TSan or MSan) or a
crash backtrace is far more useful than a description.

## What to expect

- **Acknowledgement** within 24 hours.
- **An initial assessment** within 3 to 5 business days, saying whether we can
  reproduce it and how severe we think it is.
- **Updates** as the fix progresses.
- **A release** containing the fix, with a security advisory.

## Responsible disclosure

Please give us a chance to release a fix before disclosing publicly. We will
credit you in the advisory unless you would rather stay anonymous.

## Thanks

We are grateful to everyone who reports problems responsibly.
