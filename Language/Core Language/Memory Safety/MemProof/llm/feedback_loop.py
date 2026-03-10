"""
feedback_loop.py — iterative LLM + Frama-C/WP annotation refinement.

The loop:
  1. LLM generates ACSL annotations for the C function.
  2. Frama-C/WP attempts to prove all proof obligations.
  3. If any fail, the failure messages are fed back to the LLM.
  4. LLM revises the annotations.
  5. Repeat up to MAX_ROUNDS.

This loop works because WP's failure output is structured and specific:
  "function foo: goal wp_xxx: Timeout" or
  "function foo: goal rte_xxx: Unknown (Postcondition p_1 failed)"
The LLM can act on these messages without hallucinating.
"""

from .annotation_gen  import generate_annotations, refine_annotations
from ..tools.framac_runner import run_framac, FramaCResult

MAX_ROUNDS = 5


def annotate_and_verify(function_source: str,
                        c_file_context: str = '') -> tuple[str | None, FramaCResult | None]:
    """
    Run the full annotation + verification loop.

    Parameters
    ----------
    function_source  : The C function text to annotate (extracted from the file).
    c_file_context   : Full C file content (for Frama-C to analyse in context).

    Returns
    -------
    (final_annotated_source, final_framac_result)
    Both are None if no successful annotation was produced.
    """
    annotated = generate_annotations(function_source)
    if not annotated:
        return None, None

    for round_num in range(1, MAX_ROUNDS + 1):
        print(f"  [wp] Verification round {round_num}/{MAX_ROUNDS}...")

        # Splice annotated function back into the full file context
        # (Simple: replace the original function text with annotated version)
        full_source = c_file_context.replace(function_source, annotated) \
                      if c_file_context else annotated

        result = run_framac(source=full_source)

        if result.passed:
            print(f"  [wp] All proof obligations discharged in round {round_num}.")
            return annotated, result

        if round_num == MAX_ROUNDS:
            print(f"  [wp] Could not discharge all obligations after {MAX_ROUNDS} rounds.")
            return annotated, result

        print(f"  [wp] {result.failed_count} obligation(s) failed — refining...")
        refined = refine_annotations(annotated, result.failures_text)
        if not refined:
            print("  [llm] No response — stopping loop.")
            return annotated, result

        annotated = refined

    return annotated, None
