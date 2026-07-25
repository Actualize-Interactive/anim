# Security Policy

## Supported versions

`anim` is pre-1.0 and is developed on a single line. Security fixes are applied
to the latest release only; there are no maintained backport branches.

| Version | Supported |
| --- | --- |
| 0.2.x | ✅ |
| < 0.2 | ❌ |

## Reporting a vulnerability

**Please do not report security issues through public GitHub issues.**

Report privately through GitHub's
[private vulnerability reporting](https://github.com/Actualize-Interactive/anim/security/advisories/new).
The form is the only reporting channel; it lets us discuss and fix the issue
with you before anything becomes public, and it requires nothing more than a
GitHub account.

Please include:

- the affected version or commit,
- a description of the issue and its impact,
- the steps, input, or minimal program needed to reproduce it,
- and any suggested fix, if you have one.

You can expect an acknowledgement within a few business days. We will keep you
informed as we investigate, and will credit you in the release notes when the
fix ships unless you prefer otherwise.

## Scope

`anim` is a library with no network, filesystem, or process boundary of its
own — it evaluates animation curves from data supplied by the calling
application. The issues most relevant here are memory-safety problems reachable
from library inputs, such as:

- out-of-bounds reads or writes from keyframe, channel, or index arguments,
- crashes, unbounded loops, or excessive allocation triggered by unusual but
  legitimate input (extreme times, NaN or infinite values, degenerate handles),
- undefined behavior surfaced by the sanitizers or by checked iterators.

Because the library trusts its caller by design, a report that depends on the
application passing deliberately corrupt internal state is likely to be treated
as a normal bug rather than a vulnerability. Report it as a regular issue and we
will still fix it.
