"""
eth_client.py — submit attestations to the MemProofRegistry smart contract.

Dependencies: web3  (pip install web3)

Setup
-----
Set these environment variables:

  ETH_RPC_URL        RPC endpoint for your chosen chain.
                     Recommended: Polygon or Base (cheap gas, EVM-compatible)
                     Free endpoints: https://rpc.ankr.com/polygon
                                     https://mainnet.base.org
  ETH_PRIVATE_KEY    Your wallet's private key (hex, with or without 0x).
                     This signs the on-chain transaction.  Keep it secret.
  MEMPROOF_CONTRACT  Deployed address of MemProofRegistry.sol.

The private key is ONLY used to sign the attestation transaction.
It is never transmitted anywhere other than your configured RPC endpoint.

Gas cost
--------
~50,000 gas per attestation.  On Polygon/Base: ~$0.001-$0.01.
On Ethereum mainnet: ~$1-5.  Use an L2 for practical use.
"""

import hashlib
import os


def _to_bytes32(hex_hash: str) -> bytes:
    """Convert a 64-char hex SHA-256 string to 32-byte bytes32."""
    return bytes.fromhex(hex_hash[:64])


def submit_attestation(source_hash: str, ipfs_cid: str,
                       passed: bool) -> str | None:
    """
    Submit an attestation to the MemProofRegistry contract.

    Parameters
    ----------
    source_hash : hex SHA-256 of the source file (64 hex chars)
    ipfs_cid    : IPFS CID returned by ipfs_client.publish_to_ipfs()
    passed      : True if all verification tools passed

    Returns the transaction hash on success, None on failure.
    """
    try:
        from web3 import Web3
    except ImportError:
        print("  [chain] web3 not installed.  Run: pip install web3")
        return None

    rpc_url       = os.environ.get('ETH_RPC_URL')
    private_key   = os.environ.get('ETH_PRIVATE_KEY')
    contract_addr = os.environ.get('MEMPROOF_CONTRACT')

    if not all([rpc_url, private_key, contract_addr]):
        print("  [chain] ETH_RPC_URL, ETH_PRIVATE_KEY, MEMPROOF_CONTRACT not set.")
        print("          On-chain attestation skipped.")
        return None

    w3 = Web3(Web3.HTTPProvider(rpc_url))
    if not w3.is_connected():
        print(f"  [chain] Cannot connect to RPC: {rpc_url}")
        return None

    # Minimal ABI — only the attest() function
    abi = [{
        "inputs": [
            {"name": "sourceHash", "type": "bytes32"},
            {"name": "ipfsCid",    "type": "string"},
            {"name": "passed",     "type": "bool"},
        ],
        "name": "attest",
        "outputs": [],
        "stateMutability": "nonpayable",
        "type": "function",
    }]

    account  = w3.eth.account.from_key(private_key)
    contract = w3.eth.contract(
        address=Web3.to_checksum_address(contract_addr),
        abi=abi,
    )

    source_bytes32 = _to_bytes32(source_hash)

    tx = contract.functions.attest(source_bytes32, ipfs_cid, passed).build_transaction({
        'from':  account.address,
        'nonce': w3.eth.get_transaction_count(account.address),
        'gas':   80_000,
        'gasPrice': w3.eth.gas_price,
    })

    signed = w3.eth.account.sign_transaction(tx, private_key=private_key)
    tx_hash = w3.eth.send_raw_transaction(signed.raw_transaction)
    receipt = w3.eth.wait_for_transaction_receipt(tx_hash, timeout=120)

    tx_hex = tx_hash.hex()
    if receipt.status == 1:
        print(f"  [chain] On-chain attestation confirmed: {tx_hex}")
        return tx_hex
    else:
        print(f"  [chain] Transaction reverted: {tx_hex}")
        return None


def query_attestation(source_hash: str) -> dict | None:
    """
    Query the registry for an existing attestation.
    Returns a dict with ipfsCid, timestamp, attester, passed — or None.
    """
    try:
        from web3 import Web3
    except ImportError:
        print("  [chain] web3 not installed.")
        return None

    rpc_url       = os.environ.get('ETH_RPC_URL')
    contract_addr = os.environ.get('MEMPROOF_CONTRACT')
    if not rpc_url or not contract_addr:
        return None

    w3 = Web3(Web3.HTTPProvider(rpc_url))
    abi = [{
        "inputs": [{"name": "sourceHash", "type": "bytes32"}],
        "name": "getAttestation",
        "outputs": [
            {"name": "ipfsCid",   "type": "string"},
            {"name": "timestamp", "type": "uint256"},
            {"name": "attester",  "type": "address"},
            {"name": "passed",    "type": "bool"},
        ],
        "stateMutability": "view",
        "type": "function",
    }]
    contract = w3.eth.contract(
        address=Web3.to_checksum_address(contract_addr), abi=abi)
    result = contract.functions.getAttestation(_to_bytes32(source_hash)).call()
    ipfs_cid, timestamp, attester, passed = result
    if not ipfs_cid:
        return None
    return {
        'ipfs_cid':  ipfs_cid,
        'timestamp': timestamp,
        'attester':  attester,
        'passed':    passed,
    }
