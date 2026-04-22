"""Data structures — stack, queue, heap (min-heap), and hash table
implemented using flat arrays with comprehensive tests."""


def stack_push(data: list, top: list, val: int) -> None:
    data[top[0]] = val
    top[0] = top[0] + 1


def stack_pop(data: list, top: list) -> int:
    if top[0] == 0:
        return -1
    top[0] = top[0] - 1
    return data[top[0]]


def stack_peek(data: list, top: list) -> int:
    if top[0] == 0:
        return -1
    return data[top[0] - 1]


def stack_size(top: list) -> int:
    return top[0]


def stack_is_empty(top: list) -> bool:
    return top[0] == 0


def queue_init(head: list, tail: list) -> None:
    head[0] = 0
    tail[0] = 0


def queue_enqueue(data: list, tail: list, val: int) -> None:
    data[tail[0]] = val
    tail[0] = tail[0] + 1


def queue_dequeue(data: list, head: list, tail: list) -> int:
    if head[0] == tail[0]:
        return -1
    val: int = data[head[0]]
    head[0] = head[0] + 1
    return val


def queue_peek(data: list, head: list, tail: list) -> int:
    if head[0] == tail[0]:
        return -1
    return data[head[0]]


def queue_size(head: list, tail: list) -> int:
    return tail[0] - head[0]


def queue_is_empty(head: list, tail: list) -> bool:
    return head[0] == tail[0]


def heap_parent(i: int) -> int:
    return (i - 1) // 2


def heap_left(i: int) -> int:
    return 2 * i + 1


def heap_right(i: int) -> int:
    return 2 * i + 2


def heap_swap(data: list, i: int, j: int) -> None:
    temp: int = data[i]
    data[i] = data[j]
    data[j] = temp


def heap_sift_up(data: list, idx: int) -> None:
    while idx > 0:
        p: int = heap_parent(idx)
        if data[idx] < data[p]:
            heap_swap(data, idx, p)
            idx = p
        else:
            idx = 0


def heap_sift_down(data: list, idx: int, size: int) -> None:
    done: bool = False
    while not done:
        smallest: int = idx
        left: int = heap_left(idx)
        right: int = heap_right(idx)
        if left < size and data[left] < data[smallest]:
            smallest = left
        if right < size and data[right] < data[smallest]:
            smallest = right
        if smallest != idx:
            heap_swap(data, idx, smallest)
            idx = smallest
        else:
            done = True


def heap_insert(data: list, size: list, val: int) -> None:
    data[size[0]] = val
    heap_sift_up(data, size[0])
    size[0] = size[0] + 1


def heap_extract_min(data: list, size: list) -> int:
    if size[0] == 0:
        return -1
    min_val: int = data[0]
    size[0] = size[0] - 1
    data[0] = data[size[0]]
    heap_sift_down(data, 0, size[0])
    return min_val


def heap_peek_min(data: list, size: list) -> int:
    if size[0] == 0:
        return -1
    return data[0]


def hash_func(key: int, capacity: int) -> int:
    h: int = key
    if h < 0:
        h = 0 - h
    return h % capacity


def ht_init(keys: list, vals: list, used: list, capacity: int) -> None:
    i: int = 0
    while i < capacity:
        keys[i] = 0
        vals[i] = 0
        used[i] = 0
        i = i + 1


def ht_put(keys: list, vals: list, used: list, capacity: int, key: int, val: int) -> bool:
    h: int = hash_func(key, capacity)
    probes: int = 0
    while probes < capacity:
        idx: int = (h + probes) % capacity
        if used[idx] == 0:
            keys[idx] = key
            vals[idx] = val
            used[idx] = 1
            return True
        if keys[idx] == key:
            vals[idx] = val
            return True
        probes = probes + 1
    return False


def ht_get(keys: list, vals: list, used: list, capacity: int, key: int) -> int:
    h: int = hash_func(key, capacity)
    probes: int = 0
    while probes < capacity:
        idx: int = (h + probes) % capacity
        if used[idx] == 0:
            return -1
        if keys[idx] == key:
            return vals[idx]
        probes = probes + 1
    return -1


def ht_contains(keys: list, used: list, capacity: int, key: int) -> bool:
    h: int = hash_func(key, capacity)
    probes: int = 0
    while probes < capacity:
        idx: int = (h + probes) % capacity
        if used[idx] == 0:
            return False
        if keys[idx] == key:
            return True
        probes = probes + 1
    return False


def ht_count(used: list, capacity: int) -> int:
    count: int = 0
    i: int = 0
    while i < capacity:
        if used[i] == 1:
            count = count + 1
        i = i + 1
    return count


