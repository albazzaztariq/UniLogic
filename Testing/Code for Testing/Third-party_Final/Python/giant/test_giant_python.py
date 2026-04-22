# test_giant_python.py
# Giant synthetic Python program for py2ul round-trip testing.
# Designed to be compatible with py2ul constraints:
#   - No list comprehensions (explicit loops)
#   - No default parameters (explicit params passed by caller)
#   - No *args/**kwargs
#   - No lambda
#   - No f-strings (use + concatenation)
#   - No multiple inheritance
#   - Type-annotated functions
#   - Explicit self.field = value in __init__
#   - User-defined types as field types use int (to avoid UL parse issues)
#     because UL object parser needs types pre-registered before use in same object

# ============================================================
# UTILITY FUNCTIONS — avoid Python string methods that break UL round-trip
# ============================================================

def split_by_char(s: int, sep: str) -> list:
    parts: list = []
    current: str = ""
    i: int = 0
    while i < len(s):
        c: str = s[i]
        if c == sep:
            parts.append(current)
            current = ""
        else:
            current = current + c
        i = i + 1
    parts.append(current)
    return parts


def split_by_space(s: int) -> list:
    words: list = []
    current: str = ""
    i: int = 0
    while i < len(s):
        c: str = s[i]
        if c == " " or c == "\t" or c == "\n":
            if current != "":
                words.append(current)
                current = ""
        else:
            current = current + c
        i = i + 1
    if current != "":
        words.append(current)
    return words


def char_to_lower(c: str) -> str:
    code: int = ord(c)
    if code >= 65 and code <= 90:
        return chr(code + 32)
    return c


def str_to_lower(s: int) -> str:
    result: str = ""
    i: int = 0
    while i < len(s):
        result = result + char_to_lower(s[i])
        i = i + 1
    return result


# ============================================================
# UTILITY: sqrt approximation (Newton's method) — avoids ** operator
# ============================================================

def sqrt_approx(x: float) -> float:
    if x <= 0.0:
        return 0.0
    guess: float = x / 2.0
    i: int = 0
    while i < 50:
        gx: float = x / guess
        ng_sum: float = guess + gx
        new_guess: float = ng_sum / 2.0
        diff: float = new_guess - guess
        if diff < 0.0:
            diff = -diff
        if diff < 0.0001:
            break
        guess = new_guess
        i = i + 1
    return new_guess


# ============================================================
# SECTION 1: DATA STRUCTURES
# ============================================================

class Node:
    def __init__(self: "Node", val: int) -> None:
        self.val: int = val
        self.next: int = None

class LinkedList:
    def __init__(self: "LinkedList") -> None:
        self.head: int = None
        self.sz: int = 0

    def insert(self: "LinkedList", val: int) -> None:
        n: Node = Node(val)
        n.next = self.head
        self.head = n
        self.sz = self.sz + 1

    def find(self: "LinkedList", val: int) -> bool:
        cur = self.head
        while cur is not None:
            if cur.val == val:
                return True
            cur = cur.next
        return False

    def delete(self: "LinkedList", val: int) -> bool:
        if self.head is None:
            return False
        if self.head.val == val:
            self.head = self.head.next
            self.sz = self.sz - 1
            return True
        cur = self.head
        while cur.next is not None:
            if cur.next.val == val:
                cur.next = cur.next.next
                self.sz = self.sz - 1
                return True
            cur = cur.next
        return False

    def to_string(self: "LinkedList") -> str:
        parts: str = ""
        cur = self.head
        first: bool = True
        while cur is not None:
            if first:
                parts = str(cur.val)
                first = False
            else:
                parts = parts + " -> " + str(cur.val)
            cur = cur.next
        return parts

    def reverse(self: "LinkedList") -> None:
        prev = None
        cur = self.head
        while cur is not None:
            nxt = cur.next
            cur.next = prev
            prev = cur
            cur = nxt
        self.head = prev


def test_linked_list() -> None:
    print("--- LinkedList ---")
    ll: LinkedList = LinkedList()
    ll.insert(10)
    ll.insert(20)
    ll.insert(30)
    ll.insert(40)
    print("List: " + ll.to_string())
    print("Find 20: " + str(ll.find(20)))
    print("Find 99: " + str(ll.find(99)))
    ll.delete(20)
    print("After delete 20: " + ll.to_string())
    ll.reverse()
    print("Reversed: " + ll.to_string())
    print("Size: " + str(ll.sz))


class BSTNode:
    def __init__(self: "BSTNode", val: int) -> None:
        self.val: int = val
        self.left: int = None
        self.right: int = None


class BST:
    def __init__(self: "BST") -> None:
        self.root: int = None

    def insert(self: "BST", val: int) -> None:
        if self.root is None:
            self.root = BSTNode(val)
            return
        cur = self.root
        while True:
            if val < cur.val:
                if cur.left is None:
                    cur.left = BSTNode(val)
                    return
                cur = cur.left
            else:
                if cur.right is None:
                    cur.right = BSTNode(val)
                    return
                cur = cur.right

    def search(self: "BST", val: int) -> bool:
        cur = self.root
        while cur is not None:
            if cur.val == val:
                return True
            if val < cur.val:
                cur = cur.left
            else:
                cur = cur.right
        return False

    def inorder(self: "BST") -> str:
        result: str = ""
        stack: list = []
        cur = self.root
        while cur is not None or len(stack) > 0:
            while cur is not None:
                stack.append(cur)
                cur = cur.left
            cur = stack.pop()
            if result == "":
                result = str(cur.val)
            else:
                result = result + " " + str(cur.val)
            cur = cur.right
        return result


def test_bst() -> None:
    print("--- BST ---")
    bst: BST = BST()
    bst.insert(5)
    bst.insert(3)
    bst.insert(7)
    bst.insert(1)
    bst.insert(4)
    bst.insert(6)
    bst.insert(8)
    print("Inorder: " + bst.inorder())
    print("Search 4: " + str(bst.search(4)))
    print("Search 9: " + str(bst.search(9)))


