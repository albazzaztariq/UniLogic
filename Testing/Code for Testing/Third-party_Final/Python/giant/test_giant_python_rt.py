# @dr python_compat = true


def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

# python_compat type aliases (UL keywords remapped by py2ul)
_py_list = list
_py_map = dict
_py_type = type
_py_int = int
_py_float = float
_py_string = str
_py_bool = bool
_py_object = object

class Node:
    def __init__(self, val):
        self.val = val
        self.next = None

class LinkedList:
    def __init__(self):
        self.head = None
        self.sz = 0
    def insert(self, val):
        n = Node(val)
        n.next = self.head
        self.head = n
        self.sz = (self.sz + 1)
    def find(self, val):
        cur = self.head
        while (cur != None):
            if (cur.val == val):
                return True
            cur = cur.next
        return False
    def delete(self, val):
        if (self.head == None):
            return False
        if (self.head.val == val):
            self.head = self.head.next
            self.sz = (self.sz - 1)
            return True
        cur = self.head
        while (cur.next != None):
            if (cur.next.val == val):
                cur.next = cur.next.next
                self.sz = (self.sz - 1)
                return True
            cur = cur.next
        return False
    def to_string(self):
        parts = ''
        cur = self.head
        first = True
        while (cur != None):
            if first:
                parts = str(cur.val)
                first = False
            else:
                parts = ((parts + ' -> ') + str(cur.val))
            cur = cur.next
        return parts
    def reverse(self):
        prev = None
        cur = self.head
        while (cur != None):
            nxt = cur.next
            cur.next = prev
            prev = cur
            cur = nxt
        self.head = prev

class BSTNode:
    def __init__(self, val):
        self.val = val
        self.left = None
        self.right = None

class BST:
    def __init__(self):
        self.root = None
    def insert(self, val):
        if (self.root == None):
            self.root = BSTNode(val)
            return
        cur = self.root
        while True:
            if (val < cur.val):
                if (cur.left == None):
                    cur.left = BSTNode(val)
                    return
                cur = cur.left
            else:
                if (cur.right == None):
                    cur.right = BSTNode(val)
                    return
                cur = cur.right
    def search(self, val):
        cur = self.root
        while (cur != None):
            if (cur.val == val):
                return True
            if (val < cur.val):
                cur = cur.left
            else:
                cur = cur.right
        return False
    def inorder(self):
        result = ''
        stack = []
        cur = self.root
        while ((cur != None) or (len(stack) > 0)):
            while (cur != None):
                stack.append(cur)
                cur = cur.left
            cur = stack.pop()
            if (result == ''):
                result = str(cur.val)
            else:
                result = ((result + ' ') + str(cur.val))
            cur = cur.right
        return result

class Stack:
    def __init__(self):
        self.data = []
        self.top_idx = (-1)
    def push(self, val):
        self.data.append(val)
        self.top_idx = (self.top_idx + 1)
    def pop(self):
        if (self.top_idx < 0):
            return (-1)
        val = self.data[self.top_idx]
        self.data.pop()
        self.top_idx = (self.top_idx - 1)
        return val
    def peek(self):
        if (self.top_idx < 0):
            return (-1)
        return self.data[self.top_idx]
    def is_empty(self):
        return (self.top_idx < 0)
    def get_size(self):
        return (self.top_idx + 1)

class Queue:
    def __init__(self):
        self.data = []
        self.front = 0
    def enqueue(self, val):
        self.data.append(val)
    def dequeue(self):
        if (self.front >= len(self.data)):
            return (-1)
        val = self.data[self.front]
        self.front = (self.front + 1)
        return val
    def is_empty(self):
        return (self.front >= len(self.data))
    def get_size(self):
        return (len(self.data) - self.front)