def is_balanced_parens(expr: str) -> bool:
    stack_data: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    stack_top: list = [0]
    i: int = 0
    while i < len(expr):
        ch: str = expr[i:i + 1]
        if ch == "(" or ch == "[" or ch == "{":
            stack_push(stack_data, stack_top, i)
        if ch == ")" or ch == "]" or ch == "}":
            if stack_is_empty(stack_top):
                return False
            stack_pop(stack_data, stack_top)
        i = i + 1
    return stack_is_empty(stack_top)


def reverse_with_stack(arr: list, count: int) -> None:
    stack_data: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    stack_top: list = [0]
    i: int = 0
    while i < count:
        stack_push(stack_data, stack_top, arr[i])
        i = i + 1
    i = 0
    while i < count:
        arr[i] = stack_pop(stack_data, stack_top)
        i = i + 1


def heap_sort(arr: list, count: int) -> None:
    heap_data: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    heap_size: list = [0]
    i: int = 0
    while i < count:
        heap_insert(heap_data, heap_size, arr[i])
        i = i + 1
    i = 0
    while i < count:
        arr[i] = heap_extract_min(heap_data, heap_size)
        i = i + 1


def print_array(arr: list, count: int) -> None:
    result: str = ""
    i: int = 0
    while i < count:
        if i > 0:
            result = result + " "
        result = result + str(arr[i])
        i = i + 1
    print(result)


def test_stack() -> None:
    print("=== Stack ===")
    data: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    top: list = [0]
    if stack_is_empty(top):
        print("Empty: yes")
    stack_push(data, top, 10)
    stack_push(data, top, 20)
    stack_push(data, top, 30)
    print("Size: " + str(stack_size(top)))
    print("Peek: " + str(stack_peek(data, top)))
    print("Pop: " + str(stack_pop(data, top)))
    print("Pop: " + str(stack_pop(data, top)))
    print("Size: " + str(stack_size(top)))
    print("Pop: " + str(stack_pop(data, top)))
    if stack_is_empty(top):
        print("Empty: yes")
    print("Pop empty: " + str(stack_pop(data, top)))

    i: int = 0
    while i < 10:
        stack_push(data, top, i * 5)
        i = i + 1
    print("Size after 10 pushes: " + str(stack_size(top)))
    result: str = ""
    while not stack_is_empty(top):
        val: int = stack_pop(data, top)
        if result != "":
            result = result + " "
        result = result + str(val)
    print("Popped: " + result)


def test_queue() -> None:
    print("=== Queue ===")
    data: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    head: list = [0]
    tail: list = [0]
    queue_init(head, tail)
    if queue_is_empty(head, tail):
        print("Empty: yes")
    queue_enqueue(data, tail, 100)
    queue_enqueue(data, tail, 200)
    queue_enqueue(data, tail, 300)
    print("Size: " + str(queue_size(head, tail)))
    print("Peek: " + str(queue_peek(data, head, tail)))
    print("Dequeue: " + str(queue_dequeue(data, head, tail)))
    print("Dequeue: " + str(queue_dequeue(data, head, tail)))
    print("Size: " + str(queue_size(head, tail)))
    queue_enqueue(data, tail, 400)
    queue_enqueue(data, tail, 500)
    print("Size: " + str(queue_size(head, tail)))
    result: str = ""
    while not queue_is_empty(head, tail):
        val: int = queue_dequeue(data, head, tail)
        if result != "":
            result = result + " "
        result = result + str(val)
    print("Drained: " + result)


def test_heap() -> None:
    print("=== Min-Heap ===")
    data: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    size: list = [0]
    heap_insert(data, size, 50)
    heap_insert(data, size, 30)
    heap_insert(data, size, 70)
    heap_insert(data, size, 10)
    heap_insert(data, size, 40)
    heap_insert(data, size, 20)
    heap_insert(data, size, 60)
    print("Size: " + str(size[0]))
    print("Min: " + str(heap_peek_min(data, size)))
    result: str = ""
    while size[0] > 0:
        val: int = heap_extract_min(data, size)
        if result != "":
            result = result + " "
        result = result + str(val)
    print("Extracted in order: " + result)

    vals: list = [9, 8, 7, 6, 5, 4, 3, 2, 1, 0]
    i: int = 0
    while i < 10:
        heap_insert(data, size, vals[i])
        i = i + 1
    result2: str = ""
    while size[0] > 0:
        val2: int = heap_extract_min(data, size)
        if result2 != "":
            result2 = result2 + " "
        result2 = result2 + str(val2)
    print("Reverse insert extracted: " + result2)