class Stack:
    def __init__(self: "Stack") -> None:
        self.data: list = []
        self.top_idx: int = -1

    def push(self: "Stack", val: int) -> None:
        self.data.append(val)
        self.top_idx = self.top_idx + 1

    def pop(self: "Stack") -> int:
        if self.top_idx < 0:
            return -1
        val: int = self.data[self.top_idx]
        self.data.pop()
        self.top_idx = self.top_idx - 1
        return val

    def peek(self: "Stack") -> int:
        if self.top_idx < 0:
            return -1
        return self.data[self.top_idx]

    def is_empty(self: "Stack") -> bool:
        return self.top_idx < 0

    def get_size(self: "Stack") -> int:
        return self.top_idx + 1


class Queue:
    def __init__(self: "Queue") -> None:
        self.data: list = []
        self.front: int = 0

    def enqueue(self: "Queue", val: int) -> None:
        self.data.append(val)

    def dequeue(self: "Queue") -> int:
        if self.front >= len(self.data):
            return -1
        val: int = self.data[self.front]
        self.front = self.front + 1
        return val

    def is_empty(self: "Queue") -> bool:
        return self.front >= len(self.data)

    def get_size(self: "Queue") -> int:
        return len(self.data) - self.front


def test_stack_queue() -> None:
    print("--- Stack ---")
    s: Stack = Stack()
    s.push(1)
    s.push(2)
    s.push(3)
    print("Top: " + str(s.peek()))
    print("Pop: " + str(s.pop()))
    print("Pop: " + str(s.pop()))
    print("Size: " + str(s.get_size()))
    print("Empty: " + str(s.is_empty()))

    print("--- Queue ---")
    q: Queue = Queue()
    q.enqueue(10)
    q.enqueue(20)
    q.enqueue(30)
    print("Dequeue: " + str(q.dequeue()))
    print("Dequeue: " + str(q.dequeue()))
    print("Size: " + str(q.get_size()))
    print("Empty: " + str(q.is_empty()))


class PriorityQueue:
    def __init__(self: "PriorityQueue") -> None:
        self.data: list = []

    def insert(self: "PriorityQueue", val: int) -> None:
        self.data.append(val)
        i: int = len(self.data) - 1
        while i > 0:
            i_minus_1: int = i - 1
            parent: int = i_minus_1 // 2
            if self.data[i] < self.data[parent]:
                tmp: int = self.data[i]
                self.data[i] = self.data[parent]
                self.data[parent] = tmp
                i = parent
            else:
                break

    def extract_min(self: "PriorityQueue") -> int:
        if len(self.data) == 0:
            return -1
        min_val: int = self.data[0]
        last: int = self.data.pop()
        if len(self.data) > 0:
            self.data[0] = last
            i: int = 0
            n: int = len(self.data)
            while True:
                left: int = 2 * i + 1
                right: int = 2 * i + 2
                smallest: int = i
                if left < n and self.data[left] < self.data[smallest]:
                    smallest = left
                if right < n and self.data[right] < self.data[smallest]:
                    smallest = right
                if smallest == i:
                    break
                tmp2: int = self.data[i]
                self.data[i] = self.data[smallest]
                self.data[smallest] = tmp2
                i = smallest
        return min_val

    def get_size(self: "PriorityQueue") -> int:
        return len(self.data)


def test_priority_queue() -> None:
    print("--- PriorityQueue ---")
    pq: PriorityQueue = PriorityQueue()
    pq.insert(5)
    pq.insert(2)
    pq.insert(8)
    pq.insert(1)
    pq.insert(9)
    pq.insert(3)
    result: str = ""
    while pq.get_size() > 0:
        v: int = pq.extract_min()
        if result == "":
            result = str(v)
        else:
            result = result + " " + str(v)
    print("Sorted: " + result)


# ============================================================
# SECTION 2: ALGORITHMS
# ============================================================

def bubble_sort(arr: list, n: int) -> None:
    i: int = 0
    while i < n - 1:
        j: int = 0
        while j < n - 1 - i:
            if arr[j] > arr[j + 1]:
                tmp: int = arr[j]
                arr[j] = arr[j + 1]
                arr[j + 1] = tmp
            j = j + 1
        i = i + 1


def merge_arrays(arr: list, left: int, mid: int, right: int) -> None:
    tmp: list = []
    i: int = left
    j: int = mid + 1
    while i <= mid and j <= right:
        if arr[i] <= arr[j]:
            tmp.append(arr[i])
            i = i + 1
        else:
            tmp.append(arr[j])
            j = j + 1
    while i <= mid:
        tmp.append(arr[i])
        i = i + 1
    while j <= right:
        tmp.append(arr[j])
        j = j + 1
    k: int = 0
    while k < len(tmp):
        arr[left + k] = tmp[k]
        k = k + 1


def merge_sort(arr: list, left: int, right: int) -> None:
    if left >= right:
        return
    lr_sum: int = left + right
    mid: int = lr_sum // 2
    merge_sort(arr, left, mid)
    merge_sort(arr, mid + 1, right)
    merge_arrays(arr, left, mid, right)


def partition(arr: list, low: int, high: int) -> int:
    pivot: int = arr[high]
    i: int = low - 1
    j: int = low
    while j < high:
        if arr[j] <= pivot:
            i = i + 1
            tmp: int = arr[i]
            arr[i] = arr[j]
            arr[j] = tmp
        j = j + 1
    tmp2: int = arr[i + 1]
    arr[i + 1] = arr[high]
    arr[high] = tmp2
    return i + 1


def quicksort(arr: list, low: int, high: int) -> None:
    if low >= high:
        return
    pivot_idx: int = partition(arr, low, high)
    quicksort(arr, low, pivot_idx - 1)
    quicksort(arr, pivot_idx + 1, high)


def binary_search(arr: list, target: int, n: int) -> int:
    lo: int = 0
    hi: int = n - 1
    while lo <= hi:
        lohi_sum: int = lo + hi
        mid: int = lohi_sum // 2
        if arr[mid] == target:
            return mid
        if arr[mid] < target:
            lo = mid + 1
        else:
            hi = mid - 1
    return -1


