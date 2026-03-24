# Word Counter
# Counts word frequencies in a text string
# Concepts: arrays/dicts, string manipulation, sorting

def count_words(text):
    """Split text into words and count each occurrence"""
    # Convert to lowercase for consistent counting
    text = text.lower()

    # Split on spaces
    words = text.split(" ")

    # Count each word using a dictionary
    counts = {}
    for word in words:
        if word == "":
            continue
        if word in counts:
            counts[word] += 1
        else:
            counts[word] = 1

    return counts

# Main program
print("=== Word Counter ===")
print("")

text = "the cat sat on the mat the cat liked the mat"
print("Text: " + text)
print("")

# Count words
word_counts = count_words(text)

# Display results sorted alphabetically
print("Word frequencies:")
keys = sorted(word_counts.keys())
for word in keys:
    count = word_counts[word]
    bar = "*" * count
    print("  %-8s %d %s" % (word, count, bar))

print("")
print("Unique words: %d" % len(keys))
print("Total words:  %d" % sum(word_counts.values()))
