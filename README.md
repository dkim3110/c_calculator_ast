# CLI Calculator

## DESCRIPTION:

A simple command-line utility to calculate basic expressions.

## INSTALLATION:

```bash
# Build
git clone https://github.com/dkim3110/c_calculator_ast.git
cd c_calculator_ast
make

# Deploy
sudo make install
# To uninstall, run 'sudo make uninstall'
```

## USAGE:

- Inputs must be wrapped in quotations (ex. `"(1 + 2) / 3"`)
  - Simple expressions may be written plainly without spaces (ex. `"1+2"`)
- Only accepts parentheses; do not input brackets or curly braces

### OPERATORS:

- `+`
- `-`
- `*`
- `/`
- `%`
- `!`

### FUNCTIONS:

- `sqrt()`
- `sin()`
  - `arcsin`
- `cos()`
  - `arccos`
- `tan()`
  - `arctan`
- `abs()`
- `ln()`
- `log()`

### CONSTANTS:

- `e`
- `pi`
- `tau`
- `phi`
- `inf`
