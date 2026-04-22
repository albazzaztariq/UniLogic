"""
Test file: simplified celery/states.py.
Self-contained, no external deps.
Stresses: module-level constants, frozenset (-> array), dict built from zip,
          function with try/except KeyError, class subclassing str with comparison
          operator overloads, dynamic dispatch via precedence table.
Adaptations:
  - frozenset -> list (UL has no frozenset; verified with contains checks).
  - PRECEDENCE_LOOKUP: dict(zip(...)) replaced with explicit map construction.
  - state class: str subclass with __gt__/__ge__/__lt__/__le__ — emitted as object.
  - None in PRECEDENCE list replaced with "" sentinel (UL has no None in array).
"""

PENDING = "PENDING"
RECEIVED = "RECEIVED"
STARTED = "STARTED"
SUCCESS = "SUCCESS"
FAILURE = "FAILURE"
REVOKED = "REVOKED"
REJECTED = "REJECTED"
RETRY = "RETRY"
IGNORED = "IGNORED"

# Precedence list: lower index = higher precedence.
# None replaced by "" as UL arrays cannot hold None.
PRECEDENCE = [
    "SUCCESS",
    "FAILURE",
    "",
    "REVOKED",
    "STARTED",
    "RECEIVED",
    "REJECTED",
    "RETRY",
    "PENDING",
]

# Build lookup dict manually (no dict(zip(...)) in UL round-trip).
PRECEDENCE_LOOKUP = {}
for _i, _s in enumerate(PRECEDENCE):
    PRECEDENCE_LOOKUP[_s] = _i

NONE_PRECEDENCE = PRECEDENCE_LOOKUP[""]

READY_STATES = [SUCCESS, FAILURE, REVOKED]
UNREADY_STATES = [PENDING, RECEIVED, STARTED, REJECTED, RETRY]
EXCEPTION_STATES = [RETRY, FAILURE, REVOKED]
PROPAGATE_STATES = [FAILURE, REVOKED]

ALL_STATES = [PENDING, RECEIVED, STARTED, SUCCESS, FAILURE, RETRY, REVOKED]


def precedence(s):
    """Get precedence index for state. Lower index = higher precedence."""
    try:
        return PRECEDENCE_LOOKUP[s]
    except KeyError:
        return NONE_PRECEDENCE


class state:
    """Task state with precedence-based comparison."""

    def __init__(self, s=""):
        self.s = s

    def __gt__(self, other):
        return precedence(self.s) < precedence(other.s)

    def __ge__(self, other):
        return precedence(self.s) <= precedence(other.s)

    def __lt__(self, other):
        return precedence(self.s) > precedence(other.s)

    def __le__(self, other):
        return precedence(self.s) >= precedence(other.s)

    def __str__(self):
        return self.s

    def __repr__(self):
        return "state(" + self.s + ")"


def main():
    # Module-level constant checks
    print("PENDING:", PENDING)
    print("SUCCESS:", SUCCESS)
    print("FAILURE:", FAILURE)

    # precedence function
    print("precedence(SUCCESS):", precedence(SUCCESS))
    print("precedence(FAILURE):", precedence(FAILURE))
    print("precedence(PENDING):", precedence(PENDING))
    print("precedence(unknown):", precedence("CUSTOM"))

    # NONE_PRECEDENCE
    print("NONE_PRECEDENCE:", NONE_PRECEDENCE)

    # State set membership
    print("SUCCESS in READY_STATES:", SUCCESS in READY_STATES)
    print("PENDING in READY_STATES:", PENDING in READY_STATES)
    print("PENDING in UNREADY_STATES:", PENDING in UNREADY_STATES)
    print("FAILURE in EXCEPTION_STATES:", FAILURE in EXCEPTION_STATES)
    print("FAILURE in PROPAGATE_STATES:", FAILURE in PROPAGATE_STATES)
    print("REVOKED in PROPAGATE_STATES:", REVOKED in PROPAGATE_STATES)
    print("STARTED in PROPAGATE_STATES:", STARTED in PROPAGATE_STATES)

    # state class comparisons (precedence-based)
    s_pending = state(PENDING)
    s_success = state(SUCCESS)
    s_failure = state(FAILURE)
    s_started = state(STARTED)
    s_custom = state("PROGRESS")

    print("state(PENDING) < state(SUCCESS):", s_pending < s_success)
    print("state(SUCCESS) > state(PENDING):", s_success > s_pending)
    print("state(FAILURE) > state(SUCCESS):", s_failure > s_success)
    print("state(SUCCESS) >= state(SUCCESS):", s_success >= s_success)
    print("state(SUCCESS) <= state(SUCCESS):", s_success <= s_success)

    # Custom state: unknown precedence -> NONE_PRECEDENCE (index 2)
    # NONE_PRECEDENCE=2, SUCCESS=0, FAILURE=1, STARTED=4
    # custom > STARTED means precedence(custom) < precedence(STARTED) => 2 < 4 => True
    print("state(PROGRESS) > state(STARTED):", s_custom > s_started)
    # custom > SUCCESS: precedence(custom)=2, precedence(SUCCESS)=0 => 2 < 0 => False
    print("state(PROGRESS) > state(SUCCESS):", s_custom > s_success)

    # PRECEDENCE list traversal
    print("PRECEDENCE length:", len(PRECEDENCE))
    print("PRECEDENCE[0]:", PRECEDENCE[0])
    print("PRECEDENCE[8]:", PRECEDENCE[8])

    # ALL_STATES length
    print("ALL_STATES length:", len(ALL_STATES))

    # state __str__ and __repr__
    print("str(state(FAILURE)):", str(s_failure))
    print("repr(state(RETRY)):", repr(state(RETRY)))


main()
