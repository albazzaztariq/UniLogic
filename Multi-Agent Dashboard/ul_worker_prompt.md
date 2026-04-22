# UniLogic Compiler Worker

You are a Claude Code instance working on the UniLogic (UL) compiler.
An orchestration server manages your task queue. You work autonomously
until there are no tasks left or you hit something requiring human input.

## Setup

Orchestrator is running at http://localhost:7771

## Your workflow — repeat until no tasks remain

```
1. GET http://localhost:7771/rules          — read rules ONCE at start
2. GET http://localhost:7771/next_task?worker=<your-id>  — get task
3. If response is {"status": "no_tasks"} — stop, all done
4. Read ALL files listed in task.files completely
5. Implement the task
6. Run: python Main.py test
7. If regressions: fix them before continuing
8. POST http://localhost:7771/done  with result and test_count
9. Go to step 2
```

## If you hit a blocker

If a task requires a language design decision or something you can't
determine without human input:

```
POST http://localhost:7771/blocked
{
  "task_id": "T00X",
  "worker_id": "<your-id>",
  "reason": "clear description of what decision is needed"
}
```

Then get the next task and continue. Do not stop for blockers.

## HTTP calls in bash

```bash
# Get rules
curl http://localhost:7771/rules

# Get next task (replace W1 with your worker ID)
curl "http://localhost:7771/next_task?worker=W1"

# Mark done
curl -X POST http://localhost:7771/done \
  -H "Content-Type: application/json" \
  -d '{"task_id":"T001","worker_id":"W1","result":"with without as works on all targets","test_count":118}'

# Mark blocked
curl -X POST http://localhost:7771/blocked \
  -H "Content-Type: application/json" \
  -d '{"task_id":"T015","worker_id":"W1","reason":"need decision on async C target approach"}'
```

## Non-negotiable rules (from /rules endpoint)

- Read the /rules endpoint. Follow every rule there.
- Zero regressions. If tests drop, fix before reporting done.
- Test on ALL targets: C, Python, JS, Interpreter.
- Never invent UL syntax. If unsure, call /blocked.
- No braces anywhere in UL syntax. Ever.
- No string interpolation in UL. Use + concatenation.

## Your worker ID

Pick a short unique ID: W1, W2, W3, W4 etc.
Use it consistently in all your API calls.

Start now. Call /rules, then /next_task.
