# Linked List
# Implements a singly linked list with insert, search, and print
# Concepts: classes (structs), references (pointers), data structures

class Node:
    """A single node in the linked list"""
    def __init__(self, value):
        self.value = value
        self.next = None

class LinkedList:
    """Singly linked list with head pointer"""
    def __init__(self):
        self.head = None
        self.size = 0

    def push_front(self, value):
        """Insert a new node at the front"""
        node = Node(value)
        node.next = self.head
        self.head = node
        self.size += 1

    def find(self, value):
        """Search for a value, return True if found"""
        current = self.head
        while current is not None:
            if current.value == value:
                return True
            current = current.next
        return False

    def print_list(self):
        """Print all elements: [a -> b -> c]"""
        result = "["
        current = self.head
        while current is not None:
            if current != self.head:
                result += " -> "
            result += str(current.value)
            current = current.next
        result += "]"
        print(result)

# Main program
print("=== Linked List ===")
print("")

# Build a list by pushing to front
ll = LinkedList()
values = [10, 20, 30, 40, 50]
for v in values:
    ll.push_front(v)
    print("Pushed %d, size = %d" % (v, ll.size))

print("")
print("List contents:")
ll.print_list()

print("")

# Search for values
search_for = [30, 99, 10]
for v in search_for:
    if ll.find(v):
        print("Found %d in list" % v)
    else:
        print("%d not in list" % v)
