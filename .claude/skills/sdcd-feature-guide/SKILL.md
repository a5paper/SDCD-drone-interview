---
name: sdcd-feature-guide
description: Guides a developer who wants to add or change a feature in the SDCD-for-interview repo (drone simulator, ground control station, protocols, detection/tracking). It explains the code, decisions and tradeoffs the feature touches, warns when the feature conflicts with an existing tradeoff, assumption or rejected idea, lists what it would change, and asks for confirmation through question boxes before any code is written. Use it whenever someone says "add", "implement", "change", "support", "make it", "can we", "I want the drone/GCS to...", or proposes a new behaviour, setting, message field, protocol, UI element or dependency in this repo, even if they do not ask for guidance.
---

# SDCD feature guide

The developer must defend every change in an interview, so a feature that
quietly contradicts an earlier decision is worse than none. Load
`sdcd-codebase` first. Ask every question with AskUserQuestion: 2-4 options,
recommended first, each with what it gives and costs; look up what the code
answers instead of asking.

1. **Restate** the feature in one or two sentences; ask only what you cannot
   infer.
2. **Explore**: read every file, message, constant and setting it touches.
   Show a table File | What it does now | What would change, and name the
   decision IDs (`references/decisions.md`) that shaped it.
3. **Check conflicts** against every A/T/W/D row, the rejected list, the
   known gaps, the conventions and the assignment scope
   (`references/assignment.md`). Report:
   ```
   Feature: <one line>
   Touches: <files / messages / settings / docs>
   Conflicts: <ID> <decision>: <how> -> <what would change>
   Scope: <graded / optional / out of scope while items are open>
   ```
   No conflicts: say so; never invent one.
4. **Confirm each conflict** with a question box: proceed and update the
   decision, adjust the feature, postpone to "Later", or cancel. A rejected
   idea needs an explicit "I changed my mind".
5. **Decide the design** one question at a time; smallest design that works.
   UI change: wireframe first.
6. **Last call**: ask "Is there anything else you would like to say before I
   continue?" and wait for go-ahead.
7. **Implement** to the `sdcd-codebase` conventions; update README,
   decisions, status (after a real test) and measured numbers. Report what was
   and was not verified. Do not commit.
