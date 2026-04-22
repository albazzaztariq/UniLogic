/*
 * test_curl_statemachine.c
 * A curl-style connection state machine extracted from curl's multi.c pattern.
 * Tests: integer state constants, switch dispatch, do-while, multiple return paths.
 * No pointers, no malloc. Uses #define constants instead of typedef enum
 * to avoid enum-constant translation issues in the UL transpiler.
 *
 * Note: avoids 'break' inside 'if' inside 'do-while' — uses flag variable
 * instead to avoid c2ul break-in-if-body translation issue.
 */
#include <stdio.h>

/* Connection states (from curl's multi.c CURLMstate — as #define constants) */
#define MSTATE_INIT             0
#define MSTATE_CONNECT          1
#define MSTATE_RESOLVING        2
#define MSTATE_CONNECTING       3
#define MSTATE_CONNECTED        4
#define MSTATE_SENDPROTOCONNECT 5
#define MSTATE_PROTOCONNECT     6
#define MSTATE_DO               7
#define MSTATE_DOING            8
#define MSTATE_DID              9
#define MSTATE_PERFORMING       10
#define MSTATE_RATELIMITING     11
#define MSTATE_DONE             12
#define MSTATE_COMPLETED        13
#define MSTATE_MSGSENT          14
#define MSTATE_LAST             15

/* Return name of a state */
static const char *state_name(int s)
{
    switch (s) {
    case MSTATE_INIT:             return "INIT";
    case MSTATE_CONNECT:          return "CONNECT";
    case MSTATE_RESOLVING:        return "RESOLVING";
    case MSTATE_CONNECTING:       return "CONNECTING";
    case MSTATE_CONNECTED:        return "CONNECTED";
    case MSTATE_SENDPROTOCONNECT: return "SENDPROTOCONNECT";
    case MSTATE_PROTOCONNECT:     return "PROTOCONNECT";
    case MSTATE_DO:               return "DO";
    case MSTATE_DOING:            return "DOING";
    case MSTATE_DID:              return "DID";
    case MSTATE_PERFORMING:       return "PERFORMING";
    case MSTATE_RATELIMITING:     return "RATELIMITING";
    case MSTATE_DONE:             return "DONE";
    case MSTATE_COMPLETED:        return "COMPLETED";
    case MSTATE_MSGSENT:          return "MSGSENT";
    default:                      return "UNKNOWN";
    }
}

/* Determine next state from current state (simplified linear happy-path) */
static int next_state(int current)
{
    switch (current) {
    case MSTATE_INIT:             return MSTATE_CONNECT;
    case MSTATE_CONNECT:          return MSTATE_RESOLVING;
    case MSTATE_RESOLVING:        return MSTATE_CONNECTING;
    case MSTATE_CONNECTING:       return MSTATE_CONNECTED;
    case MSTATE_CONNECTED:        return MSTATE_SENDPROTOCONNECT;
    case MSTATE_SENDPROTOCONNECT: return MSTATE_PROTOCONNECT;
    case MSTATE_PROTOCONNECT:     return MSTATE_DO;
    case MSTATE_DO:               return MSTATE_DOING;
    case MSTATE_DOING:            return MSTATE_DID;
    case MSTATE_DID:              return MSTATE_PERFORMING;
    case MSTATE_PERFORMING:       return MSTATE_DONE;
    case MSTATE_DONE:             return MSTATE_COMPLETED;
    case MSTATE_COMPLETED:        return MSTATE_MSGSENT;
    default:                      return MSTATE_LAST;
    }
}

/* Check if a state is "done" (connection finished) */
static int state_is_terminal(int s)
{
    return (s == MSTATE_COMPLETED || s == MSTATE_MSGSENT || s == MSTATE_LAST);
}

/* Simulate running through the state machine until completion.
 * Uses a 'done' flag instead of break-in-if to be UL-compatible.
 */
static int run_statemachine(int initial, int max_steps)
{
    int state = initial;
    int steps = 0;
    int done = 0;

    do {
        printf("  step %d: %s\n", steps, state_name(state));
        if (state_is_terminal(state))
            done = 1;
        else {
            state = next_state(state);
            steps++;
        }
    } while (done == 0 && steps < max_steps);

    printf("  final: %s steps=%d\n", state_name(state), steps);
    return steps;
}

int main(void)
{
    int i;

    printf("=== State Machine Traversal ===\n");
    run_statemachine(MSTATE_INIT, 20);

    printf("=== State Names ===\n");
    for (i = 0; i < MSTATE_LAST; i++) {
        printf("state=%d name=%s terminal=%d\n",
               i,
               state_name(i),
               state_is_terminal(i));
    }

    return 0;
}
