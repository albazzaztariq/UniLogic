"""
ipfs_client.py — publish an attestation to IPFS via Pinata.

Why IPFS?
  The CID (Content Identifier) returned by IPFS IS the SHA-256 of the
  content.  You cannot change the content without changing the CID.
  That makes it a perfect immutable store for attestations.

Why Pinata?
  Running your own IPFS node is fine but requires infrastructure.
  Pinata is a pinning service — they keep your content available on the
  IPFS network.  Free tier: 1 GB.  No account needed for reading.

To use this module set the env var:  PINATA_JWT=<your jwt token>
Get a free token at: https://app.pinata.cloud/

Alternative: web3.storage  (set WEB3_STORAGE_TOKEN instead)
"""

import json
import os
import urllib.request
import urllib.error
from dataclasses import asdict
from ..core.attestation import Attestation

_PINATA_URL = 'https://api.pinata.cloud/pinning/pinJSONToIPFS'


def publish_to_ipfs(att: Attestation) -> str | None:
    """
    Upload the attestation JSON to IPFS via Pinata.
    Returns the IPFS CID on success, None on failure.

    The CID is content-addressed: it uniquely identifies this exact
    attestation.  Store it alongside your source code.
    """
    token = os.environ.get('PINATA_JWT')
    if not token:
        print("  [ipfs] PINATA_JWT not set — skipping IPFS publish.")
        print("         Set PINATA_JWT=<your token> to enable.")
        return None

    payload = json.dumps({
        'pinataContent': asdict(att),
        'pinataMetadata': {
            'name': f'memresistant-{att.source_hash[:12]}',
        },
    }).encode('utf-8')

    req = urllib.request.Request(
        _PINATA_URL,
        data    = payload,
        headers = {
            'Content-Type':  'application/json',
            'Authorization': f'Bearer {token}',
        },
        method = 'POST',
    )

    try:
        with urllib.request.urlopen(req, timeout=30) as resp:
            result = json.loads(resp.read())
            cid = result.get('IpfsHash')
            if cid:
                print(f"  [ipfs] Published: ipfs://{cid}")
                print(f"         Gateway:  https://gateway.pinata.cloud/ipfs/{cid}")
            return cid
    except urllib.error.HTTPError as e:
        print(f"  [ipfs] HTTP {e.code}: {e.read().decode()}")
        return None
    except Exception as e:
        print(f"  [ipfs] Error: {e}")
        return None
