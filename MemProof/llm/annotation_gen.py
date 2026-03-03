"""
annotation_gen.py — use a local LLM (via Ollama) or any OpenAI-compatible
API to generate ACSL annotations for C functions.

Why local models?
  No API key required.  No code leaves the machine.  No per-call cost.
  Models like deepseek-coder, codellama, or mistral-code work well.

Ollama setup (recommended):
  1. Install: https://ollama.ai
  2. Pull a model: ollama pull deepseek-coder:6.7b
  3. Run: ollama serve   (starts on http://localhost:11434)

Alternative: any OpenAI-compatible endpoint.
  Set MEMPROOF_LLM_URL and MEMPROOF_LLM_MODEL env vars.

Environment variables:
  MEMPROOF_LLM_URL    Base URL of the LLM API (default: http://localhost:11434)
  MEMPROOF_LLM_MODEL  Model name            (default: deepseek-coder:6.7b)
"""

import json
import os
import urllib.request
import urllib.error
from pathlib import Path

_PROMPTS = Path(__file__).parent / 'prompts'

_DEFAULT_URL   = 'http://localhost:11434'
_DEFAULT_MODEL = 'deepseek-coder:6.7b'


def _load_prompt(name: str, **kwargs) -> str:
    text = (_PROMPTS / name).read_text(encoding='utf-8')
    for key, val in kwargs.items():
        text = text.replace('{' + key + '}', val)
    return text


def _call_ollama(prompt: str, url: str, model: str) -> str | None:
    """Call an Ollama-compatible /api/generate endpoint."""
    payload = json.dumps({
        'model':  model,
        'prompt': prompt,
        'stream': False,
    }).encode('utf-8')

    req = urllib.request.Request(
        f'{url}/api/generate',
        data    = payload,
        headers = {'Content-Type': 'application/json'},
        method  = 'POST',
    )
    try:
        with urllib.request.urlopen(req, timeout=120) as r:
            result = json.loads(r.read())
            return result.get('response', '')
    except urllib.error.URLError as e:
        print(f"  [llm] Cannot reach LLM at {url}: {e}")
        print("        Is Ollama running?  (ollama serve)")
        return None
    except Exception as e:
        print(f"  [llm] Unexpected error: {e}")
        return None


def generate_annotations(function_source: str) -> str | None:
    """
    Ask the LLM to generate ACSL annotations for a C function.
    Returns the annotated function text, or None on failure.
    """
    url   = os.environ.get('MEMPROOF_LLM_URL',   _DEFAULT_URL)
    model = os.environ.get('MEMPROOF_LLM_MODEL',  _DEFAULT_MODEL)

    prompt = _load_prompt('generate_acsl.txt', function_source=function_source)
    print(f"  [llm] Generating ACSL annotations (model: {model})...")
    return _call_ollama(prompt, url, model)


def refine_annotations(annotated_source: str, wp_failures: str) -> str | None:
    """
    Ask the LLM to fix annotations that Frama-C/WP rejected.
    Returns the corrected annotated function, or None on failure.
    """
    url   = os.environ.get('MEMPROOF_LLM_URL',   _DEFAULT_URL)
    model = os.environ.get('MEMPROOF_LLM_MODEL',  _DEFAULT_MODEL)

    prompt = _load_prompt('refine_acsl.txt',
                          annotated_source=annotated_source,
                          wp_failures=wp_failures)
    print(f"  [llm] Refining annotations based on WP failures...")
    return _call_ollama(prompt, url, model)
