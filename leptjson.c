#include "leptjson.h"
#include <alloca.h>
#include <assert.h> /* assert() */
#include <errno.h>  /* ERANGE, errno */
#include <math.h>   /* HUGE_VAL */
#include <stddef.h>
#include <stdio.h>  /* sprintf */
#include <stdlib.h> /* NULL, malloc(), realloc(), free(), strtod() */
#include <string.h> /* memcpy() */

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif /* ifndef LEPT_PARSE_STACK_INIT_SIZE */

#ifndef LEPT_PARSE_STRINGFY_INIT_SIZE
#define LEPT_PARSE_STRINGFY_INIT_SIZE 256
#endif

#define EXPECT(c, ch)                                                          \
  do {                                                                         \
    assert(*c->json == (ch));                                                  \
    c->json++;                                                                 \
  } while (0)

#define ISDIGIT(ch) ((ch >= '0' && ch <= '9'))
#define ISDIGIT1TO9(ch) ((ch >= '1' && ch <= '9'))
#define ISSPACE(ch) ((ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'))
#define UNESCAPE_CHAR(c)                                                       \
  ((c) == '\n'   ? 'n'                                                         \
   : (c) == '\t' ? 't'                                                         \
   : (c) == '\r' ? 'r'                                                         \
   : (c) == '\b' ? 'b'                                                         \
   : (c) == '\f' ? 'f'                                                         \
   : (c) == '\\' ? '\\'                                                        \
   : (c) == '\"' ? '\"'                                                        \
   : (c) == '/'  ? '/'                                                         \
                 : ('\0'))
#define PUTC(c, ch)                                                            \
  do {                                                                         \
    *(char *)lept_context_push(c, sizeof(char)) = (ch);                        \
  } while (0)
#define PUTS(c, s, len)                                                        \
  do {                                                                         \
    memcpy(lept_context_push(c, len), s, len);                                 \
  } while (0)
#define STRING_ERROR(ret)                                                      \
  do {                                                                         \
    c->top = head;                                                             \
    return ret;                                                                \
  } while (0)

/**
 * @brief Context structure for parsing JSON.
 */
typedef struct {
  const char *json; /**< JSON string to be parsed */
  char *stack;      /**< Stack for storing intermediate values */
  size_t size;      /**< Size of the stack */
  size_t top;       /**< Top of the stack */
} lept_context;

/**
 * @brief Parses a JSON value.
 * 
 * @param c Context for parsing
 * @param v JSON value to be parsed
 * @return int Parsing result
 */
static int lept_parse_value(lept_context *c, lept_value *v);

/**
 * @brief Pushes a value onto the context stack.
 * 
 * @param c Context for parsing
 * @param size Size of the value to be pushed
 * @return void* Pointer to the pushed value
 */
static void *lept_context_push(lept_context *c, size_t size) {
  void *ret;
  assert(c != NULL && size > 0);
  if (c->top + size >= c->size) {
    if (c->size == 0) {
      c->size = LEPT_PARSE_STACK_INIT_SIZE;
    }
    while (c->top + size >= c->size) {
      /* c->size * 1.5 */
      c->size += c->size >> 1;
    }
    c->stack = (char *)realloc(c->stack, c->size);
  }
  ret = c->stack + c->top;
  c->top += size;
  return ret;
}

/**
 * @brief Pops a value from the context stack.
 * 
 * @param c Context for parsing
 * @param size Size of the value to be popped
 * @return void* Pointer to the popped value
 */
static void *lept_context_pop(lept_context *c, size_t size) {
  assert(c->top >= size);
  return c->stack + (c->top -= size);
}

/**
 * @brief Parses whitespace characters in the JSON string.
 * 
 * @param c Context for parsing
 */
static void lept_parse_whitespace(lept_context *c) {
  const char *p = c->json;
  while (ISSPACE(*p)) {
    p++;
  }
  c->json = p;
}

/**
 * @brief Parses a literal value (null, true, false) in the JSON string.
 * 
 * @param c Context for parsing
 * @param v JSON value to be parsed
 * @param literal Literal string to be matched
 * @param type Type of the literal value
 * @return int Parsing result
 */
static int lept_parse_literal(lept_context *c, lept_value *v,
                              const char *literal, lept_type type) {
  EXPECT(c, *literal);
  literal++;
  while (*literal) {
    if (*literal != *(c->json)) {
      return LEPT_PARSE_INVALID_VALUE;
    }
    literal++;
    c->json++;
  }
  v->type = type;
  return LEPT_PARSE_OK;
}

/**
 * @brief Parses a number value in the JSON string.
 * 
 * @param c Context for parsing
 * @param v JSON value to be parsed
 * @return int Parsing result
 */
