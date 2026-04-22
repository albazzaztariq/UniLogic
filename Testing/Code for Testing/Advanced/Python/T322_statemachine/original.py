"""Simple state machine — traffic light controller with pedestrian button."""


def get_state_name(state: int) -> str:
    if state == 0:
        return "RED"
    if state == 1:
        return "GREEN"
    if state == 2:
        return "YELLOW"
    if state == 3:
        return "PED_WALK"
    if state == 4:
        return "PED_FLASH"
    return "UNKNOWN"


def next_state(state: int, ped_button: int) -> int:
    if state == 0:
        if ped_button == 1:
            return 3
        return 1
    if state == 1:
        if ped_button == 1:
            return 2
        return 2
    if state == 2:
        return 0
    if state == 3:
        return 4
    if state == 4:
        return 0
    return 0


def state_duration(state: int) -> int:
    if state == 0:
        return 30
    if state == 1:
        return 45
    if state == 2:
        return 5
    if state == 3:
        return 20
    if state == 4:
        return 10
    return 0


def simulate(events: list, event_count: int) -> None:
    state: int = 0
    total_time: int = 0
    transitions: int = 0
    time_in_red: int = 0
    time_in_green: int = 0
    time_in_yellow: int = 0
    time_in_ped: int = 0

    i: int = 0
    while i < event_count:
        ped: int = events[i]
        dur: int = state_duration(state)
        name: str = get_state_name(state)
        print("tick " + str(i) + ": state=" + name + " dur=" + str(dur) + " ped=" + str(ped))

        if state == 0:
            time_in_red = time_in_red + dur
        if state == 1:
            time_in_green = time_in_green + dur
        if state == 2:
            time_in_yellow = time_in_yellow + dur
        if state == 3 or state == 4:
            time_in_ped = time_in_ped + dur

        total_time = total_time + dur
        state = next_state(state, ped)
        transitions = transitions + 1
        i = i + 1

    print("--- Summary ---")
    print("transitions: " + str(transitions))
    print("total_time: " + str(total_time))
    print("time_in_red: " + str(time_in_red))
    print("time_in_green: " + str(time_in_green))
    print("time_in_yellow: " + str(time_in_yellow))
    print("time_in_ped: " + str(time_in_ped))
    print("final_state: " + get_state_name(state))


def count_transitions(events: list, event_count: int) -> int:
    state: int = 0
    count: int = 0
    prev_state: int = -1
    i: int = 0
    while i < event_count:
        if state != prev_state:
            count = count + 1
        prev_state = state
        state = next_state(state, events[i])
        i = i + 1
    return count


def main() -> int:
    print("=== Traffic Light State Machine ===")

    print("--- Run 1: No pedestrians ---")
    events1: list = [0, 0, 0, 0, 0, 0, 0, 0]
    simulate(events1, 8)

    print("--- Run 2: Pedestrian at tick 1 ---")
    events2: list = [0, 1, 0, 0, 0, 0, 0, 0]
    simulate(events2, 8)

    print("--- Run 3: Multiple pedestrians ---")
    events3: list = [1, 0, 0, 1, 0, 0, 1, 0, 0, 0]
    simulate(events3, 10)

    print("--- Transition counts ---")
    print("run1: " + str(count_transitions(events1, 8)))
    print("run2: " + str(count_transitions(events2, 8)))
    print("run3: " + str(count_transitions(events3, 10)))

    return 0


if __name__ == "__main__":
    main()
