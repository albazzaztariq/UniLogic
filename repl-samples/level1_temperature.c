/* Temperature Converter
   Converts Celsius to Fahrenheit and Kelvin
   Concepts: variables, print, basic math */

#include <stdio.h>

int main(void) {
    /* Define temperatures to convert */
    double temps_c[] = {0.0, 20.0, 37.0, 100.0};
    int count = 4;

    printf("=== Temperature Converter ===\n");
    printf("\n");

    /* Convert each temperature */
    for (int i = 0; i < count; i++) {
        double c = temps_c[i];
        /* Celsius to Fahrenheit: F = C * 9/5 + 32 */
        double f = c * 9.0 / 5.0 + 32.0;
        /* Celsius to Kelvin: K = C + 273.15 */
        double k = c + 273.15;

        printf("%.1f C = %.1f F = %.2f K\n", c, f, k);
    }

    printf("\n");
    printf("Freezing point of water: 0.0 C\n");
    printf("Boiling point of water:  100.0 C\n");
    printf("Normal body temperature: 37.0 C\n");

    return 0;
}