static int lept_parse_number(lept_context *c, lept_value *v) {
  const char *p = c->json;

  /* Negative sign */
  if (*p == '-') {
    p++;
  }
  /* Integer part */
  if (*p == '0') {
    p++;
  } else {
    if (!ISDIGIT1TO9(*p)) {
      return LEPT_PARSE_INVALID_VALUE;
    }
    p++;
    while (ISDIGIT(*p)) {
      p++;
    }
  }
  /* Decimal point */
  if (*p == '.') {
    p++;
    if (!ISDIGIT(*p)) {
      return LEPT_PARSE_INVALID_VALUE;
    }
    p++;
    while (ISDIGIT(*p)) {
      p++;
    }
  }
  /* Exponent symbol */
  if (*p == 'E' || *p == 'e') {
    p++;
    if (*p == '+' || *p == '-') {
      p++;
    }
    if (!ISDIGIT(*p)) {
      return LEPT_PARSE_INVALID_VALUE;
    }
    p++;
    while (ISDIGIT(*p)) {
      p++;
    }
  }

  errno = 0;
  v->u.n = strtod(c->json, NULL);
  if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL)) {
    return LEPT_PARSE_NUMBER_TOO_BIG;
  }
  v->type = LEPT_NUMBER;
  c->json = p;

  return LEPT_PARSE_OK;
}

/**
 * @brief Parses a 4-digit hexadecimal number in the JSON string.
 * 
 * @param p Pointer to the JSON string
 * @param u Pointer to the parsed value
 * @return const char* Pointer to the next character in the JSON string
 */
static const char *lept_parse_hex4(const char *p, unsigned *u) {
  *u = 0;
  for (int i = 0; i < 4; i++) {
    char ch = *p++;
    *u <<= 4;
    if (ch >= '0' && ch <= '9') {
      *u |= ch - '0';
    } else if (ch >= 'A' && ch <= 'F') {
      *u |= ch - ('A' - 10);
    } else if (ch >= 'a' && ch <= 'f') {
      *u |= ch - ('a' - 10);
    } else {
      return NULL;
    }
  }
  return p;
}

/**
 * @brief Encodes a Unicode code point as UTF-8 and pushes it onto the context stack.
 * 
 * @param c Context for parsing
 * @param u Unicode code point to be encoded
 */
static void lept_encode_utf8(lept_context *c, unsigned u) {
  if (u <= 0x007F) {
    PUTC(c, (u & 0x7F));
  } else if (u <= 0x07FF) {
    PUTC(c, (0xC0 | (u >> 6) & 0x1F));
    PUTC(c, (0x80 | u & 0x3F));
  } else if (u <= 0xFFFF) {
    PUTC(c, (0xE0 | (u >> 12) & 0x0F));
    PUTC(c, (0x80 | (u >> 6) & 0x3F));
    PUTC(c, (0x80 | u & 0x3F));
  } else {
    PUTC(c, (0xF0 | (u >> 18) & 0x07));
    PUTC(c, (0x80 | (u >> 12) & 0x3F));
    PUTC(c, (0x80 | (u >> 6) & 0x3F));
    PUTC(c, (0x80 | u & 0x3F));
  }
}

/**
 * @brief Parses a raw string value in the JSON string.
 * 
 * @param c Context for parsing
 * @param str Pointer to the parsed string
 * @param len Pointer to the length of the parsed string
 * @return int Parsing result
 */
static int lept_parse_string_raw(lept_context *c, char **str, size_t *len) {
  size_t head = c->top;
  const char *p;
  unsigned u, u2;
  EXPECT(c, '\"');
  p = c->json;
  while (1) {
    char ch = *p++;
    switch (ch) {
    case '\"':
      *len = c->top - head;
      c->json = p;
      *str = lept_context_pop(c, *len);
      return LEPT_PARSE_OK;
    case '\0':
      STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);
    case '\\':
      switch (*p++) {
      case '\"':
        PUTC(c, '\"');
        break;
      case '\\':
        PUTC(c, '\\');
        break;
      case '/':
        PUTC(c, '/');
        break;
      case 'b':
        PUTC(c, '\b');
        break;
      case 'f':
        PUTC(c, '\f');
        break;
      case 'n':
        PUTC(c, '\n');
        break;
      case 'r':
        PUTC(c, '\r');
        break;
      case 't':
        PUTC(c, '\t');
        break;
      case 'u':
        if (!(p = lept_parse_hex4(p, &u))) {
          STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
        }
        if (u >= 0xD800 && u <= 0xDBFF) {
          if (*(p++) != '\\') {
            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
          }
          if (*(p++) != 'u') {
            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
          }
          if (!(p = lept_parse_hex4(p, &u2))) {
            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
          }
          if (u2 < 0xDC00 || u2 > 0xDFFF) {
            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
          }
          u = 0x10000 + (((u - 0xD800) << 10) | (u2 - 0xDC00));
        }
        lept_encode_utf8(c, u);
        break;
      default:
        STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
      }
      break;
    default:
      if ((unsigned char)ch < 0x20) {
        STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
      }
      PUTC(c, ch);
    }
  }
}