def linear_search(arr: list, target: int, n: int) -> int:
    i: int = 0
    while i < n:
        if arr[i] == target:
            return i
        i = i + 1
    return -1


def arr_to_string(arr: list, n: int) -> str:
    result: str = "["
    i: int = 0
    while i < n:
        if i > 0:
            result = result + ", "
        result = result + str(arr[i])
        i = i + 1
    result = result + "]"
    return result


def test_sorting_searching() -> None:
    print("--- Sorting & Searching ---")
    a1: list = [64, 25, 12, 22, 11]
    bubble_sort(a1, 5)
    print("Bubble sort: " + arr_to_string(a1, 5))

    a2: list = [38, 27, 43, 3, 9, 82, 10]
    merge_sort(a2, 0, 6)
    print("Merge sort: " + arr_to_string(a2, 7))

    a3: list = [10, 80, 30, 90, 40, 50, 70]
    quicksort(a3, 0, 6)
    print("Quicksort: " + arr_to_string(a3, 7))

    a4: list = [2, 4, 6, 8, 10, 12, 14, 16]
    print("Binary search 10: " + str(binary_search(a4, 10, 8)))
    print("Binary search 7: " + str(binary_search(a4, 7, 8)))
    print("Linear search 6: " + str(linear_search(a4, 6, 8)))
    print("Linear search 99: " + str(linear_search(a4, 99, 8)))


def bfs(graph: list, start: int, n: int) -> str:
    visited: list = []
    i: int = 0
    while i < n:
        visited.append(False)
        i = i + 1
    q: Queue = Queue()
    q.enqueue(start)
    visited[start] = True
    result: str = ""
    while not q.is_empty():
        node: int = q.dequeue()
        if result == "":
            result = str(node)
        else:
            result = result + " " + str(node)
        j: int = 0
        while j < n:
            if graph[node * n + j] == 1 and not visited[j]:
                visited[j] = True
                q.enqueue(j)
            j = j + 1
    return result


def dfs(graph: list, start: int, visited: list, n: int, result: list) -> None:
    visited[start] = True
    result.append(start)
    j: int = 0
    while j < n:
        if graph[start * n + j] == 1 and not visited[j]:
            dfs(graph, j, visited, n, result)
        j = j + 1


def test_graph_algorithms() -> None:
    print("--- Graph BFS/DFS ---")
    n: int = 6
    graph_size: int = n * n
    graph: list = [0] * graph_size
    graph[0 * n + 1] = 1
    graph[1 * n + 0] = 1
    graph[0 * n + 2] = 1
    graph[2 * n + 0] = 1
    graph[1 * n + 3] = 1
    graph[3 * n + 1] = 1
    graph[1 * n + 4] = 1
    graph[4 * n + 1] = 1
    graph[2 * n + 4] = 1
    graph[4 * n + 2] = 1
    graph[3 * n + 5] = 1
    graph[5 * n + 3] = 1
    print("BFS from 0: " + bfs(graph, 0, n))
    visited: list = [False, False, False, False, False, False]
    result: list = []
    dfs(graph, 0, visited, n, result)
    dfs_str: str = ""
    k: int = 0
    while k < len(result):
        if k == 0:
            dfs_str = str(result[k])
        else:
            dfs_str = dfs_str + " " + str(result[k])
        k = k + 1
    print("DFS from 0: " + dfs_str)


def is_palindrome(s: str) -> bool:
    n: int = len(s)
    i: int = 0
    while i < n // 2:
        if s[i] != s[n - 1 - i]:
            return False
        i = i + 1
    return True


def is_anagram(s1: str, s2: str) -> bool:
    if len(s1) != len(s2):
        return False
    counts: list = [0] * 26
    i: int = 0
    while i < len(s1):
        idx1: int = ord(s1[i]) - ord('a')
        if idx1 >= 0 and idx1 < 26:
            counts[idx1] = counts[idx1] + 1
        i = i + 1
    j: int = 0
    while j < len(s2):
        idx2: int = ord(s2[j]) - ord('a')
        if idx2 >= 0 and idx2 < 26:
            counts[idx2] = counts[idx2] - 1
        j = j + 1
    k: int = 0
    while k < 26:
        if counts[k] != 0:
            return False
        k = k + 1
    return True


def string_reverse(s: str) -> str:
    n: int = len(s)
    chars: list = []
    i: int = 0
    while i < n:
        chars.append(s[i])
        i = i + 1
    left: int = 0
    right: int = n - 1
    while left < right:
        tmp: str = chars[left]
        chars[left] = chars[right]
        chars[right] = tmp
        left = left + 1
        right = right - 1
    result: str = ""
    k: int = 0
    while k < n:
        result = result + chars[k]
        k = k + 1
    return result


def test_string_algorithms() -> None:
    print("--- String Algorithms ---")
    print("Is palindrome 'racecar': " + str(is_palindrome("racecar")))
    print("Is palindrome 'hello': " + str(is_palindrome("hello")))
    print("Is anagram 'listen' 'silent': " + str(is_anagram("listen", "silent")))
    print("Is anagram 'hello' 'world': " + str(is_anagram("hello", "world")))
    print("Reverse 'UniLogic': " + string_reverse("UniLogic"))
    print("Reverse 'abcdef': " + string_reverse("abcdef"))


# ============================================================
# SECTION 3: OOP PATTERNS
# ============================================================

class Shape:
    def __init__(self: "Shape", name: str) -> None:
        self.name: str = name

    def area(self: "Shape") -> float:
        return 0.0

    def perimeter(self: "Shape") -> float:
        return 0.0

    def get_name(self: "Shape") -> str:
        return self.name


class Circle(Shape):
    def __init__(self: "Circle", radius: float) -> None:
        super().__init__("Circle")
        self.radius: float = radius

    def area(self: "Circle") -> float:
        return 3.14159 * self.radius * self.radius

    def perimeter(self: "Circle") -> float:
        return 2.0 * 3.14159 * self.radius


