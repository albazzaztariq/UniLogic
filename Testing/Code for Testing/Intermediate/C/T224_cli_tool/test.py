from dataclasses import dataclass, field
def str_char_at(s, i): return s[i] if 0 <= i < len(s) else ''
def strlen(s): return len(s)

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def find_task_index(names, count, target):
    i = 0
    while (i < count):
        if (names[i] == target):
            return i
        i = (i + 1)
    return (-1)

def count_done(done, count):
    result = 0
    i = 0
    while (i < count):
        if (done[i] == 1):
            result = (result + 1)
        i = (i + 1)
    return result

def starts_with(s, prefix):
    slen = strlen(s)
    plen = strlen(prefix)
    if (plen > slen):
        return False
    i = 0
    while (i < plen):
        sc = str_char_at(s, i)
        pc = str_char_at(prefix, i)
        if (sc != pc):
            return False
        i = (i + 1)
    return True

def format_line(index, name, is_done):
    idx_str = str(index)
    prefix = ''
    if (is_done == 1):
        prefix = (idx_str + '. [x] ')
    else:
        prefix = (idx_str + '. [ ] ')
    return (prefix + name)

def main():
    names = ['', '', '', '', '', '', '', '', '', '']
    done = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    task_count = 0
    print('--- Adding tasks ---')
    names[0] = 'Buy groceries'
    names[1] = 'Write report'
    names[2] = 'Build prototype'
    names[3] = 'Review code'
    names[4] = 'Buy hardware'
    task_count = 5
    print(('Tasks added: ' + str(task_count)))
    print('--- All tasks ---')
    i = 0
    while (i < task_count):
        print(format_line(i, names[i], done[i]))
        i = (i + 1)
    print('--- Completing tasks ---')
    done[0] = 1
    done[2] = 1
    print(('Completed: ' + str(count_done(done, task_count))))
    print('--- Updated tasks ---')
    i = 0
    while (i < task_count):
        print(format_line(i, names[i], done[i]))
        i = (i + 1)
    print('--- Search: Buy ---')
    i = 0
    while (i < task_count):
        if starts_with(names[i], 'Buy'):
            print(format_line(i, names[i], done[i]))
        i = (i + 1)
    print('--- Find ---')
    idx = find_task_index(names, task_count, 'Review code')
    print(("Found 'Review code' at index: " + str(idx)))
    idx2 = find_task_index(names, task_count, 'Nonexistent')
    print(("Found 'Nonexistent' at index: " + str(idx2)))
    print('--- Summary ---')
    comp_str = str(count_done(done, task_count))
    total_str = str(task_count)
    summary = (((comp_str + '/') + total_str) + ' completed')
    print(summary)
    return 0


if __name__ == "__main__":
    main()