/**
 * @brief Parses a string value in the JSON string.
 * 
 * @param c Context for parsing
 * @param v JSON value to be parsed
 * @return int Parsing result
 */
static int lept_parse_string(lept_context *c, lept_value *v) {
  size_t len;
  char *s = NULL;
  int ret;
  if ((ret = lept_parse_string_raw(c, &s, &len)) == LEPT_PARSE_OK) {
    lept_set_string(v, s, len);
  }
  return ret;
}

/**
 * @brief Parses an array value in the JSON string.
 * 
 * @param c Context for parsing
 * @param v JSON value to be parsed
 * @return int Parsing result
 */
static int lept_parse_array(lept_context *c, lept_value *v) {
  size_t size = 0;
  int ret;
  EXPECT(c, '[');
  lept_parse_whitespace(c);
  if (*c->json == ']') {
    c->json++;
    lept_set_array(v, 0);
    return LEPT_PARSE_OK;
  }
  while (1) {
    lept_value e;
    lept_init(&e);
    if ((ret = lept_parse_value(c, &e)) != LEPT_PARSE_OK) {
      break;
    }
    memcpy(lept_context_push(c, sizeof(lept_value)), &e, sizeof(lept_value));
    size++;
    lept_parse_whitespace(c);
    if (*c->json == ',') {
      c->json++;
      lept_parse_whitespace(c);
    } else if (*c->json == ']') {
      c->json++;
      lept_set_array(v, size);
      memcpy(v->u.a.e, lept_context_pop(c, size * sizeof(lept_value)),
             size * sizeof(lept_value));
      v->u.a.size = size;
      return LEPT_PARSE_OK;
    } else {
      ret = LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
      break;
    }
  }
  for (size_t i = 0; i < size; ++i) {
    lept_free((lept_value *)lept_context_pop(c, sizeof(lept_value)));
  }
  return ret;
}

/**
 * @brief Parses an object value in the JSON string.
 * 
 * @param c Context for parsing
 * @param v JSON value to be parsed
 * @return int Parsing result
 */
static int lept_parse_object(lept_context *c, lept_value *v) {
  size_t size;
  lept_member m;
  int ret;
  char *str;
  EXPECT(c, '{');
  lept_parse_whitespace(c);
  if (*c->json == '}') {
    c->json++;
    lept_set_object(v, 0);
    return LEPT_PARSE_OK;
  }
  m.k = NULL;
  size = 0;
  while (1) {
    lept_init(&m.v);
    if (*c->json != '"') {
      ret = LEPT_PARSE_MISS_KEY;
      break;
    }
    if ((ret = lept_parse_string_raw(c, &str, &m.klen)) != LEPT_PARSE_OK) {
      break;
    }
    m.k = (char *)malloc(m.klen + 1);
    memcpy(m.k, str, m.klen);
    m.k[m.klen] = '\0';
    lept_parse_whitespace(c);
    if (*c->json != ':') {
      ret = LEPT_PARSE_MISS_COLON;
      break;
    }
    c->json++;
    lept_parse_whitespace(c);
    if ((ret = lept_parse_value(c, &m.v)) != LEPT_PARSE_OK) {
      break;
    }
    memcpy(lept_context_push(c, sizeof(lept_member)), &m, sizeof(lept_member));
    size++;
    m.k = NULL;
    lept_parse_whitespace(c);
    if (*c->json == ',') {
      c->json++;
      lept_parse_whitespace(c);
    } else if (*c->json == '}') {
      c->json++;
      lept_set_object(v, size);
      memcpy(v->u.o.m, lept_context_pop(c, size * sizeof(lept_member)),
             size * sizeof(lept_member));
      v->u.o.size = size;
      return LEPT_PARSE_OK;
    } else {
      ret = LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
      break;
    }
  }
  free(m.k);
  for (size_t i = 0; i < size; i++) {
    lept_member *m = (lept_member *)lept_context_pop(c, sizeof(lept_member));
    lept_free(&m->v);
    free(m->k);
  }
  v->type = LEPT_NULL;
  return ret;
}

