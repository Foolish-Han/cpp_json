#ifndef LEPTJSON_H__
#define LEPTJSON_H__

#include <stddef.h> /* size_t */

#define LEPT_KEY_NOT_EXIST ((size_t)-1)

/**
 * @brief JSON value types.
 */
typedef enum {
  LEPT_NULL,   /**< Null type */
  LEPT_FALSE,  /**< Boolean false type */
  LEPT_TRUE,   /**< Boolean true type */
  LEPT_NUMBER, /**< Number type */
  LEPT_STRING, /**< String type */
  LEPT_ARRAY,  /**< Array type */
  LEPT_OBJECT  /**< Object type */
} lept_type;

/**
 * @brief JSON value structure.
 */
typedef struct lept_value lept_value;

/**
 * @brief JSON object member structure.
 */
typedef struct lept_member lept_member;

/**
 * @brief JSON value structure.
 */
struct lept_value {
  union {
    double n; /**< Number value */
    struct {
      lept_member *m; /**< Object members */
      size_t size;    /**< Number of members */
      size_t capacity;/**< Capacity of members */
    } o; /**< Object */
    struct {
      lept_value *e; /**< Array elements */
      size_t size;   /**< Number of elements */
      size_t capacity;/**< Capacity of elements */
    } a; /**< Array */
    struct {
      char *s; /**< String value */
      size_t len; /**< Length of the string */
    } s; /**< String */
  } u; /**< Union of value types */
  lept_type type; /**< Type of the value */
};

/**
 * @brief JSON object member structure.
 */
struct lept_member {
  char *k; /**< Member key string */
  size_t klen; /**< Length of the key string */
  lept_value v; /**< Member value */
};

/**
 * @brief JSON parsing result codes.
 */
enum {
  LEPT_PARSE_OK = 0, /**< Parsing successful */
  LEPT_PARSE_EXPECT_VALUE, /**< Expecting a value */
  LEPT_PARSE_INVALID_VALUE, /**< Invalid value */
  LEPT_PARSE_ROOT_NOT_SINGULAR, /**< Root not singular */
  LEPT_PARSE_NUMBER_TOO_BIG, /**< Number too big */
  LEPT_PARSE_MISS_QUOTATION_MARK, /**< Missing quotation mark */
  LEPT_PARSE_INVALID_STRING_ESCAPE, /**< Invalid string escape */
  LEPT_PARSE_INVALID_STRING_CHAR, /**< Invalid string character */
  LEPT_PARSE_INVALID_UNICODE_HEX, /**< Invalid Unicode hex */
  LEPT_PARSE_INVALID_UNICODE_SURROGATE, /**< Invalid Unicode surrogate */
  LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, /**< Missing comma or square bracket */
  LEPT_PARSE_MISS_KEY, /**< Missing key */
  LEPT_PARSE_MISS_COLON, /**< Missing colon */
  LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET /**< Missing comma or curly bracket */
};

/**
 * @brief Initializes a JSON value.
 * 
 * @param v JSON value to be initialized
 */
#define lept_init(v)                                                           \
  do {                                                                         \
    (v)->type = LEPT_NULL;                                                     \
  } while (0)

/**
 * @brief Parses a JSON string.
 * 
 * @param v JSON value to be parsed
 * @param json JSON string to be parsed
 * @return int Parsing result
 */
int lept_parse(lept_value *v, const char *json);

/**
 * @brief Stringifies a JSON value.
 * 
 * @param v JSON value to be stringified
 * @param length Pointer to the length of the stringified value
 * @return char* Stringified JSON value
 */
char *lept_stringify(const lept_value *v, size_t *length);

/**
 * @brief Copies a JSON value.
 * 
 * @param dst Destination JSON value
 * @param src Source JSON value
 */
void lept_copy(lept_value *dst, const lept_value *src);

/**
 * @brief Moves a JSON value.
 * 
 * @param dst Destination JSON value
 * @param src Source JSON value
 */
void lept_move(lept_value *dst, lept_value *src);

/**
 * @brief Swaps two JSON values.
 * 
 * @param lhs Left-hand side JSON value
 * @param rhs Right-hand side JSON value
 */
void lept_swap(lept_value *lhs, lept_value *rhs);

/**
 * @brief Frees a JSON value.
 * 
 * @param v JSON value to be freed
 */
void lept_free(lept_value *v);

/**
 * @brief Gets the type of a JSON value.
 * 
 * @param v JSON value
 * @return lept_type Type of the JSON value
 */
lept_type lept_get_type(const lept_value *v);

/**
 * @brief Checks if two JSON values are equal.
 * 
 * @param lhs Left-hand side JSON value
 * @param rhs Right-hand side JSON value
 * @return int 1 if equal, 0 otherwise
 */
int lept_is_equal(const lept_value *lhs, const lept_value *rhs);

/**
 * @brief Sets a JSON value to null.
 * 
 * @param v JSON value
 */
#define lept_set_null(v) lept_free(v)

/**
 * @brief Gets the boolean value of a JSON value.
 * 
 * @param v JSON value
 * @return int Boolean value (0 or 1)
 */
int lept_get_boolean(const lept_value *v);

/**
 * @brief Sets the boolean value of a JSON value.
 * 
 * @param v JSON value
 * @param b Boolean value (0 or 1)
 */
void lept_set_boolean(lept_value *v, int b);

/**
 * @brief Gets the number value of a JSON value.
 * 
 * @param v JSON value
 * @return double Number value
 */
double lept_get_number(const lept_value *v);

/**
 * @brief Sets the number value of a JSON value.
 * 
 * @param v JSON value
 * @param n Number value
 */
void lept_set_number(lept_value *v, double n);