class Rectangle(Shape):
    def __init__(self: "Rectangle", width: float, height: float) -> None:
        super().__init__("Rectangle")
        self.width: float = width
        self.height: float = height

    def area(self: "Rectangle") -> float:
        return self.width * self.height

    def perimeter(self: "Rectangle") -> float:
        wh_sum: float = self.width + self.height
        return 2.0 * wh_sum


class Triangle(Shape):
    def __init__(self: "Triangle", a: float, b: float, c: float) -> None:
        super().__init__("Triangle")
        self.a: float = a
        self.b: float = b
        self.c: float = c

    def area(self: "Triangle") -> float:
        abc_sum: float = self.a + self.b + self.c
        s: float = abc_sum / 2.0
        sa: float = s - self.a
        sb: float = s - self.b
        sc: float = s - self.c
        val: float = s * sa * sb * sc
        if val <= 0.0:
            return 0.0
        return sqrt_approx(val)

    def perimeter(self: "Triangle") -> float:
        return self.a + self.b + self.c


def shape_describe(sh: Shape) -> str:
    a: float = sh.area()
    p: float = sh.perimeter()
    a_str: str = str(round(a, 4))
    p_str: str = str(round(p, 4))
    return sh.get_name() + " area=" + a_str + " perimeter=" + p_str


def test_shapes() -> None:
    print("--- Shapes ---")
    shapes: list = []
    c: Circle = Circle(5.0)
    r: Rectangle = Rectangle(4.0, 6.0)
    t: Triangle = Triangle(3.0, 4.0, 5.0)
    shapes.append(c)
    shapes.append(r)
    shapes.append(t)
    i: int = 0
    while i < len(shapes):
        print(shape_describe(shapes[i]))
        i = i + 1


class Animal:
    def __init__(self: "Animal", name: str) -> None:
        self.name: str = name

    def speak(self: "Animal") -> str:
        return self.name + " says ..."


class Dog(Animal):
    def __init__(self: "Dog", name: str) -> None:
        super().__init__(name)

    def speak(self: "Dog") -> str:
        return self.name + " says Woof!"


class Cat(Animal):
    def __init__(self: "Cat", name: str) -> None:
        super().__init__(name)

    def speak(self: "Cat") -> str:
        return self.name + " says Meow!"


class Duck(Animal):
    def __init__(self: "Duck", name: str) -> None:
        super().__init__(name)

    def speak(self: "Duck") -> str:
        return self.name + " says Quack!"


def test_animals() -> None:
    print("--- Animals ---")
    animals: list = []
    d: Dog = Dog("Rex")
    c: Cat = Cat("Whiskers")
    du: Duck = Duck("Donald")
    animals.append(d)
    animals.append(c)
    animals.append(du)
    i: int = 0
    while i < len(animals):
        print(animals[i].speak())
        i = i + 1


class PersonBuilder:
    def __init__(self: "PersonBuilder") -> None:
        self.first_name: str = ""
        self.last_name: str = ""
        self.age: int = 0
        self.email: str = ""

    def set_first(self: "PersonBuilder", name: str) -> int:
        self.first_name = name
        return self

    def set_last(self: "PersonBuilder", name: str) -> int:
        self.last_name = name
        return self

    def set_age(self: "PersonBuilder", age: int) -> int:
        self.age = age
        return self

    def set_email(self: "PersonBuilder", email: str) -> int:
        self.email = email
        return self

    def build(self: "PersonBuilder") -> str:
        return self.first_name + " " + self.last_name + " age=" + str(self.age) + " email=" + self.email


def test_builder() -> None:
    print("--- Builder ---")
    pb: PersonBuilder = PersonBuilder()
    result: str = pb.set_first("Alice").set_last("Smith").set_age(30).set_email("alice@example.com").build()
    print(result)
    pb2: PersonBuilder = PersonBuilder()
    result2: str = pb2.set_first("Bob").set_last("Jones").set_age(25).set_email("bob@example.com").build()
    print(result2)


class EventObserver:
    def __init__(self: "EventObserver", name: str) -> None:
        self.name: str = name
        self.events: list = []

    def on_event(self: "EventObserver", event: str) -> None:
        self.events.append(event)

    def get_events(self: "EventObserver") -> str:
        result: str = ""
        i: int = 0
        while i < len(self.events):
            if i == 0:
                result = self.events[i]
            else:
                result = result + ", " + self.events[i]
            i = i + 1
        return result


class EventPublisher:
    def __init__(self: "EventPublisher") -> None:
        self.observers: list = []

    def subscribe(self: "EventPublisher", obs: int) -> None:
        self.observers.append(obs)

    def publish(self: "EventPublisher", event: str) -> None:
        i: int = 0
        while i < len(self.observers):
            self.observers[i].on_event(event)
            i = i + 1


def test_observer() -> None:
    print("--- Observer ---")
    pub: EventPublisher = EventPublisher()
    obs1: EventObserver = EventObserver("Logger")
    obs2: EventObserver = EventObserver("Notifier")
    pub.subscribe(obs1)
    pub.subscribe(obs2)
    pub.publish("start")
    pub.publish("process")
    pub.publish("stop")
    print("Logger events: " + obs1.get_events())
    print("Notifier events: " + obs2.get_events())


# ============================================================
# SECTION 4: EXCEPTION HANDLING
# ============================================================

class AppError(Exception):
    def __init__(self: "AppError", message: str) -> None:
        self.message: str = message
        super().__init__(message)


class ValidationError(AppError):
    def __init__(self: "ValidationError", message: str) -> None:
        super().__init__("ValidationError: " + message)


class NetworkError(AppError):
    def __init__(self: "NetworkError", message: str) -> None:
        super().__init__("NetworkError: " + message)


class DatabaseError(AppError):
    def __init__(self: "DatabaseError", message: str) -> None:
        super().__init__("DatabaseError: " + message)


