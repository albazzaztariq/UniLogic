# Prime Checker
# Tests numbers for primality and finds primes in a range
# Concepts: functions, loops, conditionals

def is_prime(n):
    """Check if a number is prime"""
    if n < 2:
        return False
    if n == 2:
        return True
    if n % 2 == 0:
        return False
    # Only check odd divisors up to sqrt(n)
    i = 3
    while i * i <= n:
        if n % i == 0:
            return False
        i += 2
    return True

def count_primes(limit):
    """Count how many primes exist below a limit"""
    count = 0
    for num in range(2, limit):
        if is_prime(num):
            count += 1
    return count

# Main program
print("=== Prime Checker ===")
print("")

# Test specific numbers
test_nums = [1, 2, 7, 15, 23, 42, 97]
for n in test_nums:
    if is_prime(n):
        print("%d is prime" % n)
    else:
        print("%d is not prime" % n)

print("")

# Find all primes below 50
print("Primes below 50:")
line = ""
for n in range(2, 50):
    if is_prime(n):
        if line != "":
            line += " "
        line += str(n)
print(line)

print("")
total = count_primes(100)
print("Total primes below 100: %d" % total)
