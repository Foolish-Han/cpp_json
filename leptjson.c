#include "leptjson.h"
#include <assert.h> /* assert() */
#include <errno.h>  /* ERANGE, errno */
#include <math.h>   /* HUGE_VAL */
#include <stdio.h>
#include <stdlib.h> /* NULL, malloc(), realloc(), free(), strtod() */
#include <string.h> /* memcpy() */

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif /* ifndef LEPT_PARSE_STACK_INIT_SIZE */

#define EXPECT(c, ch)                                                          \
  do {                                                                         \
    assert(*c->json == (ch));                                                  \
    c->json++;                                                                 \
  } while (0)

#define ISDIGIT(ch) ((ch >= '0' && ch <= '9'))
#define ISDIGIT1TO9(ch) ((ch >= '1' && ch <= '9'))
#define ISSPACE(ch) ((ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'))
#define PUTC(c, ch)                                                            \
  do {                                                                         \
    *(char *)lept_context_push(c, sizeof(char)) = (ch);                        \
  } while (0)

typedef struct {
  const char *json;
  char *stack;
  size_t size, top;
} lept_context;

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

static void *lept_context_pop(lept_context *c, size_t size) {
  assert(c->top >= size);
  return c->stack + (c->top -= size);
}

static void lept_parse_whitespace(lept_context *c) {
  const char *p = c->json;
  while (ISSPACE(*p)) {
    p++;
  }
  c->json = p;
}

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
  v->n = strtod(c->json, NULL);
  if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) {
    return LEPT_PARSE_NUMBER_TOO_BIG;
  }
  v->type = LEPT_NUMBER;
  c->json = p;

  return LEPT_PARSE_OK;
}

static char get_escape_char(char ch) {}
static int lept_parse_string(lept_context *c, lept_value *v) {
  size_t head = c->top, len;
  const char *p;
  EXPECT(c, '\"');
  p = c->json;
  while (1) {
    char ch = *p++;
    switch (ch) {
    case '\"':
      len = c->top - head;
      lept_set_string(v, (const char *)lept_context_pop(c, len), len);
      c->json = p;
      return LEPT_PARSE_OK;
    case '\0':
      c->top = head;
      return LEPT_PARSE_MISS_QUOTATION_MARK;
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
      default:
        c->top = head;
        return LEPT_PARSE_INVALID_STRING_ESCAPE;
      }
      break;
    default:
      if ((unsigned char)ch < 0x20) {
        c->top = head;
        return LEPT_PARSE_INVALID_STRING_CHAR;
      }
      PUTC(c, ch);
    }
  }
}

static int lept_parse_value(lept_context *c, lept_value *v) {
  switch (*c->json) {
  case 'n':
    return lept_parse_literal(c, v, "null", LEPT_NULL);
  case 't':
    return lept_parse_literal(c, v, "true", LEPT_TRUE);
  case 'f':
    return lept_parse_literal(c, v, "false", LEPT_FALSE);
  case '\0':
    return LEPT_PARSE_EXPECT_VALUE;
  case '\"':
    return lept_parse_string(c, v);
  default:
    return lept_parse_number(c, v);
  }
}

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

void lept_free(lept_value *v) {
  assert(v != NULL);
  if (v->type == LEPT_STRING) {
    free(v->u.s.s);
  }
  v->type = LEPT_NULL;
}

lept_type lept_get_type(const lept_value *v) {
  assert(v != NULL);
  return v->type;
}

int lept_get_boolean(const lept_value *v) {
  assert(v != NULL && (v->type == LEPT_FALSE || v->type == LEPT_TRUE));
  return (v->type == LEPT_FALSE) ? 0 : 1;
}

void lept_set_boolean(lept_value *v, int b) {
  lept_free(v);
  v->type = b ? LEPT_TRUE : LEPT_FALSE;
}

double lept_get_number(const lept_value *v) {
  assert(v != NULL && v->type == LEPT_NUMBER);
  return v->n;
}

void lept_set_number(lept_value *v, double n) {
  lept_free(v);
  v->n = n;
  v->type = LEPT_NUMBER;
}

const char *lept_get_string(const lept_value *v) {
  assert(v != NULL && v->type == LEPT_STRING);
  return v->u.s.s;
}

size_t lept_get_string_length(const lept_value *v) {
  assert(v != NULL && v->type == LEPT_STRING);
  return v->u.s.len;
}

void lept_set_string(lept_value *v, const char *s, size_t len) {
  assert(v != NULL && (s != NULL || len == 0));
  lept_free(v);
  v->u.s.s = (char *)malloc(len + 1);
  memcpy(v->u.s.s, s, len);
  v->u.s.s[len] = '\0';
  v->u.s.len = len;
  v->type = LEPT_STRING;
}