def validate_age(age: int) -> int:
    if age < 0:
        raise ValidationError("age must be non-negative")
    if age > 150:
        raise ValidationError("age unrealistic value")
    return age


def fetch_data(url: str) -> str:
    if url == "":
        raise NetworkError("400 empty URL")
    if url == "bad":
        raise NetworkError("404 not found")
    return "data from " + url


def str_contains_sub(haystack: str, needle: str) -> bool:
    hlen: int = len(haystack)
    nlen: int = len(needle)
    if nlen > hlen:
        return False
    i: int = 0
    while i <= hlen - nlen:
        match: bool = True
        j: int = 0
        while j < nlen:
            if haystack[i + j] != needle[j]:
                match = False
                break
            j = j + 1
        if match:
            return True
        i = i + 1
    return False


def query_db(sql: str) -> str:
    if str_contains_sub(sql, "DROP"):
        raise DatabaseError("DROP in query: " + sql)
    return "rows for: " + sql


def test_exceptions() -> None:
    print("--- Exceptions ---")
    try:
        result: int = validate_age(25)
        print("Age 25 valid: " + str(result))
    except ValidationError as e:
        print("Error: " + e.message)

    try:
        result2: int = validate_age(-5)
        print("Should not reach here")
    except ValidationError as e:
        print("Caught: " + e.message)

    try:
        result3: int = validate_age(200)
    except ValidationError as e:
        print("Caught: " + e.message)

    try:
        s: str = fetch_data("http://example.com")
        print("Fetched: " + s)
    except NetworkError as e:
        print("Network error: " + e.message)

    try:
        s2: str = fetch_data("")
    except NetworkError as e:
        print("Caught network: " + e.message)

    try:
        s3: str = fetch_data("bad")
    except NetworkError as e:
        print("Caught network: " + e.message)

    try:
        r: str = query_db("SELECT * FROM users")
        print("DB result: " + r)
    except DatabaseError as e:
        print("DB error: " + e.message)

    try:
        r2: str = query_db("DROP TABLE users")
    except DatabaseError as e:
        print("Caught DB: " + e.message)

    print("Finally test:")
    try:
        raise AppError("test error")
    except AppError as e:
        print("Caught app: " + e.message)
    finally:
        print("Finally executed")

    print("Nested exception handling:")
    try:
        try:
            raise NetworkError("500 server error")
        except ValidationError as e:
            print("Wrong handler")
        except NetworkError as e:
            print("Inner caught: " + e.message)
            raise AppError("re-wrapped: " + e.message)
    except AppError as e:
        print("Outer caught: " + e.message)


# ============================================================
# SECTION 5: STRING PROCESSING
# ============================================================

def parse_csv_line(line: str) -> list:
    fields: list = []
    current: str = ""
    i: int = 0
    while i < len(line):
        c: str = line[i]
        if c == ",":
            fields.append(current)
            current = ""
        else:
            current = current + c
        i = i + 1
    fields.append(current)
    return fields


def csv_to_table(csv_data: int) -> list:
    rows: list = []
    lines: list = split_by_char(csv_data, "\n")
    i: int = 0
    while i < len(lines):
        line: str = lines[i]
        if line != "":
            row: list = parse_csv_line(line)
            rows.append(row)
        i = i + 1
    return rows


def test_csv_parser() -> None:
    print("--- CSV Parser ---")
    csv_str: str = "name,age,city\nAlice,30,London\nBob,25,Paris\nCarol,35,Berlin"
    table: list = csv_to_table(csv_str)
    i: int = 0
    while i < len(table):
        row: list = table[i]
        j: int = 0
        row_str: str = ""
        while j < len(row):
            if j > 0:
                row_str = row_str + " | "
            row_str = row_str + row[j]
            j = j + 1
        print(row_str)
        i = i + 1


def count_words(text: int) -> dict:
    counts: dict = {}
    words: list = split_by_space(text)
    i: int = 0
    while i < len(words):
        word: str = str_to_lower(words[i])
        if word in counts:
            counts[word] = counts[word] + 1
        else:
            counts[word] = 1
        i = i + 1
    return counts


def test_word_frequency() -> None:
    print("--- Word Frequency ---")
    text: str = "the quick brown fox jumps over the lazy dog the fox"
    counts: dict = count_words(text)
    sorted_words: list = sorted(counts.keys())
    i: int = 0
    while i < len(sorted_words):
        w: str = sorted_words[i]
        print(w + ": " + str(counts[w]))
        i = i + 1


def wrap_text(text: int, width: int) -> str:
    words: list = split_by_space(text)
    lines: list = []
    current_line: str = ""
    i: int = 0
    while i < len(words):
        word: str = words[i]
        if current_line == "":
            current_line = word
        elif len(current_line) + 1 + len(word) <= width:
            current_line = current_line + " " + word
        else:
            lines.append(current_line)
            current_line = word
        i = i + 1
    if current_line != "":
        lines.append(current_line)
    result: str = ""
    j: int = 0
    while j < len(lines):
        if j > 0:
            result = result + "\n"
        result = result + lines[j]
        j = j + 1
    return result


def test_text_formatter() -> None:
    print("--- Text Formatter ---")
    text: str = "The quick brown fox jumps over the lazy dog near the river bank"
    wrapped: str = wrap_text(text, 30)
    print("Wrapped at 30:")
    lines: list = split_by_char(wrapped, "\n")
    i: int = 0
    while i < len(lines):
        print(lines[i])
        i = i + 1


# ============================================================
# SECTION 6: MATH
# ============================================================

