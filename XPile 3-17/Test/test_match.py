from dataclasses import dataclass, field

def test_int_match():
    x = 2
    match x:
        case 1:
            print('one')
        case 2:
            print('two')
        case 3:
            print('three')
        case _:
            print('other')

def test_default():
    y = 99
    match y:
        case 1:
            print('one')
        case 2:
            print('two')
        case _:
            print('default hit')

def test_match_expr():
    val = (3 + 2)
    match val:
        case 4:
            print('four')
        case 5:
            print('five')
        case 6:
            print('six')

def test_match_negative():
    n = (-1)
    match n:
        case (-1):
            print('negative one')
        case 0:
            print('zero')
        case 1:
            print('positive one')

def main():
    test_int_match()
    test_default()
    test_match_expr()
    test_match_negative()
    return 0


if __name__ == "__main__":
    main()