/**
 * @brief Parses a JSON value.
 * 
 * @param c Context for parsing
 * @param v JSON value to be parsed
 * @return int Parsing result
 */
static int lept_parse_value(lept_context *c, lept_value *v) {
  switch (*c->json) {
  case 'n':
    return lept_parse_literal(c, v, "null", LEPT_NULL);
  case 't':
    return lept_parse_literal(c, v, "true", LEPT_TRUE);
  case 'f':
    return lept_parse_literal(c, v, "false", LEPT_FALSE);
  case '\"':
    return lept_parse_string(c, v);
  case '[':
    return lept_parse_array(c, v);
  case '\0':
    return LEPT_PARSE_EXPECT_VALUE;
  case '{':
    return lept_parse_object(c, v);
  default:
    return lept_parse_number(c, v);
  }
}

/**
 * @brief Parses a JSON string.
 * 
 * @param v JSON value to be parsed
 * @param json JSON string to be parsed
 * @return int Parsing result
 */
int lept_parse(lept_value *v, const char *json) {
  lept_context c;
  int ret;
  assert(v != NULL);
  c.json = json;
  c.stack = NULL;
  c.top = 0;
  c.size = 0;
  lept_init(v);
  lept_parse_whitespace(&c);
  if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
    lept_parse_whitespace(&c);
    if (*c.json != '\0') {
      v->type = LEPT_NULL;
      ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
  }
  assert(c.top == 0);
  free(c.stack);
  return ret;
}

/**
 * @brief Stringifies a string value and pushes it onto the context stack.
 * 
 * @param c Context for parsing
 * @param s String value to be stringified
 * @param len Length of the string value
 */
static void lept_stringify_string(lept_context *c, const char *s, size_t len) {
  static const char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  size_t i, size;
  char *head, *p;
  assert(s != NULL);
  p = head = lept_context_push(c, size = len * 6 + 2); /* "\u00xx..." */
  *p++ = '"';
  for (i = 0; i < len; i++) {
    unsigned char ch = (unsigned char)s[i];
    switch (ch) {
    case '\"':
      *p++ = '\\';
      *p++ = '\"';
      break;
    case '\\':
      *p++ = '\\';
      *p++ = '\\';
      break;
    case '\b':
      *p++ = '\\';
      *p++ = 'b';
      break;
    case '\f':
      *p++ = '\\';
      *p++ = 'f';
      break;
    case '\n':
      *p++ = '\\';
      *p++ = 'n';
      break;
    case '\r':
      *p++ = '\\';
      *p++ = 'r';
      break;
    case '\t':
      *p++ = '\\';
      *p++ = 't';
      break;
    default:
      if (ch < 0x20) {
        *p++ = '\\';
        *p++ = 'u';
        *p++ = '0';
        *p++ = '0';
        *p++ = hex_digits[ch >> 4];
        *p++ = hex_digits[ch & 15];
      } else
        *p++ = s[i];
    }
  }
  *p++ = '"';
  c->top -= size - (p - head);
}

/**
 * @brief Stringifies a JSON value and pushes it onto the context stack.
 * 
 * @param c Context for parsing
 * @param v JSON value to be stringified
 */
static void lept_stringify_value(lept_context *c, const lept_value *v) {
  size_t i, n;
  switch (v->type) {
  case LEPT_NULL:
    PUTS(c, "null", 4);
    break;
  case LEPT_FALSE:
    PUTS(c, "false", 5);
    break;
  case LEPT_TRUE:
    PUTS(c, "true", 4);
    break;
  case LEPT_NUMBER:
    c->top -= 32 - sprintf(lept_context_push(c, 32), "%.17g", v->u.n);
    break;
  case LEPT_STRING:
    lept_stringify_string(c, v->u.s.s, v->u.s.len);
    break;
  case LEPT_ARRAY:
    PUTC(c, '[');
    for (i = 0; i < v->u.a.size; i++) {
      if (i > 0)
        PUTC(c, ',');
      lept_stringify_value(c, &v->u.a.e[i]);
    }
    PUTC(c, ']');
    break;
  case LEPT_OBJECT:
    PUTC(c, '{');
    for (i = 0; i < v->u.o.size; i++) {
      if (i > 0)
        PUTC(c, ',');
      lept_stringify_string(c, v->u.o.m[i].k, v->u.o.m[i].klen);
      PUTC(c, ':');
      lept_stringify_value(c, &v->u.o.m[i].v);
    }
    PUTC(c, '}');
    break;
  default:
    assert(0 && "invalid type");
    break;
  }
}

/**
 * @brief Stringifies a JSON value.
 * 
 * @param v JSON value to be stringified
 * @param length Pointer to the length of the stringified value
 * @return char* Stringified JSON value
 */