def test_hash_table() -> None:
    print("=== Hash Table ===")
    capacity: int = 16
    keys: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    vals: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    used: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    ht_init(keys, vals, used, capacity)
    ht_put(keys, vals, used, capacity, 42, 100)
    ht_put(keys, vals, used, capacity, 17, 200)
    ht_put(keys, vals, used, capacity, 99, 300)
    ht_put(keys, vals, used, capacity, 5, 400)
    ht_put(keys, vals, used, capacity, 33, 500)
    print("Count: " + str(ht_count(used, capacity)))
    print("Get 42: " + str(ht_get(keys, vals, used, capacity, 42)))
    print("Get 17: " + str(ht_get(keys, vals, used, capacity, 17)))
    print("Get 99: " + str(ht_get(keys, vals, used, capacity, 99)))
    print("Get 0: " + str(ht_get(keys, vals, used, capacity, 0)))
    if ht_contains(keys, used, capacity, 42):
        print("Contains 42: yes")
    if not ht_contains(keys, used, capacity, 100):
        print("Contains 100: no")
    ht_put(keys, vals, used, capacity, 42, 999)
    print("Get 42 after update: " + str(ht_get(keys, vals, used, capacity, 42)))
    print("Count after update: " + str(ht_count(used, capacity)))

    i: int = 0
    while i < 8:
        ht_put(keys, vals, used, capacity, i * 16, i * 10)
        i = i + 1
    print("Count after collisions: " + str(ht_count(used, capacity)))
    print("Get 0: " + str(ht_get(keys, vals, used, capacity, 0)))
    print("Get 48: " + str(ht_get(keys, vals, used, capacity, 48)))
    print("Get 112: " + str(ht_get(keys, vals, used, capacity, 112)))


def test_balanced_parens() -> None:
    print("=== Balanced Parens ===")
    tests: list = ["()", "(())", "((()))", "()()", "(()", "())", "([{}])", "([)]", "{[()]}"]
    i: int = 0
    while i < 9:
        expr: str = tests[i]
        if is_balanced_parens(expr):
            print(expr + ": balanced")
        else:
            print(expr + ": not balanced")
        i = i + 1


def test_reverse_stack() -> None:
    print("=== Reverse with Stack ===")
    arr: list = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    count: int = 10
    print("Before:")
    print_array(arr, count)
    reverse_with_stack(arr, count)
    print("After:")
    print_array(arr, count)


def test_heap_sort() -> None:
    print("=== Heap Sort ===")
    arr: list = [64, 34, 25, 12, 22, 11, 90, 45, 33, 17]
    count: int = 10
    print("Before:")
    print_array(arr, count)
    heap_sort(arr, count)
    print("After:")
    print_array(arr, count)

    arr2: list = [5, 4, 3, 2, 1, 0, 0, 0, 0, 0]
    count2: int = 5
    print("Before:")
    print_array(arr2, count2)
    heap_sort(arr2, count2)
    print("After:")
    print_array(arr2, count2)


def test_stack_eval() -> None:
    print("=== Stack Eval ===")
    expr_data: list = [3, 4, -1, 2, -2, 0, 0, 0, 0, 0]
    expr_len: int = 5
    stack_data: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    stack_top: list = [0]
    i: int = 0
    while i < expr_len:
        token: int = expr_data[i]
        if token >= 0:
            stack_push(stack_data, stack_top, token)
        else:
            b: int = stack_pop(stack_data, stack_top)
            a: int = stack_pop(stack_data, stack_top)
            result: int = 0
            if token == -1:
                result = a + b
            if token == -2:
                result = a * b
            if token == -3:
                result = a - b
            if token == -4:
                if b != 0:
                    result = a // b
            stack_push(stack_data, stack_top, result)
        i = i + 1
    print("(3+4)*2 = " + str(stack_pop(stack_data, stack_top)))

    expr2: list = [5, 1, 2, -1, 4, -2, -1, 3, -3, 0]
    expr2_len: int = 9
    stack_top[0] = 0
    i = 0
    while i < expr2_len:
        token2: int = expr2[i]
        if token2 >= 0:
            stack_push(stack_data, stack_top, token2)
        else:
            b2: int = stack_pop(stack_data, stack_top)
            a2: int = stack_pop(stack_data, stack_top)
            res2: int = 0
            if token2 == -1:
                res2 = a2 + b2
            if token2 == -2:
                res2 = a2 * b2
            if token2 == -3:
                res2 = a2 - b2
            if token2 == -4:
                if b2 != 0:
                    res2 = a2 // b2
            stack_push(stack_data, stack_top, res2)
        i = i + 1
    print("5+(1+2)*4-3 = " + str(stack_pop(stack_data, stack_top)))


def main() -> int:
    test_stack()
    test_queue()
    test_heap()
    test_hash_table()
    test_balanced_parens()
    test_reverse_stack()
    test_heap_sort()
    test_stack_eval()
    return 0


if __name__ == "__main__":
    main()
