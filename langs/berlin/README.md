```
/*
Multiline comment

*/

```
func fibrec(x) {
  # if (x < 3) {
  #     return 1
  # } else {
  #     return fibrec(x-1)+fibrec(x-2)
  # }
  if (x < 3)
    return 1

  return fibrec(x-1)+fibrec(x-2)
}

func fibiter(x) {
  a = 1
  b = 1
  
  for (i = 3; i < x; i++) {
    c = a + b
    a = b
    b = c
  }
}
```

# call both
fibrec(10)
fibiter(10)

func add_array() ret35 {
  arr = [1,2,3,4]
  for (i = 0; i < 4; i++) {
    ret += arr[i]
  }
  # returns the specified return var ("ret35") if specified
  # syntactically invalid if no return var specified and there's no "return" statement
}
```