char *lept_stringify(const lept_value *v, size_t *length) {
  lept_context c;
  assert(v != NULL);
  c.stack = (char *)malloc(c.size = LEPT_PARSE_STRINGFY_INIT_SIZE);
  c.top = 0;
  lept_stringify_value(&c, v);
  if (length) {
    *length = c.top;
  }
  PUTC(&c, '\0');
  return c.stack;
}

/**
 * @brief Copies a JSON value.
 * 
 * @param dst Destination JSON value
 * @param src Source JSON value
 */
void lept_copy(lept_value *dst, const lept_value *src) {
  assert(src != NULL && dst != NULL && src != dst);
  switch (src->type) {
  case LEPT_STRING:
    lept_set_string(dst, src->u.s.s, src->u.s.len);
    break;
  case LEPT_ARRAY:
    lept_set_array(dst, src->u.a.capacity);
    for (size_t i = 0; i < src->u.a.size; i++) {
      lept_copy(lept_pushback_array_element(dst), src->u.a.e + i);
    }
    break;
  case LEPT_OBJECT:
    lept_set_object(dst, src->u.a.capacity);
    for (size_t i = 0; i < src->u.a.size; i++) {
      lept_copy(lept_set_object_value(dst, src->u.o.m[i].k, src->u.o.m[i].klen),
                &src->u.o.m[i].v);
    }
    break;
  default:
    memcpy(dst, src, sizeof(lept_value));
    break;
  }
}

/**
 * @brief Moves a JSON value.
 * 
 * @param dst Destination JSON value
 * @param src Source JSON value
 */
void lept_move(lept_value *dst, lept_value *src) {
  assert(src != NULL && dst != NULL && src != dst);
  lept_free(dst);
  memcpy(dst, src, sizeof(lept_value));
  lept_init(src);
}

/**
 * @brief Swaps two JSON values.
 * 
 * @param lhs Left-hand side JSON value
 * @param rhs Right-hand side JSON value
 */
void lept_swap(lept_value *lhs, lept_value *rhs) {
  assert(lhs != NULL && rhs != NULL);
  if (lhs != rhs) {
    lept_value temp;
    memcpy(&temp, lhs, sizeof(lept_value));
    memcpy(lhs, rhs, sizeof(lept_value));
    memcpy(rhs, &temp, sizeof(lept_value));
  }
}

/**
 * @brief Frees a JSON value.
 * 
 * @param v JSON value to be freed
 */
void lept_free(lept_value *v) {
  size_t i;
  assert(v != NULL);
  switch (v->type) {
  case LEPT_STRING:
    free(v->u.s.s);
    break;
  case LEPT_ARRAY:
    for (i = 0; i < v->u.a.size; i++) {
      lept_free(&v->u.a.e[i]);
    }
    free(v->u.a.e);
    break;
  case LEPT_OBJECT:
    for (i = 0; i < v->u.o.size; i++) {
      lept_free(&v->u.o.m[i].v);
      free(v->u.o.m[i].k);
    }
    free(v->u.o.m);
    break;
  default:
    break;
  }
  v->type = LEPT_NULL;
}

/**
 * @brief Checks if two JSON values are equal.
 * 
 * @param lhs Left-hand side JSON value
 * @param rhs Right-hand side JSON value
 * @return int 1 if equal, 0 otherwise
 */
int lept_is_equal(const lept_value *lhs, const lept_value *rhs) {
  size_t i;
  assert(lhs != NULL && rhs != NULL);
  if (lhs->type != rhs->type) {
    return 0;
  }
  switch (lhs->type) {
  case LEPT_STRING:
    return lhs->u.s.len == rhs->u.s.len &&
           memcmp(lhs->u.s.s, rhs->u.s.s, lhs->u.s.len) == 0;
  case LEPT_NUMBER:
    return lhs->u.n == rhs->u.n;
  case LEPT_ARRAY:
    if (lhs->u.a.size != rhs->u.a.size) {
      return 0;
    }
    for (i = 0; i < lhs->u.a.size; i++) {
      if (lept_is_equal(lhs->u.a.e + i, rhs->u.a.e + i) == 0) {
        return 0;
      }
    }
    return 1;
  case LEPT_OBJECT:
    if (lhs->u.o.size != rhs->u.o.size) {
      return 0;
    }
    for (i = 0; i < lhs->u.o.size; i++) {
      size_t index;
      if ((index = lept_find_object_index(rhs, lhs->u.o.m[i].k,
                                          lhs->u.o.m[i].klen)) ==
          LEPT_KEY_NOT_EXIST) {
        return index;
      }
      if (lept_is_equal(&lhs->u.o.m[i].v, &rhs->u.o.m[index].v) == 0) {
        return 0;
      }
    }
    return 1;
  default:
    return lhs->type == rhs->type;
  }
}