class PriorityQueue:
    def __init__(self):
        self.data = []
    def insert(self, val):
        self.data.append(val)
        i = (len(self.data) - 1)
        while (i > 0):
            i_minus_1 = (i - 1)
            parent = (i_minus_1 // 2)
            if (self.data[i] < self.data[parent]):
                tmp = self.data[i]
                self.data[i] = self.data[parent]
                self.data[parent] = tmp
                i = parent
            else:
                break
    def extract_min(self):
        if (len(self.data) == 0):
            return (-1)
        min_val = self.data[0]
        last = self.data.pop()
        if (len(self.data) > 0):
            self.data[0] = last
            i = 0
            n = len(self.data)
            while True:
                _py_left = ((2 * i) + 1)
                _py_right = ((2 * i) + 2)
                smallest = i
                if ((_py_left < n) and (self.data[_py_left] < self.data[smallest])):
                    smallest = _py_left
                if ((_py_right < n) and (self.data[_py_right] < self.data[smallest])):
                    smallest = _py_right
                if (smallest == i):
                    break
                tmp2 = self.data[i]
                self.data[i] = self.data[smallest]
                self.data[smallest] = tmp2
                i = smallest
        return min_val
    def get_size(self):
        return len(self.data)

class Shape:
    def __init__(self, name):
        self.name = name
    def area(self):
        return 0.0
    def perimeter(self):
        return 0.0
    def get_name(self):
        return self.name

class Circle(Shape):
    def __init__(self, radius):
        super().__init__('Circle')
        self.radius = radius
    def area(self):
        return ((3.14159 * self.radius) * self.radius)
    def perimeter(self):
        return ((2.0 * 3.14159) * self.radius)

class Rectangle(Shape):
    def __init__(self, width, height):
        super().__init__('Rectangle')
        self.width = width
        self.height = height
    def area(self):
        return (self.width * self.height)
    def perimeter(self):
        wh_sum = (self.width + self.height)
        return (2.0 * wh_sum)

class Triangle(Shape):
    def __init__(self, a, b, c):
        super().__init__('Triangle')
        self.a = a
        self.b = b
        self.c = c
    def area(self):
        abc_sum = ((self.a + self.b) + self.c)
        s = (abc_sum / 2.0)
        sa = (s - self.a)
        sb = (s - self.b)
        sc = (s - self.c)
        val = (((s * sa) * sb) * sc)
        if (val <= 0.0):
            return 0.0
        return sqrt_approx(val)
    def perimeter(self):
        return ((self.a + self.b) + self.c)

class Animal:
    def __init__(self, name):
        self.name = name
    def speak(self):
        return (self.name + ' says ...')

class Dog(Animal):
    def __init__(self, name):
        super().__init__(name)
    def speak(self):
        return (self.name + ' says Woof!')

class Cat(Animal):
    def __init__(self, name):
        super().__init__(name)
    def speak(self):
        return (self.name + ' says Meow!')

class Duck(Animal):
    def __init__(self, name):
        super().__init__(name)
    def speak(self):
        return (self.name + ' says Quack!')

class PersonBuilder:
    def __init__(self):
        self.first_name = ''
        self.last_name = ''
        self.age = 0
        self.email = ''
    def set_first(self, name):
        self.first_name = name
        return self
    def set_last(self, name):
        self.last_name = name
        return self
    def set_age(self, age):
        self.age = age
        return self
    def set_email(self, email):
        self.email = email
        return self
    def build(self):
        return ((((((self.first_name + ' ') + self.last_name) + ' age=') + str(self.age)) + ' email=') + self.email)

class EventObserver:
    def __init__(self, name):
        self.name = name
        self.events = []
    def on_event(self, event):
        self.events.append(event)
    def get_events(self):
        result = ''
        i = 0
        while (i < len(self.events)):
            if (i == 0):
                result = self.events[i]
            else:
                result = ((result + ', ') + self.events[i])
            i = (i + 1)
        return result

class EventPublisher:
    def __init__(self):
        self.observers = []
    def subscribe(self, obs):
        self.observers.append(obs)
    def publish(self, event):
        i = 0
        while (i < len(self.observers)):
            self.observers[i].on_event(event)
            i = (i + 1)

class AppError(Exception):
    def __init__(self, message):
        self.message = message
        super().__init__(message)

class ValidationError(AppError):
    def __init__(self, message):
        super().__init__(('ValidationError: ' + message))

class NetworkError(AppError):
    def __init__(self, message):
        super().__init__(('NetworkError: ' + message))

class DatabaseError(AppError):
    def __init__(self, message):
        super().__init__(('DatabaseError: ' + message))

class Matrix:
    def __init__(self, rows, cols):
        self.rows = rows
        self.cols = cols
        mat_size = (rows * cols)
        self.data = ([0] * mat_size)
    def mat_get(self, r, c):
        return self.data[((r * self.cols) + c)]
    def set_val(self, r, c, val):
        self.data[((r * self.cols) + c)] = val
    def add(self, other):
        Matrix
        mres = Matrix(self.rows, self.cols)
        i = 0
        while (i < self.rows):
            j = 0
            while (j < self.cols):
                av = self.mat_get(i, j)
                bv = other.mat_get(i, j)
                mres.set_val(i, j, (av + bv))
                j = (j + 1)
            i = (i + 1)
        return mres
    def multiply(self, other):
        Matrix
        mres = Matrix(self.rows, other.cols)
        i = 0
        while (i < self.rows):
            j = 0
            while (j < other.cols):
                s = 0
                k = 0
                while (k < self.cols):
                    av2 = self.mat_get(i, k)
                    bv2 = other.mat_get(k, j)
                    s = (s + (av2 * bv2))
                    k = (k + 1)
                mres.set_val(i, j, s)
                j = (j + 1)
            i = (i + 1)
        return mres
    def transpose(self):
        Matrix
        mres = Matrix(self.cols, self.rows)
        i = 0
        while (i < self.rows):
            j = 0
            while (j < self.cols):
                tv = self.mat_get(i, j)
                mres.set_val(j, i, tv)
                j = (j + 1)
            i = (i + 1)
        return mres
    def to_string(self):
        mat_result = ''
        i = 0
        while (i < self.rows):
            row_str = ''
            j = 0
            while (j < self.cols):
                if (j > 0):
                    row_str = (row_str + ' ')
                mat_val = self.mat_get(i, j)
                row_str = (row_str + str(mat_val))
                j = (j + 1)
            if (i > 0):
                mat_result = (mat_result + '\n')
            mat_result = (mat_result + row_str)
            i = (i + 1)
        return mat_result

class ComplexNumber:
    def __init__(self, real, imag):
        self.real = real
        self.imag = imag
    def add(self, other):
        return ComplexNumber((self.real + other.real), (self.imag + other.imag))
    def multiply(self, other):
        r = ((self.real * other.real) - (self.imag * other.imag))
        imag_part = ((self.real * other.imag) + (self.imag * other.real))
        return ComplexNumber(r, imag_part)
    def magnitude(self):
        return sqrt_approx(((self.real * self.real) + (self.imag * self.imag)))
    def conjugate(self):
        return ComplexNumber(self.real, (-self.imag))
    def to_string(self):
        if (self.imag >= 0.0):
            return (((str(round(self.real, 4)) + '+') + str(round(self.imag, 4))) + 'i')
        return ((str(round(self.real, 4)) + str(round(self.imag, 4))) + 'i')

class TrafficLight:
    def __init__(self):
        self.state = 'RED'
        self.cycles = 0
    def next_state(self):
        if (self.state == 'RED'):
            self.state = 'GREEN'
        else:
            if (self.state == 'GREEN'):
                self.state = 'YELLOW'
            else:
                if (self.state == 'YELLOW'):
                    self.state = 'RED'
                    self.cycles = (self.cycles + 1)
    def get_state(self):
        return self.state

class Student:
    def __init__(self, name, grade, score):
        self.name = name
        self.grade = grade
        self.score = score
    def to_string(self):
        return ((((self.name + ' grade=') + str(self.grade)) + ' score=') + str(self.score))


def split_by_char(s, sep):
    parts = []
    current = ''
    i = 0
    while (i < len(s)):
        c = s[i]
        if (c == sep):
            parts.append(current)
            current = ''
        else:
            current = (current + c)
        i = (i + 1)
    parts.append(current)
    return parts

def split_by_space(s):
    words = []
    current = ''
    i = 0
    while (i < len(s)):
        c = s[i]
        if (((c == ' ') or (c == '\t')) or (c == '\n')):
            if (current != ''):
                words.append(current)
                current = ''
        else:
            current = (current + c)
        i = (i + 1)
    if (current != ''):
        words.append(current)
    return words

def char_to_lower(c):
    code = ord(c[0])
    if ((code >= 65) and (code <= 90)):
        return chr((code + 32))
    return c

def str_to_lower(s):
    result = ''
    i = 0
    while (i < len(s)):
        result = (result + char_to_lower(s[i]))
        i = (i + 1)
    return result

def sqrt_approx(x):
    if (x <= 0.0):
        return 0.0
    guess = (x / 2.0)
    i = 0
    while (i < 50):
        gx = (x / guess)
        ng_sum = (guess + gx)
        new_guess = (ng_sum / 2.0)
        _py_diff = (new_guess - guess)
        if (_py_diff < 0.0):
            _py_diff = (-_py_diff)
        if (_py_diff < 0.0001):
            break
        guess = new_guess
        i = (i + 1)
    return new_guess

def test_linked_list():
    print('--- LinkedList ---')
    ll = LinkedList()
    ll.insert(10)
    ll.insert(20)
    ll.insert(30)
    ll.insert(40)
    print(('List: ' + ll.to_string()))
    print(('Find 20: ' + str(ll.find(20))))
    print(('Find 99: ' + str(ll.find(99))))
    ll.delete(20)
    print(('After delete 20: ' + ll.to_string()))
    ll.reverse()
    print(('Reversed: ' + ll.to_string()))
    print(('Size: ' + str(ll.sz)))

def test_bst():
    print('--- BST ---')
    bst = BST()
    bst.insert(5)
    bst.insert(3)
    bst.insert(7)
    bst.insert(1)
    bst.insert(4)
    bst.insert(6)
    bst.insert(8)
    print(('Inorder: ' + bst.inorder()))
    print(('Search 4: ' + str(bst.search(4))))
    print(('Search 9: ' + str(bst.search(9))))

def test_stack_queue():
    print('--- Stack ---')
    s = Stack()
    s.push(1)
    s.push(2)
    s.push(3)
    print(('Top: ' + str(s.peek())))
    print(('Pop: ' + str(s.pop())))
    print(('Pop: ' + str(s.pop())))
    print(('Size: ' + str(s.get_size())))
    print(('Empty: ' + str(s.is_empty())))
    print('--- Queue ---')
    q = Queue()
    q.enqueue(10)
    q.enqueue(20)
    q.enqueue(30)
    print(('Dequeue: ' + str(q.dequeue())))
    print(('Dequeue: ' + str(q.dequeue())))
    print(('Size: ' + str(q.get_size())))
    print(('Empty: ' + str(q.is_empty())))

def test_priority_queue():
    print('--- PriorityQueue ---')
    pq = PriorityQueue()
    pq.insert(5)
    pq.insert(2)
    pq.insert(8)
    pq.insert(1)
    pq.insert(9)
    pq.insert(3)
    result = ''
    while (pq.get_size() > 0):
        v = pq.extract_min()
        if (result == ''):
            result = str(v)
        else:
            result = ((result + ' ') + str(v))
    print(('Sorted: ' + result))

def bubble_sort(arr, n):
    i = 0
    while (i < (n - 1)):
        j = 0
        while (j < ((n - 1) - i)):
            if (arr[j] > arr[(j + 1)]):
                tmp = arr[j]
                arr[j] = arr[(j + 1)]
                arr[(j + 1)] = tmp
            j = (j + 1)
        i = (i + 1)

def merge_arrays(arr, _py_left, mid, _py_right):
    tmp = []
    i = _py_left
    j = (mid + 1)
    while ((i <= mid) and (j <= _py_right)):
        if (arr[i] <= arr[j]):
            tmp.append(arr[i])
            i = (i + 1)
        else:
            tmp.append(arr[j])
            j = (j + 1)
    while (i <= mid):
        tmp.append(arr[i])
        i = (i + 1)
    while (j <= _py_right):
        tmp.append(arr[j])
        j = (j + 1)
    k = 0
    while (k < len(tmp)):
        arr[(_py_left + k)] = tmp[k]
        k = (k + 1)

def merge_sort(arr, _py_left, _py_right):
    if (_py_left >= _py_right):
        return
    lr_sum = (_py_left + _py_right)
    mid = (lr_sum // 2)
    merge_sort(arr, _py_left, mid)
    merge_sort(arr, (mid + 1), _py_right)
    merge_arrays(arr, _py_left, mid, _py_right)

def partition(arr, low, high):
    pivot = arr[high]
    i = (low - 1)
    j = low
    while (j < high):
        if (arr[j] <= pivot):
            i = (i + 1)
            tmp = arr[i]
            arr[i] = arr[j]
            arr[j] = tmp
        j = (j + 1)
    tmp2 = arr[(i + 1)]
    arr[(i + 1)] = arr[high]
    arr[high] = tmp2
    return (i + 1)

def quicksort(arr, low, high):
    if (low >= high):
        return
    pivot_idx = partition(arr, low, high)
    quicksort(arr, low, (pivot_idx - 1))
    quicksort(arr, (pivot_idx + 1), high)

def binary_search(arr, target, n):
    lo = 0
    hi = (n - 1)
    while (lo <= hi):
        lohi_sum = (lo + hi)
        mid = (lohi_sum // 2)
        if (arr[mid] == target):
            return mid
        if (arr[mid] < target):
            lo = (mid + 1)
        else:
            hi = (mid - 1)
    return (-1)

def linear_search(arr, target, n):
    i = 0
    while (i < n):
        if (arr[i] == target):
            return i
        i = (i + 1)
    return (-1)

def arr_to_string(arr, n):
    result = '['
    i = 0
    while (i < n):
        if (i > 0):
            result = (result + ', ')
        result = (result + str(arr[i]))
        i = (i + 1)
    result = (result + ']')
    return result

def test_sorting_searching():
    print('--- Sorting & Searching ---')
    a1 = [64, 25, 12, 22, 11]
    bubble_sort(a1, 5)
    print(('Bubble sort: ' + arr_to_string(a1, 5)))
    a2 = [38, 27, 43, 3, 9, 82, 10]
    merge_sort(a2, 0, 6)
    print(('Merge sort: ' + arr_to_string(a2, 7)))
    a3 = [10, 80, 30, 90, 40, 50, 70]
    quicksort(a3, 0, 6)
    print(('Quicksort: ' + arr_to_string(a3, 7)))
    a4 = [2, 4, 6, 8, 10, 12, 14, 16]
    print(('Binary search 10: ' + str(binary_search(a4, 10, 8))))
    print(('Binary search 7: ' + str(binary_search(a4, 7, 8))))
    print(('Linear search 6: ' + str(linear_search(a4, 6, 8))))
    print(('Linear search 99: ' + str(linear_search(a4, 99, 8))))

def bfs(graph, start, n):
    visited = []
    i = 0
    while (i < n):
        visited.append(False)
        i = (i + 1)
    q = Queue()
    q.enqueue(start)
    visited[start] = True
    result = ''
    while (not q.is_empty()):
        node = q.dequeue()
        if (result == ''):
            result = str(node)
        else:
            result = ((result + ' ') + str(node))
        j = 0
        while (j < n):
            if ((graph[((node * n) + j)] == 1) and (not visited[j])):
                visited[j] = True
                q.enqueue(j)
            j = (j + 1)
    return result

def dfs(graph, start, visited, n, result):
    visited[start] = True
    result.append(start)
    j = 0
    while (j < n):
        if ((graph[((start * n) + j)] == 1) and (not visited[j])):
            dfs(graph, j, visited, n, result)
        j = (j + 1)

def test_graph_algorithms():
    print('--- Graph BFS/DFS ---')
    n = 6
    graph_size = (n * n)
    graph = ([0] * graph_size)
    graph[((0 * n) + 1)] = 1
    graph[((1 * n) + 0)] = 1
    graph[((0 * n) + 2)] = 1
    graph[((2 * n) + 0)] = 1
    graph[((1 * n) + 3)] = 1
    graph[((3 * n) + 1)] = 1
    graph[((1 * n) + 4)] = 1
    graph[((4 * n) + 1)] = 1
    graph[((2 * n) + 4)] = 1
    graph[((4 * n) + 2)] = 1
    graph[((3 * n) + 5)] = 1
    graph[((5 * n) + 3)] = 1
    print(('BFS from 0: ' + bfs(graph, 0, n)))
    visited = [False, False, False, False, False, False]
    result = []
    dfs(graph, 0, visited, n, result)
    dfs_str = ''
    k = 0
    while (k < len(result)):
        if (k == 0):
            dfs_str = str(result[k])
        else:
            dfs_str = ((dfs_str + ' ') + str(result[k]))
        k = (k + 1)
    print(('DFS from 0: ' + dfs_str))

def is_palindrome(s):
    n = len(s)
    i = 0
    while (i < (n // 2)):
        if (s[i] != s[((n - 1) - i)]):
            return False
        i = (i + 1)
    return True

def is_anagram(s1, s2):
    if (len(s1) != len(s2)):
        return False
    counts = ([0] * 26)
    i = 0
    while (i < len(s1)):
        idx1 = (ord(s1[i]) - ord('a'[0]))
        if ((idx1 >= 0) and (idx1 < 26)):
            counts[idx1] = (counts[idx1] + 1)
        i = (i + 1)
    j = 0
    while (j < len(s2)):
        idx2 = (ord(s2[j]) - ord('a'[0]))
        if ((idx2 >= 0) and (idx2 < 26)):
            counts[idx2] = (counts[idx2] - 1)
        j = (j + 1)
    k = 0
    while (k < 26):
        if (counts[k] != 0):
            return False
        k = (k + 1)
    return True

def string_reverse(s):
    n = len(s)
    chars = []
    i = 0
    while (i < n):
        chars.append(s[i])
        i = (i + 1)
    _py_left = 0
    _py_right = (n - 1)
    while (_py_left < _py_right):
        tmp = chars[_py_left]
        chars[_py_left] = chars[_py_right]
        chars[_py_right] = tmp
        _py_left = (_py_left + 1)
        _py_right = (_py_right - 1)
    result = ''
    k = 0
    while (k < n):
        result = (result + chars[k])
        k = (k + 1)
    return result

def test_string_algorithms():
    print('--- String Algorithms ---')
    print(("Is palindrome 'racecar': " + str(is_palindrome('racecar'))))
    print(("Is palindrome 'hello': " + str(is_palindrome('hello'))))
    print(("Is anagram 'listen' 'silent': " + str(is_anagram('listen', 'silent'))))
    print(("Is anagram 'hello' 'world': " + str(is_anagram('hello', 'world'))))
    print(("Reverse 'UniLogic': " + string_reverse('UniLogic')))
    print(("Reverse 'abcdef': " + string_reverse('abcdef')))

def shape_describe(sh):
    a = sh.area()
    p = sh.perimeter()
    a_str = str(round(a, 4))
    p_str = str(round(p, 4))
    return ((((sh.get_name() + ' area=') + a_str) + ' perimeter=') + p_str)

def test_shapes():
    print('--- Shapes ---')
    shapes = []
    c = Circle(5.0)
    r = Rectangle(4.0, 6.0)
    t = Triangle(3.0, 4.0, 5.0)
    shapes.append(c)
    shapes.append(r)
    shapes.append(t)
    i = 0
    while (i < len(shapes)):
        print(shape_describe(shapes[i]))
        i = (i + 1)

def test_animals():
    print('--- Animals ---')
    animals = []
    d = Dog('Rex')
    c = Cat('Whiskers')
    du = Duck('Donald')
    animals.append(d)
    animals.append(c)
    animals.append(du)
    i = 0
    while (i < len(animals)):
        print(animals[i].speak())
        i = (i + 1)

def test_builder():
    print('--- Builder ---')
    pb = PersonBuilder()
    result = pb.set_first('Alice').set_last('Smith').set_age(30).set_email('alice@example.com').build()
    print(result)
    pb2 = PersonBuilder()
    result2 = pb2.set_first('Bob').set_last('Jones').set_age(25).set_email('bob@example.com').build()
    print(result2)

def test_observer():
    print('--- Observer ---')
    pub = EventPublisher()
    obs1 = EventObserver('Logger')
    obs2 = EventObserver('Notifier')
    pub.subscribe(obs1)
    pub.subscribe(obs2)
    pub.publish('start')
    pub.publish('process')
    pub.publish('stop')
    print(('Logger events: ' + obs1.get_events()))
    print(('Notifier events: ' + obs2.get_events()))

def validate_age(age):
    if (age < 0):
        raise ValidationError('age must be non-negative')
    if (age > 150):
        raise ValidationError('age unrealistic value')
    return age

def fetch_data(url):
    if (url == ''):
        raise NetworkError('400 empty URL')
    if (url == 'bad'):
        raise NetworkError('404 not found')
    return ('data from ' + url)

def str_contains_sub(haystack, needle):
    hlen = len(haystack)
    nlen = len(needle)
    if (nlen > hlen):
        return False
    i = 0
    while (i <= (hlen - nlen)):
        _py_match = True
        j = 0
        while (j < nlen):
            if (haystack[(i + j)] != needle[j]):
                _py_match = False
                break
            j = (j + 1)
        if _py_match:
            return True
        i = (i + 1)
    return False

def query_db(sql):
    if str_contains_sub(sql, 'DROP'):
        raise DatabaseError(('DROP in query: ' + sql))
    return ('rows for: ' + sql)

def test_exceptions():
    print('--- Exceptions ---')
    try:
        result = validate_age(25)
        print(('Age 25 valid: ' + str(result)))
    except ValidationError as e:
        print(('Error: ' + e.message))
    try:
        result2 = validate_age((-5))
        print('Should not reach here')
    except ValidationError as e:
        print(('Caught: ' + e.message))
    try:
        result3 = validate_age(200)
    except ValidationError as e:
        print(('Caught: ' + e.message))
    try:
        s = fetch_data('http://example.com')
        print(('Fetched: ' + s))
    except NetworkError as e:
        print(('Network error: ' + e.message))
    try:
        s2 = fetch_data('')
    except NetworkError as e:
        print(('Caught network: ' + e.message))
    try:
        s3 = fetch_data('bad')
    except NetworkError as e:
        print(('Caught network: ' + e.message))
    try:
        r = query_db('SELECT * FROM users')
        print(('DB result: ' + r))
    except DatabaseError as e:
        print(('DB error: ' + e.message))
    try:
        r2 = query_db('DROP TABLE users')
    except DatabaseError as e:
        print(('Caught DB: ' + e.message))
    print('Finally test:')
    try:
        raise AppError('test error')
    except AppError as e:
        print(('Caught app: ' + e.message))
    finally:
        print('Finally executed')
    print('Nested exception handling:')
    try:
        try:
            raise NetworkError('500 server error')
        except ValidationError as e:
            print('Wrong handler')
        except NetworkError as e:
            print(('Inner caught: ' + e.message))
            raise AppError(('re-wrapped: ' + e.message))
    except AppError as e:
        print(('Outer caught: ' + e.message))

def parse_csv_line(line):
    fields = []
    current = ''
    i = 0
    while (i < len(line)):
        c = line[i]
        if (c == ','):
            fields.append(current)
            current = ''
        else:
            current = (current + c)
        i = (i + 1)
    fields.append(current)
    return fields

def csv_to_table(csv_data):
    rows = []
    lines = split_by_char(csv_data, '\n')
    i = 0
    while (i < len(lines)):
        line = lines[i]
        if (line != ''):
            row = parse_csv_line(line)
            rows.append(row)
        i = (i + 1)
    return rows

def test_csv_parser():
    print('--- CSV Parser ---')
    csv_str = 'name,age,city\nAlice,30,London\nBob,25,Paris\nCarol,35,Berlin'
    table = csv_to_table(csv_str)
    i = 0
    while (i < len(table)):
        row = table[i]
        j = 0
        row_str = ''
        while (j < len(row)):
            if (j > 0):
                row_str = (row_str + ' | ')
            row_str = (row_str + row[j])
            j = (j + 1)
        print(row_str)
        i = (i + 1)

def count_words(text):
    _dict0 = {}
    counts = _dict0
    words = split_by_space(text)
    i = 0
    while (i < len(words)):
        word = str_to_lower(words[i])
        if (word in counts):
            counts[word] = (counts[word] + 1)
        else:
            counts[word] = 1
        i = (i + 1)
    return counts

def test_word_frequency():
    print('--- Word Frequency ---')
    text = 'the quick brown fox jumps over the lazy dog the fox'
    counts = count_words(text)
    sorted_words = sorted(counts.keys())
    i = 0
    while (i < len(sorted_words)):
        w = sorted_words[i]
        print(((w + ': ') + str(counts[w])))
        i = (i + 1)

def wrap_text(text, width):
    words = split_by_space(text)
    lines = []
    current_line = ''
    i = 0
    while (i < len(words)):
        word = words[i]
        if (current_line == ''):
            current_line = word
        else:
            if (((len(current_line) + 1) + len(word)) <= width):
                current_line = ((current_line + ' ') + word)
            else:
                lines.append(current_line)
                current_line = word
        i = (i + 1)
    if (current_line != ''):
        lines.append(current_line)
    result = ''
    j = 0
    while (j < len(lines)):
        if (j > 0):
            result = (result + '\n')
        result = (result + lines[j])
        j = (j + 1)
    return result

def test_text_formatter():
    print('--- Text Formatter ---')
    text = 'The quick brown fox jumps over the lazy dog near the river bank'
    wrapped = wrap_text(text, 30)
    print('Wrapped at 30:')
    lines = split_by_char(wrapped, '\n')
    i = 0
    while (i < len(lines)):
        print(lines[i])
        i = (i + 1)

def determinant2x2(a, b, c, d):
    return ((a * d) - (b * c))

def test_matrix():
    print('--- Matrix ---')
    m1 = Matrix(2, 2)
    m1.set_val(0, 0, 1)
    m1.set_val(0, 1, 2)
    m1.set_val(1, 0, 3)
    m1.set_val(1, 1, 4)
    m2 = Matrix(2, 2)
    m2.set_val(0, 0, 5)
    m2.set_val(0, 1, 6)
    m2.set_val(1, 0, 7)
    m2.set_val(1, 1, 8)
    print('M1:')
    print(m1.to_string())
    print('M2:')
    print(m2.to_string())
    m3 = m1.add(m2)
    print('M1 + M2:')
    print(m3.to_string())
    m4 = m1.multiply(m2)
    print('M1 * M2:')
    print(m4.to_string())
    m5 = m1.transpose()
    print('M1 transposed:')
    print(m5.to_string())
    da = m1.mat_get(0, 0)
    db = m1.mat_get(0, 1)
    dc = m1.mat_get(1, 0)
    dd = m1.mat_get(1, 1)
    det = determinant2x2(da, db, dc, dd)
    print(('Det M1: ' + str(det)))

def test_complex():
    print('--- Complex Numbers ---')
    c1 = ComplexNumber(3.0, 4.0)
    c2 = ComplexNumber(1.0, (-2.0))
    print(('c1 = ' + c1.to_string()))
    print(('c2 = ' + c2.to_string()))
    print(('c1 + c2 = ' + c1.add(c2).to_string()))
    print(('c1 * c2 = ' + c1.multiply(c2).to_string()))
    print(('|c1| = ' + str(round(c1.magnitude(), 4))))
    print(('conj(c1) = ' + c1.conjugate().to_string()))

def calc_mean(data):
    total = 0.0
    i = 0
    while (i < len(data)):
        total = (total + data[i])
        i = (i + 1)
    return (total / len(data))

def calc_median(data):
    n = len(data)
    sorted_data = sorted(data)
    mid = (n // 2)
    if ((n % 2) == 0):
        mid_minus_1 = (mid - 1)
        pair_sum = (sorted_data[mid_minus_1] + sorted_data[mid])
        return (pair_sum / 2.0)
    return _py_float(sorted_data[mid])

def calc_mode(data):
    _dict1 = {}
    counts = _dict1
    i = 0
    while (i < len(data)):
        val = data[i]
        if (val in counts):
            counts[val] = (counts[val] + 1)
        else:
            counts[val] = 1
        i = (i + 1)
    max_count = 0
    mode_val = data[0]
    keys = sorted(counts.keys())
    j = 0
    while (j < len(keys)):
        k = keys[j]
        if (counts[k] > max_count):
            max_count = counts[k]
            mode_val = k
        j = (j + 1)
    return mode_val

def calc_variance(data):
    mean = calc_mean(data)
    total = 0.0
    i = 0
    while (i < len(data)):
        _py_diff = (data[i] - mean)
        total = (total + (_py_diff * _py_diff))
        i = (i + 1)
    return (total / len(data))

def calc_std_dev(data):
    return sqrt_approx(calc_variance(data))

def test_statistics():
    print('--- Statistics ---')
    data = [4, 7, 13, 2, 8, 4, 5, 4, 12, 9]
    print(('Data: ' + arr_to_string(data, len(data))))
    print(('Mean: ' + str(round(calc_mean(data), 4))))
    print(('Median: ' + str(round(calc_median(data), 4))))
    print(('Mode: ' + str(calc_mode(data))))
    print(('Variance: ' + str(round(calc_variance(data), 4))))
    print(('Std Dev: ' + str(round(calc_std_dev(data), 4))))

def test_state_machine():
    print('--- State Machine ---')
    light = TrafficLight()
    i = 0
    while (i < 9):
        print(('State: ' + light.get_state()))
        light.next_state()
        i = (i + 1)
    print(('Cycles: ' + str(light.cycles)))

def fibonacci(n):
    if (n <= 1):
        return n
    return (fibonacci((n - 1)) + fibonacci((n - 2)))

def factorial(n):
    if (n <= 1):
        return 1
    return (n * factorial((n - 1)))

def power(base, exp):
    if (exp == 0):
        return 1
    if ((exp % 2) == 0):
        half = power(base, (exp // 2))
        return (half * half)
    return (base * power(base, (exp - 1)))

def gcd(a, b):
    if (b == 0):
        return a
    return gcd(b, (a % b))

def sum_digits(n):
    if (n < 10):
        return n
    return ((n % 10) + sum_digits((n // 10)))

def test_recursion():
    print('--- Recursion ---')
    fib_str = ''
    i = 0
    while (i < 10):
        if (i > 0):
            fib_str = (fib_str + ' ')
        fib_str = (fib_str + str(fibonacci(i)))
        i = (i + 1)
    print(('Fibonacci: ' + fib_str))
    print(('10! = ' + str(factorial(10))))
    print(('2^10 = ' + str(power(2, 10))))
    print(('GCD(48,18) = ' + str(gcd(48, 18))))
    print(('Sum digits 12345 = ' + str(sum_digits(12345))))

def count_char(s, c):
    count = 0
    i = 0
    while (i < len(s)):
        if (s[i] == c):
            count = (count + 1)
        i = (i + 1)
    return count

def replace_char(s, old, new_char):
    result = ''
    i = 0
    while (i < len(s)):
        if (s[i] == old):
            result = (result + new_char)
        else:
            result = (result + s[i])
        i = (i + 1)
    return result

def starts_with_str(s, prefix):
    if (len(prefix) > len(s)):
        return False
    i = 0
    while (i < len(prefix)):
        if (s[i] != prefix[i]):
            return False
        i = (i + 1)
    return True

def ends_with_str(s, suffix):
    if (len(suffix) > len(s)):
        return False
    offset = (len(s) - len(suffix))
    i = 0
    while (i < len(suffix)):
        if (s[(offset + i)] != suffix[i]):
            return False
        i = (i + 1)
    return True

def trim_string(s):
    start = 0
    while ((start < len(s)) and (s[start] == ' ')):
        start = (start + 1)
    _py_end = (len(s) - 1)
    while ((_py_end >= start) and (s[_py_end] == ' ')):
        _py_end = (_py_end - 1)
    result = ''
    i = start
    while (i <= _py_end):
        result = (result + s[i])
        i = (i + 1)
    return result

def to_upper(s):
    result = ''
    i = 0
    while (i < len(s)):
        c = s[i]
        code = ord(c[0])
        if ((code >= 97) and (code <= 122)):
            result = (result + chr((code - 32)))
        else:
            result = (result + c)
        i = (i + 1)
    return result

def to_lower(s):
    result = ''
    i = 0
    while (i < len(s)):
        c = s[i]
        code = ord(c[0])
        if ((code >= 65) and (code <= 90)):
            result = (result + chr((code + 32)))
        else:
            result = (result + c)
        i = (i + 1)
    return result

def test_string_manipulation():
    print('--- String Manipulation ---')
    s = '  Hello World  '
    print((("Trimmed: '" + trim_string(s)) + "'"))
    print(('Upper: ' + to_upper('hello world')))
    print(('Lower: ' + to_lower('HELLO WORLD')))
    print(("Count 'l' in 'hello world': " + str(count_char('hello world', 'l'))))
    print(("Replace 'o' with '0' in 'hello world': " + replace_char('hello world', 'o', '0')))
    print(("Starts with 'hel': " + str(starts_with_str('hello', 'hel'))))
    print(("Starts with 'abc': " + str(starts_with_str('hello', 'abc'))))
    print(("Ends with 'llo': " + str(ends_with_str('hello', 'llo'))))
    print(("Ends with 'abc': " + str(ends_with_str('hello', 'abc'))))

def sort_students_by_score(students, n):
    i = 0
    while (i < (n - 1)):
        j = 0
        while (j < ((n - 1) - i)):
            s1 = students[j]
            s2 = students[(j + 1)]
            if (s1.score < s2.score):
                students[j] = s2
                students[(j + 1)] = s1
            j = (j + 1)
        i = (i + 1)

def average_score(students, n):
    total = 0.0
    i = 0
    while (i < n):
        total = (total + students[i].score)
        i = (i + 1)
    return (total / n)

def test_data_processing():
    print('--- Data Processing ---')
    students = []
    students.append(Student('Alice', 10, 92.5))
    students.append(Student('Bob', 11, 78.0))
    students.append(Student('Carol', 10, 88.5))
    students.append(Student('Dave', 12, 95.0))
    students.append(Student('Eve', 11, 82.0))
    n = len(students)
    print('Before sort:')
    i = 0
    while (i < n):
        print(('  ' + students[i].to_string()))
        i = (i + 1)
    sort_students_by_score(students, n)
    print('After sort by score desc:')
    j = 0
    while (j < n):
        print(('  ' + students[j].to_string()))
        j = (j + 1)
    avg = average_score(students, n)
    print(('Average score: ' + str(round(avg, 2))))

def is_prime(n):
    if (n < 2):
        return False
    if (n == 2):
        return True
    if ((n % 2) == 0):
        return False
    i = 3
    while ((i * i) <= n):
        if ((n % i) == 0):
            return False
        i = (i + 2)
    return True

def sieve_of_eratosthenes(limit):
    sieve_size = (limit + 1)
    is_p = ([True] * sieve_size)
    is_p[0] = False
    if (limit >= 1):
        is_p[1] = False
    i = 2
    while ((i * i) <= limit):
        if is_p[i]:
            j = (i * i)
            while (j <= limit):
                is_p[j] = False
                j = (j + i)
        i = (i + 1)
    primes = []
    k = 2
    while (k <= limit):
        if is_p[k]:
            primes.append(k)
        k = (k + 1)
    return primes

def prime_factors(n):
    factors = []
    d = 2
    while ((d * d) <= n):
        while ((n % d) == 0):
            factors.append(d)
            n = (n // d)
        d = (d + 1)
    if (n > 1):
        factors.append(n)
    return factors

def test_number_theory():
    print('--- Number Theory ---')
    primes = sieve_of_eratosthenes(50)
    primes_str = ''
    i = 0
    while (i < len(primes)):
        if (i > 0):
            primes_str = (primes_str + ' ')
        primes_str = (primes_str + str(primes[i]))
        i = (i + 1)
    print(('Primes <= 50: ' + primes_str))
    print(('Is prime 97: ' + str(is_prime(97))))
    print(('Is prime 100: ' + str(is_prime(100))))
    factors = prime_factors(360)
    factors_str = ''
    j = 0
    while (j < len(factors)):
        if (j > 0):
            factors_str = (factors_str + ' ')
        factors_str = (factors_str + str(factors[j]))
        j = (j + 1)
    print(('Prime factors of 360: ' + factors_str))

def evaluate_rpn(tokens):
    stk = []
    i = 0
    while (i < len(tokens)):
        tok = tokens[i]
        if (tok == '+'):
            b = stk.pop()
            a = stk.pop()
            stk.append((a + b))
        else:
            if (tok == '-'):
                b2 = stk.pop()
                a2 = stk.pop()
                stk.append((a2 - b2))
            else:
                if (tok == '*'):
                    b3 = stk.pop()
                    a3 = stk.pop()
                    stk.append((a3 * b3))
                else:
                    if (tok == '/'):
                        b4 = stk.pop()
                        a4 = stk.pop()
                        if (b4 == 0.0):
                            stk.append(0.0)
                        else:
                            stk.append((a4 / b4))
                    else:
                        stk.append(_py_float(tok))
        i = (i + 1)
    if (len(stk) == 0):
        return 0.0
    return stk[0]

def test_rpn_evaluator():
    print('--- RPN Evaluator ---')
    expr1 = ['3', '4', '+', '2', '*']
    print(('(3+4)*2 = ' + str(evaluate_rpn(expr1))))
    expr2 = ['15', '7', '1', '1', '+', '-', '/', '3', '*', '2', '1', '1', '+', '+', '-']
    print(('Complex RPN = ' + str(evaluate_rpn(expr2))))
    expr3 = ['2', '3', '4', '*', '+']
    print(('2+3*4 = ' + str(evaluate_rpn(expr3))))

def main():
    print('=== GIANT PYTHON TEST ===')
    print('')
    print('== SECTION 1: DATA STRUCTURES ==')
    test_linked_list()
    test_bst()
    test_stack_queue()
    test_priority_queue()
    print('')
    print('== SECTION 2: ALGORITHMS ==')
    test_sorting_searching()
    test_graph_algorithms()
    test_string_algorithms()
    print('')
    print('== SECTION 3: OOP PATTERNS ==')
    test_shapes()
    test_animals()
    test_builder()
    test_observer()
    print('')
    print('== SECTION 4: EXCEPTION HANDLING ==')
    test_exceptions()
    print('')
    print('== SECTION 5: STRING PROCESSING ==')
    test_csv_parser()
    test_word_frequency()
    test_text_formatter()
    print('')
    print('== SECTION 6: MATH ==')
    test_matrix()
    test_complex()
    test_statistics()
    print('')
    print('== SECTION 7: STATE MACHINE ==')
    test_state_machine()
    print('')
    print('== SECTION 8: RECURSION ==')
    test_recursion()
    print('')
    print('== SECTION 9: STRING MANIPULATION ==')
    test_string_manipulation()
    print('')
    print('== SECTION 10: DATA PROCESSING ==')
    test_data_processing()
    print('')
    print('== SECTION 11: NUMBER THEORY ==')
    test_number_theory()
    print('')
    print('== SECTION 12: RPN EVALUATOR ==')
    test_rpn_evaluator()
    print('')
    print('=== ALL TESTS COMPLETE ===')
    return 0


if __name__ == "__main__":
    main()
