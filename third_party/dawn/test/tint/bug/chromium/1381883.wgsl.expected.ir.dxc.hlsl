struct main_inputs {
  float none : TEXCOORD2;
};


void main_inner(float none) {
}

void main(main_inputs inputs) {
  main_inner(inputs.none);
}