class Matrix:
    def __init__(self: "Matrix", rows: int, cols: int) -> None:
        self.rows: int = rows
        self.cols: int = cols
        mat_size: int = rows * cols
        self.data: list = [0] * mat_size

    def mat_get(self: "Matrix", r: int, c: int) -> int:
        return self.data[r * self.cols + c]

    def set_val(self: "Matrix", r: int, c: int, val: int) -> None:
        self.data[r * self.cols + c] = val

    def add(self: "Matrix", other: int) -> int:
        mres: Matrix = Matrix(self.rows, self.cols)
        i: int = 0
        while i < self.rows:
            j: int = 0
            while j < self.cols:
                av: int = self.mat_get(i, j)
                bv: int = other.mat_get(i, j)
                mres.set_val(i, j, av + bv)
                j = j + 1
            i = i + 1
        return mres

    def multiply(self: "Matrix", other: int) -> int:
        mres: Matrix = Matrix(self.rows, other.cols)
        i: int = 0
        while i < self.rows:
            j: int = 0
            while j < other.cols:
                s: int = 0
                k: int = 0
                while k < self.cols:
                    av2: int = self.mat_get(i, k)
                    bv2: int = other.mat_get(k, j)
                    s = s + av2 * bv2
                    k = k + 1
                mres.set_val(i, j, s)
                j = j + 1
            i = i + 1
        return mres

    def transpose(self: "Matrix") -> int:
        mres: Matrix = Matrix(self.cols, self.rows)
        i: int = 0
        while i < self.rows:
            j: int = 0
            while j < self.cols:
                tv: int = self.mat_get(i, j)
                mres.set_val(j, i, tv)
                j = j + 1
            i = i + 1
        return mres

    def to_string(self: "Matrix") -> str:
        mat_result: str = ""
        i: int = 0
        while i < self.rows:
            row_str: str = ""
            j: int = 0
            while j < self.cols:
                if j > 0:
                    row_str = row_str + " "
                mat_val: int = self.mat_get(i, j)
                row_str = row_str + str(mat_val)
                j = j + 1
            if i > 0:
                mat_result = mat_result + "\n"
            mat_result = mat_result + row_str
            i = i + 1
        return mat_result


def determinant2x2(a: int, b: int, c: int, d: int) -> int:
    return a * d - b * c


def test_matrix() -> None:
    print("--- Matrix ---")
    m1: Matrix = Matrix(2, 2)
    m1.set_val(0, 0, 1)
    m1.set_val(0, 1, 2)
    m1.set_val(1, 0, 3)
    m1.set_val(1, 1, 4)

    m2: Matrix = Matrix(2, 2)
    m2.set_val(0, 0, 5)
    m2.set_val(0, 1, 6)
    m2.set_val(1, 0, 7)
    m2.set_val(1, 1, 8)

    print("M1:")
    print(m1.to_string())
    print("M2:")
    print(m2.to_string())

    m3: Matrix = m1.add(m2)
    print("M1 + M2:")
    print(m3.to_string())

    m4: Matrix = m1.multiply(m2)
    print("M1 * M2:")
    print(m4.to_string())

    m5: Matrix = m1.transpose()
    print("M1 transposed:")
    print(m5.to_string())

    da: int = m1.mat_get(0, 0)
    db: int = m1.mat_get(0, 1)
    dc: int = m1.mat_get(1, 0)
    dd: int = m1.mat_get(1, 1)
    det: int = determinant2x2(da, db, dc, dd)
    print("Det M1: " + str(det))


class ComplexNumber:
    def __init__(self: "ComplexNumber", real: float, imag: float) -> None:
        self.real: float = real
        self.imag: float = imag

    def add(self: "ComplexNumber", other: int) -> int:
        return ComplexNumber(self.real + other.real, self.imag + other.imag)

    def multiply(self: "ComplexNumber", other: int) -> int:
        r: float = self.real * other.real - self.imag * other.imag
        imag_part: float = self.real * other.imag + self.imag * other.real
        return ComplexNumber(r, imag_part)

    def magnitude(self: "ComplexNumber") -> float:
        return sqrt_approx(self.real * self.real + self.imag * self.imag)

    def conjugate(self: "ComplexNumber") -> int:
        return ComplexNumber(self.real, -self.imag)

    def to_string(self: "ComplexNumber") -> str:
        if self.imag >= 0.0:
            return str(round(self.real, 4)) + "+" + str(round(self.imag, 4)) + "i"
        return str(round(self.real, 4)) + str(round(self.imag, 4)) + "i"


def test_complex() -> None:
    print("--- Complex Numbers ---")
    c1: ComplexNumber = ComplexNumber(3.0, 4.0)
    c2: ComplexNumber = ComplexNumber(1.0, -2.0)
    print("c1 = " + c1.to_string())
    print("c2 = " + c2.to_string())
    print("c1 + c2 = " + c1.add(c2).to_string())
    print("c1 * c2 = " + c1.multiply(c2).to_string())
    print("|c1| = " + str(round(c1.magnitude(), 4)))
    print("conj(c1) = " + c1.conjugate().to_string())


def calc_mean(data: list) -> float:
    total: float = 0.0
    i: int = 0
    while i < len(data):
        total = total + data[i]
        i = i + 1
    return total / len(data)


def calc_median(data: list) -> float:
    n: int = len(data)
    sorted_data: list = sorted(data)
    mid: int = n // 2
    if n % 2 == 0:
        mid_minus_1: int = mid - 1
        pair_sum: float = sorted_data[mid_minus_1] + sorted_data[mid]
        return pair_sum / 2.0
    return float(sorted_data[mid])


def calc_mode(data: list) -> int:
    counts: dict = {}
    i: int = 0
    while i < len(data):
        val: int = data[i]
        if val in counts:
            counts[val] = counts[val] + 1
        else:
            counts[val] = 1
        i = i + 1
    max_count: int = 0
    mode_val: int = data[0]
    keys: list = sorted(counts.keys())
    j: int = 0
    while j < len(keys):
        k: int = keys[j]
        if counts[k] > max_count:
            max_count = counts[k]
            mode_val = k
        j = j + 1
    return mode_val


def calc_variance(data: list) -> float:
    mean: float = calc_mean(data)
    total: float = 0.0
    i: int = 0
    while i < len(data):
        diff: float = data[i] - mean
        total = total + diff * diff
        i = i + 1
    return total / len(data)


def calc_std_dev(data: list) -> float:
    return sqrt_approx(calc_variance(data))


