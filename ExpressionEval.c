#include "ExpresionEval.h"

// Custom function that returns the sum of its two arguments
static float add(struct expr_func *f, vec_expr_t *args, void *c) {
  float a = expr_eval(&vec_nth(args, 0));
  float b = expr_eval(&vec_nth(args, 1));
  return a + b;
}

static struct expr_func user_funcs[] = {
    {"add", add, NULL, 0},
    {NULL, NULL, NULL, 0},
};

int main() {
  const char *s = "x = 5, add(2, x)";
  struct expr_var_list vars = {0};
  struct expr *e = expr_create(s, strlen(s), &vars, user_funcs);
  if (e == NULL) {
    printf("Syntax error");
    return 1;
  }

  float result = expr_eval(e);
  printf("result: %f\n", result);

  expr_destroy(e, &vars);
  return 0;
}