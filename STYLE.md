# yterm style guide

## Function calls
This is how a function call must be formatted:
```
myfunc(arg1, arg2);
```
- no space between the name and the (
- space after commas

## Include structure
#### Order of includes in components.h
1. standard libraries
2. 3rd party libraries

These includes should only be in components.h

Includes from the yterm source code can be in any other file

Imports must be ordered based on the length of the text

## Pointer definitions
```
int* b;
```
The pointer must be on the left side

## Statements
```
if (...) {
	
}
else {
	
}
```
- } must be on a line on its own
- { must be on the line with the statement
- the else should be on a seperate line from the previous }

## Naming
- camelCase for variables
- PascalCase for functions
- PascalCase for structs/enums/typedefs etc
- camelCase for file names

## Function definitions
```
void myfunc() {
	
}
```

## Comments
- use `//` for single linecomments

## Line length
- Limited to 80 characters
- If lines are too long with paranthesis, split like this:
```
... (
	...
)
...
```
