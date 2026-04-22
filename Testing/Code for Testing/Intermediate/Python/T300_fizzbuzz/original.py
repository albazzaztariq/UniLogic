"""FizzBuzz with extensions — a classic programming exercise."""


def fizzbuzz(n: int) -> str:
    if n % 15 == 0:
        return "FizzBuzz"
    elif n % 3 == 0:
        return "Fizz"
    elif n % 5 == 0:
        return "Buzz"
    else:
        return str(n)


def count_fizz(limit: int) -> int:
    count: int = 0
    i: int = 1
    while i <= limit:
        if fizzbuzz(i) == "Fizz":
            count = count + 1
        i = i + 1
    return count


def count_buzz(limit: int) -> int:
    count: int = 0
    i: int = 1
    while i <= limit:
        if fizzbuzz(i) == "Buzz":
            count = count + 1
        i = i + 1
    return count


def count_fizzbuzz(limit: int) -> int:
    count: int = 0
    i: int = 1
    while i <= limit:
        if fizzbuzz(i) == "FizzBuzz":
            count = count + 1
        i = i + 1
    return count


def main() -> int:
    # Print first 20
    i: int = 1
    while i <= 20:
        print(fizzbuzz(i))
        i = i + 1

    # Stats
    print("Fizz count: " + str(count_fizz(100)))
    print("Buzz count: " + str(count_buzz(100)))
    print("FizzBuzz count: " + str(count_fizzbuzz(100)))
    return 0


if __name__ == "__main__":
    main()