/**
 * @brief Gets the type of a JSON value.
 * 
 * @param v JSON value
 * @return lept_type Type of the JSON value
 */
lept_type lept_get_type(const lept_value *v) {
  assert(v != NULL);
  return v->type;
}

/**
 * @brief Gets the boolean value of a JSON value.
 * 
 * @param v JSON value
 * @return int Boolean value (0 or 1)
 */
int lept_get_boolean(const lept_value *v) {
  assert(v != NULL && (v->type == LEPT_FALSE || v->type == LEPT_TRUE));
  return (v->type == LEPT_FALSE) ? 0 : 1;
}

/**
 * @brief Sets the boolean value of a JSON value.
 * 
 * @param v JSON value
 * @param b Boolean value (0 or 1)
 */
void lept_set_boolean(lept_value *v, int b) {
  lept_free(v);
  v->type = b ? LEPT_TRUE : LEPT_FALSE;
}

/**
 * @brief Gets the number value of a JSON value.
 * 
 * @param v JSON value
 * @return double Number value
 */
double lept_get_number(const lept_value *v) {
  assert(v != NULL && v->type == LEPT_NUMBER);
  return v->u.n;
}

/**
 * @brief Sets the number value of a JSON value.
 * 
 * @param v JSON value
 * @param n Number value
 */
void lept_set_number(lept_value *v, double n) {
  lept_free(v);
  v->u.n = n;
  v->type = LEPT_NUMBER;
}

/**
 * @brief Gets the string value of a JSON value.
 * 
 * @param v JSON value
 * @return const char* String value
 */
const char *lept_get_string(const lept_value *v) {
  assert(v != NULL && v->type == LEPT_STRING);
  return v->u.s.s;
}

/**
 * @brief Gets the length of the string value of a JSON value.
 * 
 * @param v JSON value
 * @return size_t Length of the string value
 */
size_t lept_get_string_length(const lept_value *v) {
  assert(v != NULL && v->type == LEPT_STRING);
  return v->u.s.len;
}

/**
 * @brief Sets the string value of a JSON value.
 * 
 * @param v JSON value
 * @param s String value
 * @param len Length of the string value
 */
void lept_set_string(lept_value *v, const char *s, size_t len) {
  assert(v != NULL && (s != NULL || len == 0));
  lept_free(v);
  v->u.s.s = (char *)malloc(len + 1);
  memcpy(v->u.s.s, s, len);
  v->u.s.s[len] = '\0';
  v->u.s.len = len;
  v->type = LEPT_STRING;
}

/**
 * @brief Sets the array value of a JSON value.
 * 
 * @param v JSON value
 * @param capacity Capacity of the array
 */
void lept_set_array(lept_value *v, size_t capacity) {
  assert(v != NULL);
  lept_free(v);
  v->type = LEPT_ARRAY;
  v->u.a.size = 0;
  v->u.a.capacity = capacity;
  v->u.a.e =
      capacity > 0 ? (lept_value *)malloc(capacity * sizeof(lept_value)) : NULL;
}

/**
 * @brief Gets the size of the array value of a JSON value.
 * 
 * @param v JSON value
 * @return size_t Size of the array
 */
size_t lept_get_array_size(const lept_value *v) {
  assert(v != NULL && v->type == LEPT_ARRAY);
  return v->u.a.size;
}

/**
 * @brief Gets the capacity of the array value of a JSON value.
 * 
 * @param v JSON value
 * @return size_t Capacity of the array
 */
size_t lept_get_array_capacity(const lept_value *v) {
  assert(v != NULL && v->type == LEPT_ARRAY);
  return v->u.a.capacity;
}

/**
 * @brief Reserves the capacity of the array value of a JSON value.
 * 
 * @param v JSON value
 * @param capacity Capacity to be reserved
 */
void lept_reserve_array(lept_value *v, size_t capacity) {
  assert(v != NULL && v->type == LEPT_ARRAY);
  if (v->u.a.capacity < capacity) {
    v->u.a.capacity = capacity;
    v->u.a.e =
        (lept_value *)realloc(v->u.a.e, v->u.a.capacity * sizeof(lept_value));
  }
}

/**
 * @brief Shrinks the capacity of the array value of a JSON value.
 * 
 * @param v JSON value
 */
