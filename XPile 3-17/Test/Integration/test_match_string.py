from dataclasses import dataclass, field


def greet(lang):
    match lang:
        case 'en':
            print('hello')
        case 'es':
            print('hola')
        case 'fr':
            print('bonjour')
        case 'de':
            print('hallo')
        case _:
            print('unknown')

def main():
    greet('en')
    greet('es')
    greet('fr')
    greet('de')
    greet('jp')
    code = 'es'
    match code:
        case 'en':
            print('english')
        case 'es':
            print('spanish')
        case _:
            print('other')
    return 0


if __name__ == "__main__":
    main()
