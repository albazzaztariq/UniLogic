# Temperature Converter
# Converts Celsius to Fahrenheit and Kelvin
# Concepts: variables, print, basic math

# Define temperatures to convert
temps_c = [0.0, 20.0, 37.0, 100.0]

print("=== Temperature Converter ===")
print("")

# Convert each temperature
for c in temps_c:
    # Celsius to Fahrenheit: F = C * 9/5 + 32
    f = c * 9.0 / 5.0 + 32.0
    # Celsius to Kelvin: K = C + 273.15
    k = c + 273.15

    print("%.1f C = %.1f F = %.2f K" % (c, f, k))

print("")
print("Freezing point of water: 0.0 C")
print("Boiling point of water:  100.0 C")
print("Normal body temperature: 37.0 C")
