from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

NULL_PTR = 0

def list_push(values, nexts, head, next_free, val):
    values[next_free] = val
    nexts[next_free] = head
    return next_free

def list_append(values, nexts, head, next_free, val):
    values[next_free] = val
    nexts[next_free] = NULL_PTR
    if (head == NULL_PTR):
        return next_free
    cur = head
    while (nexts[cur] != NULL_PTR):
        cur = nexts[cur]
    nexts[cur] = next_free
    return head

def list_len(nexts, head):
    count = 0
    cur = head
    while (cur != NULL_PTR):
        count = (count + 1)
        cur = nexts[cur]
    return count

def list_contains(values, nexts, head, target):
    cur = head
    while (cur != NULL_PTR):
        if (values[cur] == target):
            return True
        cur = nexts[cur]
    return False

def list_remove(values, nexts, head, target):
    if (head == NULL_PTR):
        return NULL_PTR
    if (values[head] == target):
        return nexts[head]
    prev = head
    cur = nexts[head]
    while (cur != NULL_PTR):
        if (values[cur] == target):
            nexts[prev] = nexts[cur]
            return head
        prev = cur
        cur = nexts[cur]
    return head

def list_print(values, nexts, head):
    cur = head
    while (cur != NULL_PTR):
        print(values[cur])
        cur = nexts[cur]

def list_reverse(values, nexts, head):
    prev = NULL_PTR
    cur = head
    while (cur != NULL_PTR):
        next = nexts[cur]
        nexts[cur] = prev
        prev = cur
        cur = next
    return prev

def list_get(values, nexts, head, n):
    cur = head
    idx = 0
    while (cur != NULL_PTR):
        if (idx == n):
            return values[cur]
        idx = (idx + 1)
        cur = nexts[cur]
    return (-1)

def list_sum(values, nexts, head):
    total = 0
    cur = head
    while (cur != NULL_PTR):
        total = (total + values[cur])
        cur = nexts[cur]
    return total

def main():
    values = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    nexts = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    nf = 1
    head = NULL_PTR
    head = list_push(values, nexts, head, nf, 10)
    nf = (nf + 1)
    head = list_push(values, nexts, head, nf, 20)
    nf = (nf + 1)
    head = list_push(values, nexts, head, nf, 30)
    nf = (nf + 1)
    print('After push 10, 20, 30:')
    list_print(values, nexts, head)
    head = list_append(values, nexts, head, nf, 40)
    nf = (nf + 1)
    head = list_append(values, nexts, head, nf, 50)
    nf = (nf + 1)
    print('After append 40, 50:')
    list_print(values, nexts, head)
    len = list_len(nexts, head)
    print('Length:')
    print(len)
    has20 = list_contains(values, nexts, head, 20)
    has99 = list_contains(values, nexts, head, 99)
    print('Contains 20:')
    print(has20)
    print('Contains 99:')
    print(has99)
    head = list_remove(values, nexts, head, 20)
    print('After remove 20:')
    list_print(values, nexts, head)
    head = list_reverse(values, nexts, head)
    print('After reverse:')
    list_print(values, nexts, head)
    sum = list_sum(values, nexts, head)
    print('Sum:')
    print(sum)
    second = list_get(values, nexts, head, 1)
    print('Element at index 1:')
    print(second)
    final_len = list_len(nexts, head)
    print('Final length:')
    print(final_len)
    return 0


if __name__ == "__main__":
    main()