void lept_shrink_array(lept_value *v) {
  assert(v != NULL && v->type == LEPT_ARRAY);
  if (v->u.a.capacity > v->u.a.size) {
    v->u.a.capacity = v->u.a.size;
    if (v->u.a.size == 0) {
      free(v->u.a.e);
      v->u.a.e = NULL;
    } else {
      v->u.a.e =
          (lept_value *)realloc(v->u.a.e, v->u.a.capacity * sizeof(lept_value));
    }
  }
}

/**
 * @brief Clears the array value of a JSON value.
 * 
 * @param v JSON value
 */
void lept_clear_array(lept_value *v) {
  assert(v != NULL && v->type == LEPT_ARRAY);
  lept_erase_array_element(v, 0, v->u.a.size);
}

/**
 * @brief Gets an element of the array value of a JSON value.
 * 
 * @param v JSON value
 * @param index Index of the element
 * @return lept_value* Pointer to the element
 */
lept_value *lept_get_array_element(const lept_value *v, size_t index) {
  assert(v != NULL && v->type == LEPT_ARRAY);
  assert(index < v->u.a.size);
  return &v->u.a.e[index];
}

/**
 * @brief Pushes back an element to the array value of a JSON value.
 * 
 * @param v JSON value
 * @return lept_value* Pointer to the pushed element
 */
lept_value *lept_pushback_array_element(lept_value *v) {
  assert(v != NULL && v->type == LEPT_ARRAY);
  if (v->u.a.size == v->u.a.capacity) {
    lept_reserve_array(v, v->u.a.capacity == 0 ? 1 : v->u.a.capacity * 2);
  }
  lept_init(v->u.a.e + v->u.a.size);
  return v->u.a.e + (v->u.a.size++);
}

/**
 * @brief Pops back an element from the array value of a JSON value.
 * 
 * @param v JSON value
 */
void lept_popback_array_element(lept_value *v) {
  assert(v != NULL && v->type == LEPT_ARRAY && v->u.a.size > 0);
  lept_free(v->u.a.e + (--v->u.a.size));
}

/**
 * @brief Inserts an element to the array value of a JSON value.
 * 
 * @param v JSON value
 * @param index Index of the element
 * @return lept_value* Pointer to the inserted element
 */
lept_value *lept_insert_array_element(lept_value *v, size_t index) {
  assert(v != NULL && v->type == LEPT_ARRAY && index <= v->u.a.size);
  lept_reserve_array(v, v->u.a.size + 1);
  for (size_t i = v->u.a.size; i > index; i--) {
    v->u.a.e[i] = v->u.a.e[i - 1];
  }
  v->u.a.size++;
  return v->u.a.e + index;
}

/**
 * @brief Erases elements from the array value of a JSON value.
 * 
 * @param v JSON value
 * @param index Index of the first element to be erased
 * @param count Number of elements to be erased
 */
void lept_erase_array_element(lept_value *v, size_t index, size_t count) {
  assert(v != NULL && v->type == LEPT_ARRAY && index + count <= v->u.a.size);
  for (size_t i = 0; i < count; i++) {
    lept_free(v->u.a.e + (index + i));
  }
  for (size_t i = index + count; i < v->u.a.size; i++) {
    v->u.a.e[i - count] = v->u.a.e[i];
  }
  v->u.a.size -= count;
}

/**
 * @brief Sets the object value of a JSON value.
 * 
 * @param v JSON value
 * @param capacity Capacity of the object
 */
void lept_set_object(lept_value *v, size_t capacity) {
  assert(v != NULL);
  lept_free(v);
  v->type = LEPT_OBJECT;
  v->u.o.size = 0;
  v->u.o.capacity = capacity;
  v->u.o.m = capacity > 0
                 ? (lept_member *)malloc(capacity * sizeof(lept_member))
                 : NULL;
}

/**
 * @brief Gets the capacity of the object value of a JSON value.
 * 
 * @param v JSON value
 * @return size_t Capacity of the object
 */
size_t lept_get_object_capacity(const lept_value *v) {
  assert(v != NULL && v->type == LEPT_OBJECT);
  return v->u.o.capacity;
}

/**
 * @brief Gets the size of the object value of a JSON value.
 * 
 * @param v JSON value
 * @return size_t Size of the object
 */
size_t lept_get_object_size(const lept_value *v) {
  assert(v != NULL && v->type == LEPT_OBJECT);
  return v->u.o.size;
}

/**
 * @brief Reserves the capacity of the object value of a JSON value.
 * 
 * @param v JSON value
 * @param capacity Capacity to be reserved
 */
void lept_reserve_object(lept_value *v, size_t capacity) {
  assert(v != NULL && v->type == LEPT_OBJECT);
  if (v->u.o.capacity < capacity) {
    v->u.o.capacity = capacity;
    v->u.o.m =
        (lept_member *)realloc(v->u.o.m, v->u.o.capacity * sizeof(lept_member));
  }
}

