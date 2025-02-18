SKIP: TIMEOUT

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


static int x_GLF_global_loop_count = int(0);
cbuffer cbuffer_x_7 : register(b0) {
  uint4 x_7[3];
};
cbuffer cbuffer_x_10 : register(b1) {
  uint4 x_10[4];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  float f = 0.0f;
  int i = int(0);
  int i_1 = int(0);
  int i_2 = int(0);
  int i_3 = int(0);
  int i_4 = int(0);
  int i_5 = int(0);
  int i_6 = int(0);
  int i_7 = int(0);
  int i_8 = int(0);
  int i_9 = int(0);
  int i_10 = int(0);
  int i_11 = int(0);
  int i_12 = int(0);
  int i_13 = int(0);
  int i_14 = int(0);
  float sum = 0.0f;
  int r = int(0);
  x_GLF_global_loop_count = int(0);
  f = asfloat(x_7[1u].x);
  i = asint(x_10[1u].x);
  {
    while(true) {
      int v = i;
      if ((v < asint(x_10[0u].x))) {
      } else {
        break;
      }
      i_1 = asint(x_10[1u].x);
      {
        while(true) {
          int v_1 = i_1;
          if ((v_1 < asint(x_10[0u].x))) {
          } else {
            break;
          }
          i_2 = asint(x_10[1u].x);
          {
            while(true) {
              int v_2 = i_2;
              if ((v_2 < asint(x_10[0u].x))) {
              } else {
                break;
              }
              i_3 = asint(x_10[1u].x);
              {
                while(true) {
                  int v_3 = i_3;
                  if ((v_3 < asint(x_10[0u].x))) {
                  } else {
                    break;
                  }
                  i_4 = asint(x_10[1u].x);
                  {
                    while(true) {
                      int v_4 = i_4;
                      if ((v_4 < asint(x_10[0u].x))) {
                      } else {
                        break;
                      }
                      i_5 = asint(x_10[1u].x);
                      {
                        while(true) {
                          int v_5 = i_5;
                          if ((v_5 < asint(x_10[0u].x))) {
                          } else {
                            break;
                          }
                          i_6 = asint(x_10[1u].x);
                          {
                            while(true) {
                              int v_6 = i_6;
                              if ((v_6 < asint(x_10[0u].x))) {
                              } else {
                                break;
                              }
                              i_7 = asint(x_10[1u].x);
                              {
                                while(true) {
                                  int v_7 = i_7;
                                  if ((v_7 < asint(x_10[0u].x))) {
                                  } else {
                                    break;
                                  }
                                  i_8 = asint(x_10[1u].x);
                                  {
                                    while(true) {
                                      int v_8 = i_8;
                                      if ((v_8 < asint(x_10[0u].x))) {
                                      } else {
                                        break;
                                      }
                                      i_9 = asint(x_10[1u].x);
                                      {
                                        while(true) {
                                          int v_9 = i_9;
                                          if ((v_9 < asint(x_10[0u].x))) {
                                          } else {
                                            break;
                                          }
                                          i_10 = asint(x_10[1u].x);
                                          {
                                            while(true) {
                                              int v_10 = i_10;
                                              if ((v_10 < asint(x_10[0u].x))) {
                                              } else {
                                                break;
                                              }
                                              i_11 = asint(x_10[1u].x);
                                              {
                                                while(true) {
                                                  int v_11 = i_11;
                                                  if ((v_11 < asint(x_10[2u].x))) {
                                                  } else {
                                                    break;
                                                  }
                                                  i_12 = asint(x_10[1u].x);
                                                  {
                                                    while(true) {
                                                      int v_12 = i_12;
                                                      if ((v_12 < asint(x_10[0u].x))) {
                                                      } else {
                                                        break;
                                                      }
                                                      i_13 = asint(x_10[1u].x);
                                                      {
                                                        while(true) {
                                                          int v_13 = i_13;
                                                          if ((v_13 < asint(x_10[0u].x))) {
                                                          } else {
                                                            break;
                                                          }
                                                          i_14 = asint(x_10[1u].x);
                                                          {
                                                            while(true) {
                                                              int v_14 = i_14;
                                                              if ((v_14 < asint(x_10[2u].x))) {
                                                              } else {
                                                                break;
                                                              }
                                                              {
                                                                while(true) {
                                                                  x_GLF_global_loop_count = (x_GLF_global_loop_count + int(1));
                                                                  {
                                                                    int x_225 = x_GLF_global_loop_count;
                                                                    int x_227 = asint(x_10[3u].x);
                                                                    if (!((x_225 < (int(100) - x_227)))) { break; }
                                                                  }
                                                                  continue;
                                                                }
                                                              }
                                                              float v_15 = f;
                                                              f = (v_15 + asfloat(x_7[0u].x));
                                                              {
                                                                i_14 = (i_14 + int(1));
                                                              }
                                                              continue;
                                                            }
                                                          }
                                                          {
                                                            i_13 = (i_13 + int(1));
                                                          }
                                                          continue;
                                                        }
                                                      }
                                                      {
                                                        i_12 = (i_12 + int(1));
                                                      }
                                                      continue;
                                                    }
                                                  }
                                                  {
                                                    i_11 = (i_11 + int(1));
                                                  }
                                                  continue;
                                                }
                                              }
                                              {
                                                i_10 = (i_10 + int(1));
                                              }
                                              continue;
                                            }
                                          }
                                          {
                                            i_9 = (i_9 + int(1));
                                          }
                                          continue;
                                        }
                                      }
                                      {
                                        i_8 = (i_8 + int(1));
                                      }
                                      continue;
                                    }
                                  }
                                  {
                                    i_7 = (i_7 + int(1));
                                  }
                                  continue;
                                }
                              }
                              {
                                i_6 = (i_6 + int(1));
                              }
                              continue;
                            }
                          }
                          {
                            i_5 = (i_5 + int(1));
                          }
                          continue;
                        }
                      }
                      {
                        i_4 = (i_4 + int(1));
                      }
                      continue;
                    }
                  }
                  {
                    i_3 = (i_3 + int(1));
                  }
                  continue;
                }
              }
              {
                i_2 = (i_2 + int(1));
              }
              continue;
            }
          }
          {
            i_1 = (i_1 + int(1));
          }
          continue;
        }
      }
      {
        i = (i + int(1));
      }
      continue;
    }
  }
  sum = asfloat(x_7[1u].x);
  r = asint(x_10[1u].x);
  {
    while(true) {
      if ((x_GLF_global_loop_count < int(100))) {
      } else {
        break;
      }
      x_GLF_global_loop_count = (x_GLF_global_loop_count + int(1));
      sum = (sum + f);
      {
        r = (r + int(1));
      }
      continue;
    }
  }
  float v_16 = sum;
  if ((v_16 == asfloat(x_7[2u].x))) {
    float v_17 = float(asint(x_10[0u].x));
    float v_18 = float(asint(x_10[1u].x));
    float v_19 = float(asint(x_10[1u].x));
    x_GLF_color = float4(v_17, v_18, v_19, float(asint(x_10[0u].x)));
  } else {
    x_GLF_color = float4((float(asint(x_10[1u].x))).xxxx);
  }
}

main_out main_inner() {
  main_1();
  main_out v_20 = {x_GLF_color};
  return v_20;
}

main_outputs main() {
  main_out v_21 = main_inner();
  main_outputs v_22 = {v_21.x_GLF_color_1};
  return v_22;
}