/**
 * @brief Gets the string value of a JSON value.
 * 
 * @param v JSON value
 * @return const char* String value
 */
const char *lept_get_string(const lept_value *v);

/**
 * @brief Gets the length of the string value of a JSON value.
 * 
 * @param v JSON value
 * @return size_t Length of the string value
 */
size_t lept_get_string_length(const lept_value *v);

/**
 * @brief Sets the string value of a JSON value.
 * 
 * @param v JSON value
 * @param s String value
 * @param len Length of the string value
 */
void lept_set_string(lept_value *v, const char *s, size_t len);

/**
 * @brief Sets the array value of a JSON value.
 * 
 * @param v JSON value
 * @param capacity Capacity of the array
 */
void lept_set_array(lept_value *v, size_t capacity);

/**
 * @brief Gets the size of the array value of a JSON value.
 * 
 * @param v JSON value
 * @return size_t Size of the array
 */
size_t lept_get_array_size(const lept_value *v);

/**
 * @brief Gets the capacity of the array value of a JSON value.
 * 
 * @param v JSON value
 * @return size_t Capacity of the array
 */
size_t lept_get_array_capacity(const lept_value *v);

/**
 * @brief Reserves the capacity of the array value of a JSON value.
 * 
 * @param v JSON value
 * @param capacity Capacity to be reserved
 */
void lept_reserve_array(lept_value *v, size_t capacity);

/**
 * @brief Shrinks the capacity of the array value of a JSON value.
 * 
 * @param v JSON value
 */
void lept_shrink_array(lept_value *v);

/**
 * @brief Clears the array value of a JSON value.
 * 
 * @param v JSON value
 */
void lept_clear_array(lept_value *v);

/**
 * @brief Gets an element of the array value of a JSON value.
 * 
 * @param v JSON value
 * @param index Index of the element
 * @return lept_value* Pointer to the element
 */
lept_value *lept_get_array_element(const lept_value *v, size_t index);

/**
 * @brief Pushes back an element to the array value of a JSON value.
 * 
 * @param v JSON value
 * @return lept_value* Pointer to the pushed element
 */
lept_value *lept_pushback_array_element(lept_value *v);

/**
 * @brief Pops back an element from the array value of a JSON value.
 * 
 * @param v JSON value
 */
void lept_popback_array_element(lept_value *v);

/**
 * @brief Inserts an element to the array value of a JSON value.
 * 
 * @param v JSON value
 * @param index Index of the element
 * @return lept_value* Pointer to the inserted element
 */
lept_value *lept_insert_array_element(lept_value *v, size_t index);

/**
 * @brief Erases elements from the array value of a JSON value.
 * 
 * @param v JSON value
 * @param index Index of the first element to be erased
 * @param count Number of elements to be erased
 */
void lept_erase_array_element(lept_value *v, size_t index, size_t count);

/**
 * @brief Sets the object value of a JSON value.
 * 
 * @param v JSON value
 * @param capacity Capacity of the object
 */
void lept_set_object(lept_value *v, size_t capacity);

/**
 * @brief Gets the size of the object value of a JSON value.
 * 
 * @param v JSON value
 * @return size_t Size of the object
 */
size_t lept_get_object_size(const lept_value *v);

/**
 * @brief Gets the capacity of the object value of a JSON value.
 * 
 * @param v JSON value
 * @return size_t Capacity of the object
 */
size_t lept_get_object_capacity(const lept_value *v);

/**
 * @brief Reserves the capacity of the object value of a JSON value.
 * 
 * @param v JSON value
 * @param capacity Capacity to be reserved
 */
void lept_reserve_object(lept_value *v, size_t capacity);

/**
 * @brief Shrinks the capacity of the object value of a JSON value.
 * 
 * @param v JSON value
 */
void lept_shrink_object(lept_value *v);

/**
 * @brief Clears the object value of a JSON value.
 * 
 * @param v JSON value
 */
void lept_clear_object(lept_value *v);

/**
 * @brief Gets the key of an object member.
 * 
 * @param v JSON value
 * @param index Index of the member
 * @return const char* Key of the member
 */
const char *lept_get_object_key(const lept_value *v, size_t index);

/**
 * @brief Gets the length of the key of an object member.
 * 
 * @param v JSON value
 * @param index Index of the member
 * @return size_t Length of the key
 */
size_t lept_get_object_key_length(const lept_value *v, size_t index);

/**
 * @brief Gets the value of an object member.
 * 
 * @param v JSON value
 * @param index Index of the member
 * @return lept_value* Value of the member
 */
lept_value *lept_get_object_value(const lept_value *v, size_t index);

/**
 * @brief Finds the index of an object member by key.
 * 
 * @param v JSON value
 * @param key Key of the member
 * @param klen Length of the key
 * @return size_t Index of the member, or LEPT_KEY_NOT_EXIST if not found
 */
size_t lept_find_object_index(const lept_value *v, const char *key,
                              size_t klen);

/**
 * @brief Finds the value of an object member by key.
 * 
 * @param v JSON value
 * @param key Key of the member
 * @param klen Length of the key
 * @return lept_value* Value of the member, or NULL if not found
 */
lept_value *lept_find_object_value(lept_value *v, const char *key, size_t klen);

/**
 * @brief Sets the value of an object member by key.
 * 
 * @param v JSON value
 * @param key Key of the member
 * @param klen Length of the key
 * @return lept_value* Value of the member
 */
lept_value *lept_set_object_value(lept_value *v, const char *key, size_t klen);

/**
 * @brief Removes an object member by index.
 * 
 * @param v JSON value
 * @param index Index of the member
 */
void lept_remove_object_value(lept_value *v, size_t index);

#endif
