# Pragma Memory Safety

Pragma's memory safety system operates in two tiers, each with increasing verification strength.

**[View the Memory Safety Overview Page](https://pragma-project.github.io/Pragma-Language/safety.html)**

---

## Tiers

### MemResistant — Static Analysis at Build Time
Dynamic and static analysis tools that run automatically during compilation:
- **AddressSanitizer (ASan)** — heap/stack buffer overflows, use-after-free
- **ThreadSanitizer (TSan)** — data races in concurrent code
- **UndefinedBehaviorSanitizer (UBSan)** — undefined behavior detection
- **CBMC** — bounded model checking for deeper property verification
- Source is hashed (SHA-256) and an attestation record is published to IPFS

### MemProof — Formal Verification
Full mathematical proof of memory safety properties:
- **Frama-C WP** — weakest precondition calculus over ACSL annotations
- LLM-assisted ACSL annotation generation with feedback loop
- Proof attestation stored on IPFS and anchored to Ethereum (smart contract)
- Blockchain-verifiable: anyone can confirm a given source hash was formally verified
