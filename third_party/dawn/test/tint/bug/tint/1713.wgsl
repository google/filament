const a = 1 << 30u; // First shift, result should be abstract
const b = a << 10u; // Valid only if 'a' is abstract

const c = 5000000000 << 3u; // Valid only if result is abtract
