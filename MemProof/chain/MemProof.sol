// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

/**
 * MemProof Registry
 *
 * An immutable on-chain ledger mapping source file hashes to their
 * formal verification attestations stored on IPFS.
 *
 * How it works
 * ------------
 * 1. Developer runs: python memproof.py code.c --attest
 * 2. MemProof runs CBMC + Frama-C/WP + ASan, builds an attestation JSON.
 * 3. Attestation is uploaded to IPFS -> returns a CID.
 * 4. This contract records: (source_hash -> ipfs_cid, timestamp, attester)
 *
 * Anyone can verify by:
 *   a. Hashing the code they have -> source_hash
 *   b. Calling getAttestation(source_hash) to get the IPFS CID
 *   c. Fetching the attestation from IPFS
 *   d. Re-running the tools and comparing outputs
 *
 * Trust model
 * -----------
 * - The blockchain provides an immutable timestamp.  Nobody can claim
 *   their code was verified before it was — the block timestamp proves it.
 * - The IPFS CID is content-addressed: if the attestation is tampered with,
 *   the CID changes and no longer matches what is on chain.
 * - The attester address provides identity (optional — anyone can submit).
 * - There is NO central authority.  This contract is deployed once and
 *   runs forever on Ethereum without any admin key or upgrade mechanism.
 *
 * Deployment
 * ----------
 * Deploy once to any EVM chain (Ethereum mainnet, Polygon, Base, etc.).
 * Copy the contract address into memproof.py config.
 * Gas cost per attestation: ~50,000 gas (~$0.05 on L2 chains).
 */
contract MemProofRegistry {

    struct Attestation {
        string  ipfsCid;      // IPFS CID of the full attestation JSON
        uint256 timestamp;    // block.timestamp at submission
        address attester;     // who submitted (optional identity)
        bool    passed;       // did all verification tools pass?
    }

    // source_hash (bytes32 = SHA-256 truncated to 32 bytes) -> Attestation
    mapping(bytes32 => Attestation) private _attestations;

    // Ordered list of all attested hashes (for enumeration)
    bytes32[] private _allHashes;

    event Attested(
        bytes32 indexed sourceHash,
        string  ipfsCid,
        address indexed attester,
        bool    passed
    );

    /**
     * Submit an attestation.
     *
     * @param sourceHash  SHA-256 of the verified source file as bytes32.
     * @param ipfsCid     IPFS CID string of the full attestation JSON.
     * @param passed      True if all verification tools reported pass.
     *
     * An existing attestation for the same hash can be overwritten —
     * the previous entry remains visible in event logs (immutable).
     */
    function attest(
        bytes32 sourceHash,
        string calldata ipfsCid,
        bool passed
    ) external {
        require(bytes(ipfsCid).length > 0, "CID cannot be empty");

        if (bytes(_attestations[sourceHash].ipfsCid).length == 0) {
            _allHashes.push(sourceHash);   // first time seeing this hash
        }

        _attestations[sourceHash] = Attestation({
            ipfsCid:   ipfsCid,
            timestamp: block.timestamp,
            attester:  msg.sender,
            passed:    passed
        });

        emit Attested(sourceHash, ipfsCid, msg.sender, passed);
    }

    /**
     * Look up an attestation by source hash.
     * Returns empty strings/zeros if the hash has never been attested.
     */
    function getAttestation(bytes32 sourceHash)
        external view
        returns (string memory ipfsCid, uint256 timestamp,
                 address attester, bool passed)
    {
        Attestation storage a = _attestations[sourceHash];
        return (a.ipfsCid, a.timestamp, a.attester, a.passed);
    }

    /**
     * Check whether a source hash has a passing attestation.
     * Convenience function for tooling integrations.
     */
    function isVerified(bytes32 sourceHash) external view returns (bool) {
        return _attestations[sourceHash].passed;
    }

    /** Total number of unique source hashes ever attested. */
    function totalAttestations() external view returns (uint256) {
        return _allHashes.length;
    }

    /** Enumerate attested hashes by index (for off-chain indexers). */
    function hashAt(uint256 index) external view returns (bytes32) {
        return _allHashes[index];
    }
}