/**
 * @brief Shrinks the capacity of the object value of a JSON value.
 * 
 * @param v JSON value
 */
void lept_shrink_object(lept_value *v) {
  assert(v != NULL && v->type == LEPT_OBJECT);
  if (v->u.o.capacity > v->u.o.size) {
    v->u.o.capacity = v->u.o.size;
    if (v->u.o.size == 0) {
      free(v->u.o.m);
      v->u.o.m = NULL;
    } else {
      v->u.o.m = (lept_member *)realloc(v->u.o.m,
                                        v->u.o.capacity * sizeof(lept_member));
    }
  }
}

/**
 * @brief Clears the object value of a JSON value.
 * 
 * @param v JSON value
 */
void lept_clear_object(lept_value *v) {
  assert(v != NULL && v->type == LEPT_OBJECT);
  for (size_t i = 0; i < v->u.o.size; i++) {
    lept_free(&v->u.o.m[i].v);
    free(v->u.o.m[i].k);
  }
  v->u.o.size = 0;
}

/**
 * @brief Gets the key of an object member.
 * 
 * @param v JSON value
 * @param index Index of the member
 * @return const char* Key of the member
 */
const char *lept_get_object_key(const lept_value *v, size_t index) {
  assert(v != NULL && v->type == LEPT_OBJECT);
  assert(index < v->u.o.size);
  return v->u.o.m[index].k;
}

/**
 * @brief Gets the length of the key of an object member.
 * 
 * @param v JSON value
 * @param index Index of the member
 * @return size_t Length of the key
 */
size_t lept_get_object_key_length(const lept_value *v, size_t index) {
  assert(v != NULL && v->type == LEPT_OBJECT);
  assert(index < v->u.o.size);
  return v->u.o.m[index].klen;
}

/**
 * @brief Gets the value of an object member.
 * 
 * @param v JSON value
 * @param index Index of the member
 * @return lept_value* Value of the member
 */
lept_value *lept_get_object_value(const lept_value *v, size_t index) {
  assert(v != NULL && v->type == LEPT_OBJECT);
  assert(index < v->u.o.size);
  return &v->u.o.m[index].v;
}

/**
 * @brief Finds the index of an object member by key.
 * 
 * @param v JSON value
 * @param key Key of the member
 * @param klen Length of the key
 * @return size_t Index of the member, or LEPT_KEY_NOT_EXIST if not found
 */
size_t lept_find_object_index(const lept_value *v, const char *key,
                              size_t klen) {
  assert(v != NULL && v->type == LEPT_OBJECT && key != NULL);
  for (size_t i = 0; i < v->u.o.size; ++i) {
    if (v->u.o.m[i].klen == klen && memcmp(v->u.o.m[i].k, key, klen) == 0) {
      return i;
    }
  }
  return LEPT_KEY_NOT_EXIST;
}

/**
 * @brief Finds the value of an object member by key.
 * 
 * @param v JSON value
 * @param key Key of the member
 * @param klen Length of the key
 * @return lept_value* Value of the member, or NULL if not found
 */
lept_value *lept_find_object_value(lept_value *v, const char *key,
                                   size_t klen) {
  size_t index = lept_find_object_index(v, key, klen);
  return index != LEPT_KEY_NOT_EXIST ? &v->u.o.m[index].v : NULL;
}

/**
 * @brief Sets the value of an object member by key.
 * 
 * @param v JSON value
 * @param key Key of the member
 * @param klen Length of the key
 * @return lept_value* Value of the member
 */
lept_value *lept_set_object_value(lept_value *v, const char *key, size_t klen) {
  assert(v != NULL && v->type == LEPT_OBJECT && key != NULL);
  lept_reserve_object(v, v->u.o.size + 1);
  memcpy((v->u.o.m[v->u.o.size].k = (char *)malloc(klen)), key, klen);
  v->u.o.m[v->u.o.size].klen = klen;
  lept_init(&v->u.o.m[v->u.o.size].v);
  return &v->u.o.m[v->u.o.size++].v;
}

/**
 * @brief Removes an object member by index.
 * 
 * @param v JSON value
 * @param index Index of the member
 */
void lept_remove_object_value(lept_value *v, size_t index) {
  assert(v != NULL && v->type == LEPT_OBJECT && index < v->u.o.size);
  lept_free(&v->u.o.m[index].v);
  free(v->u.o.m[index].k);
  for (size_t i = index + 1; i < v->u.a.size; i++) {
    v->u.o.m[i - 1] = v->u.o.m[i];
  }
  v->u.o.size -= 1;
}
