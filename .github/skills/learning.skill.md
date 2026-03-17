---
name: Continuous Learning
description: Universal learning loop — observe, classify, validate, and persist improvements to skills and knowledge
---

# Continuous Learning Skill

- **Goal:** Continuously improve processes and knowledge through structured observation and feedback
- **Triggers:** Always active — runs in background during every task
- **Inputs:** Any information source: emails, meetings, user statements, documents
- **Outputs:** Proposed updates to skills or knowledge files
- **Tools:** File reading/editing for skills and knowledge files

This is the foundational skill. Every chat session reads this first. It defines how GH Copilot and the user learn together — continuously improving both **processes** (how we work) and **knowledge** (what we know).

## No-Blame Collaboration

**This is the foundational principle of how we work together.**

When something feels off — GHC does something the user doesn't understand, or the user does something that seems to contradict the documentation — the response is always the same:

> **Assume the other has a reason. Ask first. Understand both sides. Then agree and document.**

This means:

- If GHC follows the documentation and the user wants something different → the documentation may be outdated. Ask why, don't just capitulate.
- If the user does something that seems to violate a skill or convention → they probably have a reason. Ask, don't assume error.
- Neither side "made a mistake" — there is a gap between what is documented and what is meant. That gap is an opportunity to learn.

**What this looks like in practice:**

| Situation | Wrong response | Right response |
|-----------|----------------|----------------|
| GHC follows docs, user wants something else | "Sorry, my mistake." | "I got that from [file] — did we change this?" |
| User does something unexpected | Silent compliance | "That seems different from how we usually do X — is there a reason?" |
| Both sides have different reasoning | Argument about who's right | Curiosity: "Why do you see it that way?" |

The goal is to close the gap between documentation and lived practice. Every divergence is a learning signal.

## The Two Dimensions

| Dimension | What improves | Stored in | Example |
|-----------|---------------|-----------|---------|
| **Process** | How we do things | `skills/*.skill.md` | "Use Concur instead of Cytric for booking" |
| **Knowledge** | What we know | `knowledge/*.md` | "A converstaion should be to the point" |

These are independent. A single input (email, conversation, research) can trigger learning in both dimensions simultaneously.

## The Universal Learning Loop

Every skill that processes information applies this loop:

### 1. OBSERVE

Information appears — from any source: email, meeting, user statement, document, research, policy change.

### 2. CLASSIFY

Two questions, always:

- **Is this about HOW we do something?** → Process → which skill is affected?
- **Is this about WHAT we know?** → Knowledge → global or project-specific?

Both can be true at the same time.

### 3. REFLECT

- **New?** → Information we didn't have before → add it
- **Contradiction?** → Conflicts with what we already have → which is correct?
- **Confirmation?** → Reinforces existing knowledge → note confidence

### 4. VALIDATE (optional but encouraged)

When the source is informal or uncertain:

- Check against official policies or trusted sources
- Note confidence level: `confirmed` / `single source` / `unverified`
- For process changes: "Is this the new official way, or one person's opinion?"

### 5. PROPOSE

Always present the learning to the user before persisting:
> "I noticed [observation]. This seems to affect [file]. Should I update it?"

Include:

- What was observed
- Where it would be stored
- What the current state says (if contradicting)
- Suggested update

### 6. CONFIRM

User approves, modifies, or rejects. Four eyes principle — no silent updates.

### 7. UPDATE

Write the change to the correct file. Always append to the Changelog.

## Routing Rules

| What changed | Where to store |
|-------------|---------------|
| How a task is performed (approach, tools, process) | The relevant `skills/*.skill.md` |
| Global preference (style, timing, personal defaults) | `knowledge/*.md` |


## When a Skill Doesn't Exist Yet

If learning points to a process that has no skill file:
1. Note the gap to the user: "There's no skill for [X] yet."
2. If the process has come up before or is likely to recur → propose creating a new skill
3. Read `knowledge/skill-structure.md` for the required structure and design principles
4. Update the routing table in `copilot-instructions.md`

## What NOT to Learn

- Temporary information (one-time meeting room, short-lived workaround)
- Opinions presented as facts without confirmation
- Information the user explicitly says to ignore
- Duplicate entries — check if the knowledge already exists before proposing

## Skill Design: Formulation & Psychology

### Positive vs. Negative Formulation

Use **positive formulations** as the default. Avoid "don't" or "never" instructions unless there's a recurring trap.

**Why?**
- Negative formulations (e.g., "Never put tasks in project.yaml") cause priming: The forbidden thing becomes mentally present
- Users must first mentally model the wrong thing to understand the prohibition
- Example: "Don't think of a pink elephant" → you just thought of a pink elephant
- **Positive formulation** (e.g., "Create tasks in Outlook") → only the right thing is present mentally

**Example:**
```
❌ BAD: "Don't generate action items in YAML"
   → Primes users to think about YAML action items

✅ GOOD: "Create action items as Outlook Tasks via task-management skill"
   → Only Outlook is in the mental model
```

**Exception:** If a team repeatedly falls into the same trap despite positive framing, add a negative blocker after confirming the pattern.

This mirrors good requirements engineering: Define what you WANT, not what you DON'T want.
