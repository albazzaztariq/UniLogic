// =============================================================================
// repl-transpile-data.js
// Pre-computed transpilation examples for UniLogic web REPLs
// Contains Python, C, and UL equivalents of 4 sample programs
// =============================================================================

const TRANSPILE_DATA = {

  // =========================================================================
  // Level 1 — Temperature Converter
  // =========================================================================
  level1: {
    title: "Temperature Converter",

    ul: `-- Temperature Converter
-- Convert Fahrenheit to Celsius and classify the result
-- Concepts: variables, arrays, loops, conditionals, math

function classify(float c) returns string
  if c < 0.0
    return "freezing"
  end if
  if c < 15.0
    return "cold"
  end if
  if c < 25.0
    return "mild"
  end if
  return "hot"
end function

function main() returns int
  -- Fahrenheit values to convert
  array float[5] temps_f = [98.6, 32.0, 212.0, 0.0, 72.0]
  int count = 5
  int hot_count = 0

  -- Convert each temperature and classify
  for int i = 0 while i < count do i++
    float f = temps_f[i]
    -- Formula: C = (F - 32) * 5 / 9
    float c = (f - 32.0) * 5.0 / 9.0
    string label = classify(c)

    print cast(f, string, "%.1f") + "F = " + cast(c, string, "%.1f") + "C (" + label + ")"

    if label == "hot"
      hot_count++
    end if
  end for

  -- Summary
  print cast(hot_count, string) + " of " + cast(count, string) + " temperatures are hot"

  return 0
end function`,

    python: `# Temperature Converter
# Convert Fahrenheit to Celsius and classify the result
# Concepts: variables, lists, loops, conditionals, math

def classify(c):
    """Classify a Celsius temperature"""
    if c < 0.0:
        return "freezing"
    if c < 15.0:
        return "cold"
    if c < 25.0:
        return "mild"
    return "hot"

def main():
    # Fahrenheit values to convert
    temps_f = [98.6, 32.0, 212.0, 0.0, 72.0]
    count = len(temps_f)
    hot_count = 0

    # Convert each temperature and classify
    for f in temps_f:
        # Formula: C = (F - 32) * 5 / 9
        c = (f - 32.0) * 5.0 / 9.0
        label = classify(c)

        print("%.1fF = %.1fC (%s)" % (f, c, label))

        if label == "hot":
            hot_count += 1

    # Summary
    print("%d of %d temperatures are hot" % (hot_count, count))

main()`,

    c: `/* Temperature Converter
   Convert Fahrenheit to Celsius and classify the result
   Concepts: variables, arrays, loops, conditionals, math */

#include <stdio.h>
#include <string.h>

/* Classify a Celsius temperature */
const char *classify(double c) {
    if (c < 0.0) return "freezing";
    if (c < 15.0) return "cold";
    if (c < 25.0) return "mild";
    return "hot";
}

int main(void) {
    /* Fahrenheit values to convert */
    double temps_f[] = {98.6, 32.0, 212.0, 0.0, 72.0};
    int count = 5;
    int hot_count = 0;

    /* Convert each temperature and classify */
    for (int i = 0; i < count; i++) {
        double f = temps_f[i];
        /* Formula: C = (F - 32) * 5 / 9 */
        double c = (f - 32.0) * 5.0 / 9.0;
        const char *label = classify(c);

        printf("%.1fF = %.1fC (%s)\\n", f, c, label);

        if (strcmp(label, "hot") == 0) {
            hot_count++;
        }
    }

    /* Summary */
    printf("%d of %d temperatures are hot\\n", hot_count, count);

    return 0;
}`,

    expected_output: `98.6F = 37.0C (hot)
32.0F = 0.0C (cold)
212.0F = 100.0C (hot)
0.0F = -17.8C (freezing)
72.0F = 22.2C (mild)
2 of 5 temperatures are hot`
  },

  // =========================================================================
  // Level 2 — Prime Checker
  // =========================================================================
  level2: {
    title: "Prime Checker",

    ul: `-- Prime Checker
-- Check which numbers from 2-20 are prime, count total
-- Concepts: functions, loops, conditionals, modulo

-- Check if a number is prime
function is_prime(int n) returns int
  if n < 2
    return 0
  end if
  if n == 2
    return 1
  end if
  if n % 2 == 0
    return 0
  end if
  -- Check odd divisors up to sqrt(n)
  int i = 3
  while i * i <= n do
    if n % i == 0
      return 0
    end if
    i += 2
  end do
  return 1
end function

function main() returns int
  int prime_count = 0
  int total = 0

  -- Check each number from 2 to 20
  for int n = 2 while n <= 20 do n++
    total++
    if is_prime(n) == 1
      print cast(n, string) + " is prime"
      prime_count++
    else
      print cast(n, string) + " is not prime"
    end if
  end for

  -- Summary
  print "found " + cast(prime_count, string) + " primes in " + cast(total, string) + " numbers"

  return 0
end function`,

    python: `# Prime Checker
# Check which numbers from 2-20 are prime, count total
# Concepts: functions, loops, conditionals, modulo

def is_prime(n):
    """Check if a number is prime"""
    if n < 2:
        return 0
    if n == 2:
        return 1
    if n % 2 == 0:
        return 0
    # Check odd divisors up to sqrt(n)
    i = 3
    while i * i <= n:
        if n % i == 0:
            return 0
        i += 2
    return 1

def main():
    prime_count = 0
    total = 0

    # Check each number from 2 to 20
    for n in range(2, 21):
        total += 1
        if is_prime(n) == 1:
            print("%d is prime" % n)
            prime_count += 1
        else:
            print("%d is not prime" % n)

    # Summary
    print("found %d primes in %d numbers" % (prime_count, total))

main()`,

    c: `/* Prime Checker
   Check which numbers from 2-20 are prime, count total
   Concepts: functions, loops, conditionals, modulo */

#include <stdio.h>

/* Check if a number is prime */
int is_prime(int n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    /* Check odd divisors up to sqrt(n) */
    int i = 3;
    while (i * i <= n) {
        if (n % i == 0) return 0;
        i += 2;
    }
    return 1;
}

int main(void) {
    int prime_count = 0;
    int total = 0;

    /* Check each number from 2 to 20 */
    for (int n = 2; n <= 20; n++) {
        total++;
        if (is_prime(n) == 1) {
            printf("%d is prime\\n", n);
            prime_count++;
        } else {
            printf("%d is not prime\\n", n);
        }
    }

    /* Summary */
    printf("found %d primes in %d numbers\\n", prime_count, total);

    return 0;
}`,

    expected_output: `2 is prime
3 is prime
4 is not prime
5 is prime
6 is not prime
7 is prime
8 is not prime
9 is not prime
10 is not prime
11 is prime
12 is not prime
13 is prime
14 is not prime
15 is not prime
16 is not prime
17 is prime
18 is not prime
19 is prime
20 is not prime
found 8 primes in 19 numbers`
  },

  // =========================================================================
  // Level 3 — Word Counter
  // =========================================================================
  level3: {
    title: "Word Counter",

    ul: `-- Word Counter
-- Count word occurrences, print sorted by count descending
-- Concepts: arrays, string parsing, sorting

function main() returns int
  string text = "the cat sat on the mat the cat"

  -- Parse words and count occurrences
  array string[20] words
  array int[20] counts
  int unique = 0
  string current = ""
  int len = size(text)

  for int i = 0 while i <= len do i++
    string ch = ""
    if i < len
      ch = text[i]
    end if
    if ch == " " or i == len
      if current not ""
        -- Check if word already seen
        int found = 0
        for int j = 0 while j < unique do j++
          if words[j] == current
            counts[j]++
            found = 1
            escape
          end if
        end for
        if found == 0
          words[unique] = current
          counts[unique] = 1
          unique++
        end if
        current = ""
      end if
    else
      current = current + ch
    end if
  end for

  -- Sort by count descending (bubble sort)
  for int i = 0 while i < unique - 1 do i++
    for int j = 0 while j < unique - 1 - i do j++
      if counts[j] < counts[j + 1]
        int tc = counts[j]
        counts[j] = counts[j + 1]
        counts[j + 1] = tc
        string tw = words[j]
        words[j] = words[j + 1]
        words[j + 1] = tw
      end if
    end for
  end for

  -- Print results
  for int i = 0 while i < unique do i++
    print words[i] + ": " + cast(counts[i], string)
  end for

  return 0
end function`,

    python: `# Word Counter
# Count word occurrences, print sorted by count descending
# Concepts: dictionaries, string operations, sorting

def main():
    text = "the cat sat on the mat the cat"
    tokens = text.split(" ")

    # Count occurrences using a dictionary
    word_counts = {}
    for word in tokens:
        if word in word_counts:
            word_counts[word] += 1
        else:
            word_counts[word] = 1

    # Extract parallel lists for sorting
    words = list(word_counts.keys())
    counts = list(word_counts.values())

    # Bubble sort by count descending
    n = len(words)
    for i in range(n - 1):
        for j in range(n - 1 - i):
            if counts[j] < counts[j + 1]:
                counts[j], counts[j + 1] = counts[j + 1], counts[j]
                words[j], words[j + 1] = words[j + 1], words[j]

    # Print results
    for i in range(n):
        print("%s: %d" % (words[i], counts[i]))

main()`,

    c: `/* Word Counter
   Count word occurrences, print sorted by count descending
   Concepts: arrays, string operations, sorting */

#include <stdio.h>
#include <string.h>

int main(void) {
    char text[] = "the cat sat on the mat the cat";
    char words[20][32];
    int counts[20];
    int unique = 0;

    /* Tokenize and count occurrences */
    char buffer[256];
    strcpy(buffer, text);
    char *tok = strtok(buffer, " ");
    while (tok != NULL) {
        int found = 0;
        for (int j = 0; j < unique; j++) {
            if (strcmp(words[j], tok) == 0) {
                counts[j]++;
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(words[unique], tok);
            counts[unique] = 1;
            unique++;
        }
        tok = strtok(NULL, " ");
    }

    /* Bubble sort by count descending */
    for (int i = 0; i < unique - 1; i++) {
        for (int j = 0; j < unique - 1 - i; j++) {
            if (counts[j] < counts[j + 1]) {
                int tc = counts[j];
                counts[j] = counts[j + 1];
                counts[j + 1] = tc;
                char tw[32];
                strcpy(tw, words[j]);
                strcpy(words[j], words[j + 1]);
                strcpy(words[j + 1], tw);
            }
        }
    }

    /* Print results */
    for (int i = 0; i < unique; i++) {
        printf("%s: %d\\n", words[i], counts[i]);
    }

    return 0;
}`,

    expected_output: `the: 3
cat: 2
sat: 1
on: 1
mat: 1`
  },

  // =========================================================================
  // Level 4 — Linked List
  // =========================================================================
  level4: {
    title: "Linked List",

    ul: `-- Linked List
-- Singly-linked list: append, print, find, delete
-- Concepts: types, pointers, memtake/memgive

type Node
  int value
  Node <next>
end type

function create_node(int val) returns Node <new_node>
  Node <new_node> = cast(memtake(size(Node)), <Node>)
  <new_node>.value = val
  <new_node>.next = empty
  return <new_node>
end function

function append(Node <head>, int val) returns none
  Node <curr> = <head>
  while <curr>.next not empty do
    <curr> = <curr>.next
  end do
  <curr>.next = create_node(val)
end function

function print_list(Node <head>) returns none
  Node <curr> = <head>
  string out = ""
  while <curr> not empty do
    if out not ""
      out = out + " -> "
    end if
    out = out + cast(<curr>.value, string)
    <curr> = <curr>.next
  end do
  print out
end function

function find(Node <head>, int val) returns int
  Node <curr> = <head>
  while <curr> not empty do
    if <curr>.value == val
      return 1
    end if
    <curr> = <curr>.next
  end do
  return 0
end function

function delete_node(Node <head>, int val) returns Node <result>
  if <head>.value == val
    Node <tmp> = <head>.next
    memgive(<head>)
    return <tmp>
  end if
  Node <curr> = <head>
  while <curr>.next not empty do
    if <curr>.next.value == val
      Node <tmp> = <curr>.next
      <curr>.next = <tmp>.next
      memgive(<tmp>)
      return <head>
    end if
    <curr> = <curr>.next
  end do
  return <head>
end function

function main() returns int
  -- Build list: 10 -> 20 -> 30 -> 40 -> 50
  Node <head> = create_node(10)
  append(<head>, 20)
  append(<head>, 30)
  append(<head>, 40)
  append(<head>, 50)
  print_list(<head>)

  -- Delete 30, then print
  <head> = delete_node(<head>, 30)
  print_list(<head>)

  -- Search for values
  if find(<head>, 20) == 1
    print "found"
  else
    print "not found"
  end if
  if find(<head>, 30) == 1
    print "found"
  else
    print "not found"
  end if

  -- Cleanup
  Node <curr> = <head>
  while <curr> not empty do
    Node <tmp> = <curr>.next
    memgive(<curr>)
    <curr> = <tmp>
  end do
  return 0
end function`,

    python: `# Linked List
# Singly-linked list: append, print, find, delete
# Concepts: classes, references, traversal

class Node:
    def __init__(self, value):
        self.value = value
        self.next = None

def append(head, val):
    curr = head
    while curr.next is not None:
        curr = curr.next
    curr.next = Node(val)

def print_list(head):
    curr = head
    out = ""
    while curr is not None:
        if out != "":
            out += " -> "
        out += str(curr.value)
        curr = curr.next
    print(out)

def find(head, val):
    curr = head
    while curr is not None:
        if curr.value == val:
            return 1
        curr = curr.next
    return 0

def delete_node(head, val):
    if head.value == val:
        return head.next
    curr = head
    while curr.next is not None:
        if curr.next.value == val:
            curr.next = curr.next.next
            return head
        curr = curr.next
    return head

def main():
    # Build list: 10 -> 20 -> 30 -> 40 -> 50
    head = Node(10)
    append(head, 20)
    append(head, 30)
    append(head, 40)
    append(head, 50)
    print_list(head)

    # Delete 30, then print
    head = delete_node(head, 30)
    print_list(head)

    # Search for values
    if find(head, 20) == 1:
        print("found")
    else:
        print("not found")
    if find(head, 30) == 1:
        print("found")
    else:
        print("not found")

main()`,

    c: `/* Linked List
   Singly-linked list: append, print, find, delete
   Concepts: structs, pointers, malloc/free */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node { int value; struct Node *next; } Node;

Node *create_node(int val) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->value = val;
    n->next = NULL;
    return n;
}

void append(Node *head, int val) {
    Node *curr = head;
    while (curr->next != NULL) curr = curr->next;
    curr->next = create_node(val);
}

void print_list(Node *head) {
    Node *curr = head;
    char out[256] = "";
    while (curr != NULL) {
        if (strlen(out) > 0) strcat(out, " -> ");
        char ns[16];
        sprintf(ns, "%d", curr->value);
        strcat(out, ns);
        curr = curr->next;
    }
    printf("%s\\n", out);
}

int find(Node *head, int val) {
    Node *curr = head;
    while (curr != NULL) {
        if (curr->value == val) return 1;
        curr = curr->next;
    }
    return 0;
}

Node *delete_node(Node *head, int val) {
    if (head->value == val) {
        Node *tmp = head->next;
        free(head);
        return tmp;
    }
    Node *curr = head;
    while (curr->next != NULL) {
        if (curr->next->value == val) {
            Node *tmp = curr->next;
            curr->next = tmp->next;
            free(tmp);
            return head;
        }
        curr = curr->next;
    }
    return head;
}

int main(void) {
    /* Build list: 10 -> 20 -> 30 -> 40 -> 50 */
    Node *head = create_node(10);
    append(head, 20);
    append(head, 30);
    append(head, 40);
    append(head, 50);
    print_list(head);

    /* Delete 30, then print */
    head = delete_node(head, 30);
    print_list(head);

    /* Search for values */
    if (find(head, 20) == 1) printf("found\\n");
    else printf("not found\\n");
    if (find(head, 30) == 1) printf("found\\n");
    else printf("not found\\n");

    /* Cleanup */
    Node *curr = head;
    while (curr != NULL) {
        Node *tmp = curr->next;
        free(curr);
        curr = tmp;
    }
    return 0;
}`,

    expected_output: `10 -> 20 -> 30 -> 40 -> 50
10 -> 20 -> 40 -> 50
found
not found`
  }
};

