from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def get_state_name(state):
    if (state == 0):
        return 'RED'
    if (state == 1):
        return 'GREEN'
    if (state == 2):
        return 'YELLOW'
    if (state == 3):
        return 'PED_WALK'
    if (state == 4):
        return 'PED_FLASH'
    return 'UNKNOWN'

def next_state(state, ped_button):
    if (state == 0):
        if (ped_button == 1):
            return 3
        return 1
    if (state == 1):
        if (ped_button == 1):
            return 2
        return 2
    if (state == 2):
        return 0
    if (state == 3):
        return 4
    if (state == 4):
        return 0
    return 0

def state_duration(state):
    if (state == 0):
        return 30
    if (state == 1):
        return 45
    if (state == 2):
        return 5
    if (state == 3):
        return 20
    if (state == 4):
        return 10
    return 0

def simulate(events, event_count):
    state = 0
    total_time = 0
    transitions = 0
    time_in_red = 0
    time_in_green = 0
    time_in_yellow = 0
    time_in_ped = 0
    i = 0
    while (i < event_count):
        ped = events[i]
        dur = state_duration(state)
        name = get_state_name(state)
        print(((((((('tick ' + str(i)) + ': state=') + name) + ' dur=') + str(dur)) + ' ped=') + str(ped)))
        if (state == 0):
            time_in_red = (time_in_red + dur)
        if (state == 1):
            time_in_green = (time_in_green + dur)
        if (state == 2):
            time_in_yellow = (time_in_yellow + dur)
        if ((state == 3) or (state == 4)):
            time_in_ped = (time_in_ped + dur)
        total_time = (total_time + dur)
        state = next_state(state, ped)
        transitions = (transitions + 1)
        i = (i + 1)
    print('--- Summary ---')
    print(('transitions: ' + str(transitions)))
    print(('total_time: ' + str(total_time)))
    print(('time_in_red: ' + str(time_in_red)))
    print(('time_in_green: ' + str(time_in_green)))
    print(('time_in_yellow: ' + str(time_in_yellow)))
    print(('time_in_ped: ' + str(time_in_ped)))
    print(('final_state: ' + get_state_name(state)))

def count_transitions(events, event_count):
    state = 0
    count = 0
    prev_state = (-1)
    i = 0
    while (i < event_count):
        if (state != prev_state):
            count = (count + 1)
        prev_state = state
        state = next_state(state, events[i])
        i = (i + 1)
    return count

def main():
    print('=== Traffic Light State Machine ===')
    print('--- Run 1: No pedestrians ---')
    events1 = [0, 0, 0, 0, 0, 0, 0, 0]
    simulate(events1, 8)
    print('--- Run 2: Pedestrian at tick 1 ---')
    events2 = [0, 1, 0, 0, 0, 0, 0, 0]
    simulate(events2, 8)
    print('--- Run 3: Multiple pedestrians ---')
    events3 = [1, 0, 0, 1, 0, 0, 1, 0, 0, 0]
    simulate(events3, 10)
    print('--- Transition counts ---')
    print(('run1: ' + str(count_transitions(events1, 8))))
    print(('run2: ' + str(count_transitions(events2, 8))))
    print(('run3: ' + str(count_transitions(events3, 10))))
    return 0


if __name__ == "__main__":
    main()
