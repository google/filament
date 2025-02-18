SKIP: FAILED

static int x_GLF_global_loop_count = 0;
cbuffer cbuffer_x_7 : register(b0) {
  uint4 x_7[3];
};
cbuffer cbuffer_x_10 : register(b1) {
  uint4 x_10[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float f = 0.0f;
  int i = 0;
  int i_1 = 0;
  int i_2 = 0;
  int i_3 = 0;
  int i_4 = 0;
  int i_5 = 0;
  int i_6 = 0;
  int i_7 = 0;
  int i_8 = 0;
  int i_9 = 0;
  int i_10 = 0;
  int i_11 = 0;
  int i_12 = 0;
  int i_13 = 0;
  int i_14 = 0;
  float sum = 0.0f;
  int r = 0;
  x_GLF_global_loop_count = 0;
  f = asfloat(x_7[1].x);
  i = asint(x_10[1].x);
  while (true) {
    if ((i < asint(x_10[0].x))) {
    } else {
      break;
    }
    i_1 = asint(x_10[1].x);
    while (true) {
      if ((i_1 < asint(x_10[0].x))) {
      } else {
        break;
      }
      i_2 = asint(x_10[1].x);
      while (true) {
        if ((i_2 < asint(x_10[0].x))) {
        } else {
          break;
        }
        i_3 = asint(x_10[1].x);
        while (true) {
          if ((i_3 < asint(x_10[0].x))) {
          } else {
            break;
          }
          i_4 = asint(x_10[1].x);
          while (true) {
            if ((i_4 < asint(x_10[0].x))) {
            } else {
              break;
            }
            i_5 = asint(x_10[1].x);
            while (true) {
              if ((i_5 < asint(x_10[0].x))) {
              } else {
                break;
              }
              i_6 = asint(x_10[1].x);
              while (true) {
                if ((i_6 < asint(x_10[0].x))) {
                } else {
                  break;
                }
                i_7 = asint(x_10[1].x);
                while (true) {
                  if ((i_7 < asint(x_10[0].x))) {
                  } else {
                    break;
                  }
                  i_8 = asint(x_10[1].x);
                  while (true) {
                    if ((i_8 < asint(x_10[0].x))) {
                    } else {
                      break;
                    }
                    i_9 = asint(x_10[1].x);
                    while (true) {
                      if ((i_9 < asint(x_10[0].x))) {
                      } else {
                        break;
                      }
                      i_10 = asint(x_10[1].x);
                      while (true) {
                        if ((i_10 < asint(x_10[0].x))) {
                        } else {
                          break;
                        }
                        i_11 = asint(x_10[1].x);
                        while (true) {
                          if ((i_11 < asint(x_10[2].x))) {
                          } else {
                            break;
                          }
                          i_12 = asint(x_10[1].x);
                          while (true) {
                            if ((i_12 < asint(x_10[0].x))) {
                            } else {
                              break;
                            }
                            i_13 = asint(x_10[1].x);
                            while (true) {
                              if ((i_13 < asint(x_10[0].x))) {
                              } else {
                                break;
                              }
                              i_14 = asint(x_10[1].x);
                              while (true) {
                                if ((i_14 < asint(x_10[2].x))) {
                                } else {
                                  break;
                                }
                                while (true) {
                                  x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
                                  {
                                    int x_225 = x_GLF_global_loop_count;
                                    int x_227 = asint(x_10[3].x);
                                    if (!((x_225 < (100 - x_227)))) { break; }
                                  }
                                }
                                f = (f + asfloat(x_7[0].x));
                                {
                                  i_14 = (i_14 + 1);
                                }
                              }
                              {
                                i_13 = (i_13 + 1);
                              }
                            }
                            {
                              i_12 = (i_12 + 1);
                            }
                          }
                          {
                            i_11 = (i_11 + 1);
                          }
                        }
                        {
                          i_10 = (i_10 + 1);
                        }
                      }
                      {
                        i_9 = (i_9 + 1);
                      }
                    }
                    {
                      i_8 = (i_8 + 1);
                    }
                  }
                  {
                    i_7 = (i_7 + 1);
                  }
                }
                {
                  i_6 = (i_6 + 1);
                }
              }
              {
                i_5 = (i_5 + 1);
              }
            }
            {
              i_4 = (i_4 + 1);
            }
          }
          {
            i_3 = (i_3 + 1);
          }
        }
        {
          i_2 = (i_2 + 1);
        }
      }
      {
        i_1 = (i_1 + 1);
      }
    }
    {
      i = (i + 1);
    }
  }
  sum = asfloat(x_7[1].x);
  r = asint(x_10[1].x);
  while (true) {
    if ((x_GLF_global_loop_count < 100)) {
    } else {
      break;
    }
    x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
    sum = (sum + f);
    {
      r = (r + 1);
    }
  }
  if ((sum == asfloat(x_7[2].x))) {
    x_GLF_color = float4(float(asint(x_10[0].x)), float(asint(x_10[1].x)), float(asint(x_10[1].x)), float(asint(x_10[0].x)));
  } else {
    x_GLF_color = float4((float(asint(x_10[1].x))).xxxx);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