// =============================================================================
// Simulated performance data for each level
// Ratios: Python ~50-100x slower than C, UL interpreted ~5-15x of C,
//         UL native ~1.0-1.2x of C
// =============================================================================

const PERF_DATA = {

  level1: {
    python_ms:      4.8,     // ~60x C
    c_ms:           0.08,
    ul_interp_ms:   0.72,    // ~9x C
    ul_native_ms:   0.09,    // ~1.1x C
    python_ops:    847,
    c_ops:         156,
    ul_ops:        312
  },

  level2: {
    python_ms:      8.4,     // ~70x C
    c_ms:           0.12,
    ul_interp_ms:   1.08,    // ~9x C
    ul_native_ms:   0.13,    // ~1.1x C
    python_ops:   1240,
    c_ops:         218,
    ul_ops:        436
  },

  level3: {
    python_ms:     12.0,     // ~80x C
    c_ms:           0.15,
    ul_interp_ms:   1.65,    // ~11x C
    ul_native_ms:   0.17,    // ~1.1x C
    python_ops:   2065,
    c_ops:         384,
    ul_ops:        710
  },

  level4: {
    python_ms:     17.1,     // ~90x C
    c_ms:           0.19,
    ul_interp_ms:   2.47,    // ~13x C
    ul_native_ms:   0.22,    // ~1.2x C
    python_ops:   2840,
    c_ops:         512,
    ul_ops:        948
  }
};
