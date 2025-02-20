#include "leptjson.h"
#include <assert.h> /* assert() */
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h> /* NULL, strtod() */

#define EXPECT(c, ch)                                                          \
  do {                                                                         \
    assert(*c->json == (ch));                                                  \
    c->json++;                                                                 \
  } while (0)

typedef struct {
  const char *json;
} lept_context;

#define ISDIGIT(ch) ((ch >= '0' && ch <= '9'))
#define ISDIGIT1TO9(ch) ((ch >= '1' && ch <= '9'))
#define ISSPACE(ch) ((ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'))

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
  default:
    return lept_parse_number(c, v);
  }
}

int lept_parse(lept_value *v, const char *json) {
  lept_context c;
  int ret;
  assert(v != NULL);
  c.json = json;
  v->type = LEPT_NULL;
  lept_parse_whitespace(&c);
  if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
    lept_parse_whitespace(&c);
    if (*c.json != '\0') {
      v->type = LEPT_NULL;
      ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
  }
  return ret;
}

lept_type lept_get_type(const lept_value *v) {
  assert(v != NULL);
  return v->type;
}

double lept_get_number(const lept_value *v) {
  assert(v != NULL && v->type == LEPT_NUMBER);
  return v->n;
}
