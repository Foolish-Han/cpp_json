# Project Documentation

## Overview

This project is a JSON parser implemented in C. It is based on the tutorial from [miloyip/json-tutorial](https://github.com/miloyip/json-tutorial). The primary purpose of this project is to provide a hands-on learning experience for understanding the implementation of a JSON parser in C. By working through this project, you will gain insights into various aspects of C programming, including memory management, data structures, and algorithm design.

## Purpose

The purpose of this project is to learn and understand the implementation of a JSON parser in C. It covers various aspects of C programming, including memory management, data structures, and algorithm design. The project is based on the tutorial from [miloyip/json-tutorial](https://github.com/miloyip/json-tutorial).

## Source

This project is derived from the JSON tutorial by Milo Yip, which can be found at [miloyip/json-tutorial](https://github.com/miloyip/json-tutorial). The tutorial provides a step-by-step guide to implementing a JSON parser, and this project follows the same approach.

## Functions and Usage

### lept_parse

```c
int lept_parse(lept_value *v, const char *json);
```

Parses a JSON string and stores the result in the provided `lept_value` structure.

- `v`: Pointer to the `lept_value` structure where the parsed result will be stored.
- `json`: JSON string to be parsed.

### lept_stringify

```c
char *lept_stringify(const lept_value *v, size_t *length);
```

Stringifies a JSON value and returns the result as a string.

- `v`: Pointer to the `lept_value` structure containing the JSON value to be stringified.
- `length`: Pointer to a variable where the length of the stringified result will be stored.

### lept_copy

```c
void lept_copy(lept_value *dst, const lept_value *src);
```

Copies a JSON value from the source to the destination.

- `dst`: Pointer to the destination `lept_value` structure.
- `src`: Pointer to the source `lept_value` structure.

### lept_move

```c
void lept_move(lept_value *dst, lept_value *src);
```

Moves a JSON value from the source to the destination.

- `dst`: Pointer to the destination `lept_value` structure.
- `src`: Pointer to the source `lept_value` structure.

### lept_swap

```c
void lept_swap(lept_value *lhs, lept_value *rhs);
```

Swaps two JSON values.

- `lhs`: Pointer to the left-hand side `lept_value` structure.
- `rhs`: Pointer to the right-hand side `lept_value` structure.

### lept_free

```c
void lept_free(lept_value *v);
```

Frees a JSON value.

- `v`: Pointer to the `lept_value` structure to be freed.

### lept_get_type

```c
lept_type lept_get_type(const lept_value *v);
```

Gets the type of a JSON value.

- `v`: Pointer to the `lept_value` structure.

### lept_is_equal

```c
int lept_is_equal(const lept_value *lhs, const lept_value *rhs);
```

Checks if two JSON values are equal.

- `lhs`: Pointer to the left-hand side `lept_value` structure.
- `rhs`: Pointer to the right-hand side `lept_value` structure.

### lept_set_null

```c
#define lept_set_null(v) lept_free(v)
```

Sets a JSON value to null.

- `v`: Pointer to the `lept_value` structure.

### lept_get_boolean

```c
int lept_get_boolean(const lept_value *v);
```

Gets the boolean value of a JSON value.

- `v`: Pointer to the `lept_value` structure.

### lept_set_boolean

```c
void lept_set_boolean(lept_value *v, int b);
```

Sets the boolean value of a JSON value.

- `v`: Pointer to the `lept_value` structure.
- `b`: Boolean value (0 or 1).

### lept_get_number

```c
double lept_get_number(const lept_value *v);
```

Gets the number value of a JSON value.

- `v`: Pointer to the `lept_value` structure.

### lept_set_number

```c
void lept_set_number(lept_value *v, double n);
```

Sets the number value of a JSON value.

- `v`: Pointer to the `lept_value` structure.
- `n`: Number value.

### lept_get_string

```c
const char *lept_get_string(const lept_value *v);
```

Gets the string value of a JSON value.

- `v`: Pointer to the `lept_value` structure.

### lept_get_string_length

```c
size_t lept_get_string_length(const lept_value *v);
```

Gets the length of the string value of a JSON value.

- `v`: Pointer to the `lept_value` structure.

### lept_set_string

```c
void lept_set_string(lept_value *v, const char *s, size_t len);
```

Sets the string value of a JSON value.

- `v`: Pointer to the `lept_value` structure.
- `s`: String value.
- `len`: Length of the string value.

### lept_set_array

```c
void lept_set_array(lept_value *v, size_t capacity);
```

Sets the array value of a JSON value.

- `v`: Pointer to the `lept_value` structure.
- `capacity`: Capacity of the array.

### lept_get_array_size

```c
size_t lept_get_array_size(const lept_value *v);
```

Gets the size of the array value of a JSON value.

- `v`: Pointer to the `lept_value` structure.

### lept_get_array_capacity

```c
size_t lept_get_array_capacity(const lept_value *v);
```

Gets the capacity of the array value of a JSON value.

- `v`: Pointer to the `lept_value` structure.

### lept_reserve_array

```c
void lept_reserve_array(lept_value *v, size_t capacity);
```

Reserves the capacity of the array value of a JSON value.

- `v`: Pointer to the `lept_value` structure.
- `capacity`: Capacity to be reserved.

### lept_shrink_array

```c
void lept_shrink_array(lept_value *v);
```

Shrinks the capacity of the array value of a JSON value.

- `v`: Pointer to the `lept_value` structure.

### lept_clear_array

```c
void lept_clear_array(lept_value *v);
```

Clears the array value of a JSON value.

- `v`: Pointer to the `lept_value` structure.

### lept_get_array_element

```c
lept_value *lept_get_array_element(const lept_value *v, size_t index);
```

Gets an element of the array value of a JSON value.

- `v`: Pointer to the `lept_value` structure.
- `index`: Index of the element.

### lept_pushback_array_element

```c
lept_value *lept_pushback_array_element(lept_value *v);
```

Pushes back an element to the array value of a JSON value.

- `v`: Pointer to the `lept_value` structure.

### lept_popback_array_element

```c
void lept_popback_array_element(lept_value *v);
```

Pops back an element from the array value of a JSON value.

- `v`: Pointer to the `lept_value` structure.

### lept_insert_array_element

```c
lept_value *lept_insert_array_element(lept_value *v, size_t index);
```

Inserts an element to the array value of a JSON value.

- `v`: Pointer to the `lept_value` structure.
- `index`: Index of the element.

### lept_erase_array_element

```c
void lept_erase_array_element(lept_value *v, size_t index, size_t count);
```

Erases elements from the array value of a JSON value.

- `v`: Pointer to the `lept_value` structure.
- `index`: Index of the first element to be erased.
- `count`: Number of elements to be erased.

### lept_set_object

```c
void lept_set_object(lept_value *v, size_t capacity);
```

Sets the object value of a JSON value.

- `v`: Pointer to the `lept_value` structure.
- `capacity`: Capacity of the object.

### lept_get_object_size

```c
size_t lept_get_object_size(const lept_value *v);
```

Gets the size of the object value of a JSON value.

- `v`: Pointer to the `lept_value` structure.

### lept_get_object_capacity

```c
size_t lept_get_object_capacity(const lept_value *v);
```

Gets the capacity of the object value of a JSON value.

- `v`: Pointer to the `lept_value` structure.

### lept_reserve_object

```c
void lept_reserve_object(lept_value *v, size_t capacity);
```

Reserves the capacity of the object value of a JSON value.

- `v`: Pointer to the `lept_value` structure.
- `capacity`: Capacity to be reserved.

### lept_shrink_object

```c
void lept_shrink_object(lept_value *v);
```

Shrinks the capacity of the object value of a JSON value.

- `v`: Pointer to the `lept_value` structure.

### lept_clear_object

```c
void lept_clear_object(lept_value *v);
```

Clears the object value of a JSON value.

- `v`: Pointer to the `lept_value` structure.

### lept_get_object_key

```c
const char *lept_get_object_key(const lept_value *v, size_t index);
```

Gets the key of an object member.

- `v`: Pointer to the `lept_value` structure.
- `index`: Index of the member.

### lept_get_object_key_length

```c
size_t lept_get_object_key_length(const lept_value *v, size_t index);
```

Gets the length of the key of an object member.

- `v`: Pointer to the `lept_value` structure.
- `index`: Index of the member.

### lept_get_object_value

```c
lept_value *lept_get_object_value(const lept_value *v, size_t index);
```

Gets the value of an object member.

- `v`: Pointer to the `lept_value` structure.
- `index`: Index of the member.

### lept_find_object_index

```c
size_t lept_find_object_index(const lept_value *v, const char *key, size_t klen);
```

Finds the index of an object member by key.

- `v`: Pointer to the `lept_value` structure.
- `key`: Key of the member.
- `klen`: Length of the key.

### lept_find_object_value

```c
lept_value *lept_find_object_value(lept_value *v, const char *key, size_t klen);
```

Finds the value of an object member by key.

- `v`: Pointer to the `lept_value` structure.
- `key`: Key of the member.
- `klen`: Length of the key.

### lept_set_object_value

```c
lept_value *lept_set_object_value(lept_value *v, const char *key, size_t klen);
```

Sets the value of an object member by key.

- `v`: Pointer to the `lept_value` structure.
- `key`: Key of the member.
- `klen`: Length of the key.

### lept_remove_object_value

```c
void lept_remove_object_value(lept_value *v, size_t index);
```

Removes an object member by index.

- `v`: Pointer to the `lept_value` structure.
- `index`: Index of the member.