def test_statistics() -> None:
    print("--- Statistics ---")
    data: list = [4, 7, 13, 2, 8, 4, 5, 4, 12, 9]
    print("Data: " + arr_to_string(data, len(data)))
    print("Mean: " + str(round(calc_mean(data), 4)))
    print("Median: " + str(round(calc_median(data), 4)))
    print("Mode: " + str(calc_mode(data)))
    print("Variance: " + str(round(calc_variance(data), 4)))
    print("Std Dev: " + str(round(calc_std_dev(data), 4)))


# ============================================================
# SECTION 7: MORE OOP — STATE MACHINE
# ============================================================

class TrafficLight:
    def __init__(self: "TrafficLight") -> None:
        self.state: str = "RED"
        self.cycles: int = 0

    def next_state(self: "TrafficLight") -> None:
        if self.state == "RED":
            self.state = "GREEN"
        elif self.state == "GREEN":
            self.state = "YELLOW"
        elif self.state == "YELLOW":
            self.state = "RED"
            self.cycles = self.cycles + 1

    def get_state(self: "TrafficLight") -> str:
        return self.state


def test_state_machine() -> None:
    print("--- State Machine ---")
    light: TrafficLight = TrafficLight()
    i: int = 0
    while i < 9:
        print("State: " + light.get_state())
        light.next_state()
        i = i + 1
    print("Cycles: " + str(light.cycles))


# ============================================================
# SECTION 8: RECURSION
# ============================================================

def fibonacci(n: int) -> int:
    if n <= 1:
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)


def factorial(n: int) -> int:
    if n <= 1:
        return 1
    return n * factorial(n - 1)


