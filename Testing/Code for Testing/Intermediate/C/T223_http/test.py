from dataclasses import dataclass, field
def str_char_at(s, i): return s[i] if 0 <= i < len(s) else ''
def strlen(s): return len(s)

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def parse_csv_line(line):
    fields = 1
    len = strlen(line)
    i = 0
    while (i < len):
        ch = str_char_at(line, i)
        if (ch == ','):
            fields = (fields + 1)
        i = (i + 1)
    return fields

def main():
    response_status = '200'
    print(('status: ' + response_status))
    header = 'name,age,city,score'
    row1 = 'Alice,30,London,95'
    row2 = 'Bob,25,Paris,88'
    row3 = 'Charlie,35,Tokyo,92'
    header_fields = parse_csv_line(header)
    print(('fields: ' + str(header_fields)))
    print(('row1 fields: ' + str(parse_csv_line(row1))))
    print(('row2 fields: ' + str(parse_csv_line(row2))))
    print(('row3 fields: ' + str(parse_csv_line(row3))))
    total_rows = 3
    print(('total rows: ' + str(total_rows)))
    valid = True
    if (parse_csv_line(row1) != header_fields):
        valid = False
    if (parse_csv_line(row2) != header_fields):
        valid = False
    if (parse_csv_line(row3) != header_fields):
        valid = False
    if valid:
        print('data valid')
    else:
        print('data invalid')
    return 0


if __name__ == "__main__":
    main()
