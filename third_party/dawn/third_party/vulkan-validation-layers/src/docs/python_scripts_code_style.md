# Python Scripts Coding Style

In effort to keep the python scripts consistent (as most developers are C++ devs, not python experts), please follow the following:

# Functions

All helper functions should have the types strongly declared

```python
def someFunction(varA: bool, varB: str = "default") -> str:
```

# Strings

## Format strings with f'string (unless have reason not to)

There are many ways to format a string in python, by default use  [f'string](https://note.nkmk.me/en/python-f-strings/)

```python
function = f'void {name}(uint32_t {param}) {{ return true; }}'
```

### Exceptions

f'string has some known limitations, in those cases it is ok to use `.format` or template strings

## Use join() then write to file

```python
out = []
out.append(getSomeString())
out.append(f'something {variable}')

self.write("".join(out))
```

These are the [fastet](https://www.tutorialspoint.com/What-is-the-most-efficient-string-concatenation-method-in-python#)

## Each line is in charge of adding the new line at the end

It is easy to start putting random `\n` everywhere.

Each line to the string array is in charge of adding the `\n` at the end, so each line can assume it will.

This is important when calling into utils that return strings