def power(base: int, exp: int) -> int:
    if exp == 0:
        return 1
    if exp % 2 == 0:
        half: int = power(base, exp // 2)
        return half * half
    return base * power(base, exp - 1)


def gcd(a: int, b: int) -> int:
    if b == 0:
        return a
    return gcd(b, a % b)


def sum_digits(n: int) -> int:
    if n < 10:
        return n
    return n % 10 + sum_digits(n // 10)


def test_recursion() -> None:
    print("--- Recursion ---")
    fib_str: str = ""
    i: int = 0
    while i < 10:
        if i > 0:
            fib_str = fib_str + " "
        fib_str = fib_str + str(fibonacci(i))
        i = i + 1
    print("Fibonacci: " + fib_str)
    print("10! = " + str(factorial(10)))
    print("2^10 = " + str(power(2, 10)))
    print("GCD(48,18) = " + str(gcd(48, 18)))
    print("Sum digits 12345 = " + str(sum_digits(12345)))


# ============================================================
# SECTION 9: STRING MANIPULATION
# ============================================================

def count_char(s: str, c: str) -> int:
    count: int = 0
    i: int = 0
    while i < len(s):
        if s[i] == c:
            count = count + 1
        i = i + 1
    return count


def replace_char(s: str, old: str, new_char: str) -> str:
    result: str = ""
    i: int = 0
    while i < len(s):
        if s[i] == old:
            result = result + new_char
        else:
            result = result + s[i]
        i = i + 1
    return result


def starts_with_str(s: str, prefix: str) -> bool:
    if len(prefix) > len(s):
        return False
    i: int = 0
    while i < len(prefix):
        if s[i] != prefix[i]:
            return False
        i = i + 1
    return True


def ends_with_str(s: str, suffix: str) -> bool:
    if len(suffix) > len(s):
        return False
    offset: int = len(s) - len(suffix)
    i: int = 0
    while i < len(suffix):
        if s[offset + i] != suffix[i]:
            return False
        i = i + 1
    return True


def trim_string(s: str) -> str:
    start: int = 0
    while start < len(s) and s[start] == " ":
        start = start + 1
    end: int = len(s) - 1
    while end >= start and s[end] == " ":
        end = end - 1
    result: str = ""
    i: int = start
    while i <= end:
        result = result + s[i]
        i = i + 1
    return result


def to_upper(s: str) -> str:
    result: str = ""
    i: int = 0
    while i < len(s):
        c: str = s[i]
        code: int = ord(c)
        if code >= 97 and code <= 122:
            result = result + chr(code - 32)
        else:
            result = result + c
        i = i + 1
    return result


def to_lower(s: str) -> str:
    result: str = ""
    i: int = 0
    while i < len(s):
        c: str = s[i]
        code: int = ord(c)
        if code >= 65 and code <= 90:
            result = result + chr(code + 32)
        else:
            result = result + c
        i = i + 1
    return result


def test_string_manipulation() -> None:
    print("--- String Manipulation ---")
    s: str = "  Hello World  "
    print("Trimmed: '" + trim_string(s) + "'")
    print("Upper: " + to_upper("hello world"))
    print("Lower: " + to_lower("HELLO WORLD"))
    print("Count 'l' in 'hello world': " + str(count_char("hello world", "l")))
    print("Replace 'o' with '0' in 'hello world': " + replace_char("hello world", "o", "0"))
    print("Starts with 'hel': " + str(starts_with_str("hello", "hel")))
    print("Starts with 'abc': " + str(starts_with_str("hello", "abc")))
    print("Ends with 'llo': " + str(ends_with_str("hello", "llo")))
    print("Ends with 'abc': " + str(ends_with_str("hello", "abc")))


# ============================================================
# SECTION 10: MIXED DATA PROCESSING
# ============================================================

class Student:
    def __init__(self: "Student", name: str, grade: int, score: float) -> None:
        self.name: str = name
        self.grade: int = grade
        self.score: float = score

    def to_string(self: "Student") -> str:
        return self.name + " grade=" + str(self.grade) + " score=" + str(self.score)


def sort_students_by_score(students: list, n: int) -> None:
    i: int = 0
    while i < n - 1:
        j: int = 0
        while j < n - 1 - i:
            s1 = students[j]
            s2 = students[j + 1]
            if s1.score < s2.score:
                students[j] = s2
                students[j + 1] = s1
            j = j + 1
        i = i + 1


def average_score(students: list, n: int) -> float:
    total: float = 0.0
    i: int = 0
    while i < n:
        total = total + students[i].score
        i = i + 1
    return total / n


def test_data_processing() -> None:
    print("--- Data Processing ---")
    students: list = []
    students.append(Student("Alice", 10, 92.5))
    students.append(Student("Bob", 11, 78.0))
    students.append(Student("Carol", 10, 88.5))
    students.append(Student("Dave", 12, 95.0))
    students.append(Student("Eve", 11, 82.0))

    n: int = len(students)
    print("Before sort:")
    i: int = 0
    while i < n:
        print("  " + students[i].to_string())
        i = i + 1

    sort_students_by_score(students, n)
    print("After sort by score desc:")
    j: int = 0
    while j < n:
        print("  " + students[j].to_string())
        j = j + 1

    avg: float = average_score(students, n)
    print("Average score: " + str(round(avg, 2)))


# ============================================================
# SECTION 11: NUMBER THEORY
# ============================================================

def is_prime(n: int) -> bool:
    if n < 2:
        return False
    if n == 2:
        return True
    if n % 2 == 0:
        return False
    i: int = 3
    while i * i <= n:
        if n % i == 0:
            return False
        i = i + 2
    return True


def sieve_of_eratosthenes(limit: int) -> list:
    sieve_size: int = limit + 1
    is_p: list = [True] * sieve_size
    is_p[0] = False
    if limit >= 1:
        is_p[1] = False
    i: int = 2
    while i * i <= limit:
        if is_p[i]:
            j: int = i * i
            while j <= limit:
                is_p[j] = False
                j = j + i
        i = i + 1
    primes: list = []
    k: int = 2
    while k <= limit:
        if is_p[k]:
            primes.append(k)
        k = k + 1
    return primes


def prime_factors(n: int) -> list:
    factors: list = []
    d: int = 2
    while d * d <= n:
        while n % d == 0:
            factors.append(d)
            n = n // d
        d = d + 1
    if n > 1:
        factors.append(n)
    return factors


def test_number_theory() -> None:
    print("--- Number Theory ---")
    primes: list = sieve_of_eratosthenes(50)
    primes_str: str = ""
    i: int = 0
    while i < len(primes):
        if i > 0:
            primes_str = primes_str + " "
        primes_str = primes_str + str(primes[i])
        i = i + 1
    print("Primes <= 50: " + primes_str)
    print("Is prime 97: " + str(is_prime(97)))
    print("Is prime 100: " + str(is_prime(100)))

    factors: list = prime_factors(360)
    factors_str: str = ""
    j: int = 0
    while j < len(factors):
        if j > 0:
            factors_str = factors_str + " "
        factors_str = factors_str + str(factors[j])
        j = j + 1
    print("Prime factors of 360: " + factors_str)


# ============================================================
# SECTION 12: STACK-BASED EXPRESSION EVALUATOR
# ============================================================

def evaluate_rpn(tokens: list) -> float:
    stk: list = []
    i: int = 0
    while i < len(tokens):
        tok: str = tokens[i]
        if tok == "+":
            b: float = stk.pop()
            a: float = stk.pop()
            stk.append(a + b)
        elif tok == "-":
            b2: float = stk.pop()
            a2: float = stk.pop()
            stk.append(a2 - b2)
        elif tok == "*":
            b3: float = stk.pop()
            a3: float = stk.pop()
            stk.append(a3 * b3)
        elif tok == "/":
            b4: float = stk.pop()
            a4: float = stk.pop()
            if b4 == 0.0:
                stk.append(0.0)
            else:
                stk.append(a4 / b4)
        else:
            stk.append(float(tok))
        i = i + 1
    if len(stk) == 0:
        return 0.0
    return stk[0]


def test_rpn_evaluator() -> None:
    print("--- RPN Evaluator ---")
    expr1: list = ["3", "4", "+", "2", "*"]
    print("(3+4)*2 = " + str(evaluate_rpn(expr1)))
    expr2: list = ["15", "7", "1", "1", "+", "-", "/", "3", "*", "2", "1", "1", "+", "+", "-"]
    print("Complex RPN = " + str(evaluate_rpn(expr2)))
    expr3: list = ["2", "3", "4", "*", "+"]
    print("2+3*4 = " + str(evaluate_rpn(expr3)))


# ============================================================
# MAIN
# ============================================================

def main() -> None:
    print("=== GIANT PYTHON TEST ===")
    print("")

    print("== SECTION 1: DATA STRUCTURES ==")
    test_linked_list()
    test_bst()
    test_stack_queue()
    test_priority_queue()
    print("")

    print("== SECTION 2: ALGORITHMS ==")
    test_sorting_searching()
    test_graph_algorithms()
    test_string_algorithms()
    print("")

    print("== SECTION 3: OOP PATTERNS ==")
    test_shapes()
    test_animals()
    test_builder()
    test_observer()
    print("")

    print("== SECTION 4: EXCEPTION HANDLING ==")
    test_exceptions()
    print("")

    print("== SECTION 5: STRING PROCESSING ==")
    test_csv_parser()
    test_word_frequency()
    test_text_formatter()
    print("")

    print("== SECTION 6: MATH ==")
    test_matrix()
    test_complex()
    test_statistics()
    print("")

    print("== SECTION 7: STATE MACHINE ==")
    test_state_machine()
    print("")

    print("== SECTION 8: RECURSION ==")
    test_recursion()
    print("")

    print("== SECTION 9: STRING MANIPULATION ==")
    test_string_manipulation()
    print("")

    print("== SECTION 10: DATA PROCESSING ==")
    test_data_processing()
    print("")

    print("== SECTION 11: NUMBER THEORY ==")
    test_number_theory()
    print("")

    print("== SECTION 12: RPN EVALUATOR ==")
    test_rpn_evaluator()
    print("")

    print("=== ALL TESTS COMPLETE ===")


if __name__ == "__main__":
    main()
