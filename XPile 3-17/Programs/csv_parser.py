from dataclasses import dataclass, field
def str_char_at(s, i): return s[i] if 0 <= i < len(s) else ''
def str_index_of(s, sub): return s.find(sub)
def str_substr(s, start, length): return s[start:start+length]
def strlen(s): return len(s)

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def parse_row(row, out):
    field_count = 0
    pos = 0
    row_len = strlen(row)
    current = ''
    in_quotes = False
    while (pos < row_len):
        ch = str_char_at(row, pos)
        if in_quotes:
            if (ch == '"'):
                in_quotes = False
            else:
                current = (current + ch)
        else:
            if (ch == '"'):
                in_quotes = True
            else:
                if (ch == ','):
                    out[field_count] = current
                    field_count = (field_count + 1)
                    current = ''
                else:
                    if (ch != '\n'):
                        current = (current + ch)
        pos = (pos + 1)
    out[field_count] = current
    field_count = (field_count + 1)
    return field_count

def split_rows(csv, out):
    row_count = 0
    pos = 0
    csv_len = strlen(csv)
    current = ''
    while (pos < csv_len):
        ch = str_char_at(csv, pos)
        if (ch == '\n'):
            if (strlen(current) > 0):
                out[row_count] = current
                row_count = (row_count + 1)
            current = ''
        else:
            current = (current + ch)
        pos = (pos + 1)
    if (strlen(current) > 0):
        out[row_count] = current
        row_count = (row_count + 1)
    return row_count

def get_cell(rows, row, col):
    fields = ['', '', '', '', '', '', '', '']
    parse_row(rows[row], fields)
    return fields[col]

def get_column_index(header_row, col_name):
    headers = ['', '', '', '', '', '', '', '']
    count = parse_row(header_row, headers)
    for i in range(count):
        if (headers[i] == col_name):
            return i
    return (-1)

def main():
    csv = 'Name,Age,City,Score\nAlice,30,New York,95\nBob,25,"Los Angeles, CA",87\nCharlie,35,Chicago,92\nDiana,28,Houston,88\nEve,32,"Portland, OR",91\n'
    rows = ['', '', '', '', '', '', '', '', '', '']
    num_rows = split_rows(csv, rows)
    print('Total rows (including header):')
    print(num_rows)
    print('')
    print('=== CSV Data ===')
    headers = ['', '', '', '', '', '', '', '']
    parse_row(rows[0], headers)
    print(headers[0])
    print(headers[1])
    print(headers[2])
    print(headers[3])
    print('')
    for i in range(1, num_rows):
        fields = ['', '', '', '', '', '', '', '']
        parse_row(rows[i], fields)
        print(fields[0])
        print(fields[1])
        print(fields[2])
        print(fields[3])
        print('---')
    print('')
    print('=== Cell Lookups ===')
    cell1 = get_cell(rows, 1, 0)
    print('Row 1, Col 0 (Name):')
    print(cell1)
    cell2 = get_cell(rows, 2, 2)
    print('Row 2, Col 2 (City):')
    print(cell2)
    cell3 = get_cell(rows, 4, 3)
    print('Row 4, Col 3 (Score):')
    print(cell3)
    print('')
    print('=== Column by Name ===')
    city_col = get_column_index(rows[0], 'City')
    print('City column index:')
    print(city_col)
    print('All cities:')
    for i in range(1, num_rows):
        city = get_cell(rows, i, city_col)
        print(city)
    return 0


if __name__ == "__main__":
    main()
