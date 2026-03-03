"""
github_client.py — post an attestation to a GitHub repository.

This uses GitHub as an append-only public ledger for attestations.
Git commit history gives you timestamps.  The repo is readable by anyone.
No blockchain fees.  Less decentralised than IPFS+Ethereum but much more
practical to start with — every developer already has GitHub.

Setup
-----
1. Create a GitHub repo (public or private) for attestations, e.g.:
       github.com/yourname/memresistant-attestations
2. Generate a Personal Access Token (PAT) with repo write scope.
3. Set env vars:
       GITHUB_TOKEN=<your PAT>
       GITHUB_REPO=yourname/memresistant-attestations
"""

import base64
import json
import os
import urllib.request
import urllib.error
from dataclasses import asdict
from ..core.attestation import Attestation
from ..core.report import generate_github_markdown

_API = 'https://api.github.com'


def _headers(token: str) -> dict:
    return {
        'Authorization': f'token {token}',
        'Accept':        'application/vnd.github+json',
        'Content-Type':  'application/json',
        'X-GitHub-Api-Version': '2022-11-28',
    }


def _get(url: str, token: str) -> dict | None:
    req = urllib.request.Request(url, headers=_headers(token))
    try:
        with urllib.request.urlopen(req, timeout=15) as r:
            return json.loads(r.read())
    except urllib.error.HTTPError as e:
        if e.code == 404:
            return None
        raise


def _put(url: str, token: str, payload: dict) -> dict:
    data = json.dumps(payload).encode('utf-8')
    req = urllib.request.Request(url, data=data, headers=_headers(token),
                                 method='PUT')
    with urllib.request.urlopen(req, timeout=15) as r:
        return json.loads(r.read())


def publish_to_github(att: Attestation, ipfs_cid: str | None = None) -> str | None:
    """
    Post the attestation to the configured GitHub repo.
    Creates two files:
      attestations/<source_hash>.json   — machine-readable full attestation
      attestations/<source_hash>.md     — human-readable GitHub markdown

    Returns the URL to the attestation file, or None on failure.
    """
    token = os.environ.get('GITHUB_TOKEN')
    repo  = os.environ.get('GITHUB_REPO')

    if not token or not repo:
        print("  [github] GITHUB_TOKEN or GITHUB_REPO not set — skipping.")
        print("           Set both env vars to enable GitHub publishing.")
        return None

    short = att.source_hash[:16]
    status = 'PASS' if att.overall_pass else 'FAIL'
    commit_msg = f"attestation: {short} [{status}]"

    # Add IPFS CID to attestation dict if available
    att_dict = asdict(att)
    if ipfs_cid:
        att_dict['ipfs_cid'] = ipfs_cid

    files = {
        f'attestations/{att.source_hash}.json': json.dumps(att_dict, indent=2),
        f'attestations/{att.source_hash}.md':   generate_github_markdown(att),
    }

    results = []
    for path, content in files.items():
        url = f'{_API}/repos/{repo}/contents/{path}'
        existing = _get(url, token)
        payload = {
            'message': commit_msg,
            'content': base64.b64encode(content.encode('utf-8')).decode(),
        }
        if existing:
            payload['sha'] = existing['sha']   # required for updates
        try:
            r = _put(url, token, payload)
            results.append(r.get('content', {}).get('html_url', ''))
        except Exception as e:
            print(f"  [github] Failed to write {path}: {e}")
            return None

    url = results[0] if results else None
    if url:
        print(f"  [github] Published: {url}")
    return url
