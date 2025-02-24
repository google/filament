import {
  abstractFloatShaderBuilder,
  abstractIntShaderBuilder,
  basicExpressionBuilder,
  ShaderBuilder,
} from '../expression.js';

/* @returns a ShaderBuilder that evaluates a prefix unary operation */
export function unary(op: string): ShaderBuilder {
  return basicExpressionBuilder(value => `${op}(${value})`);
}

/* @returns a ShaderBuilder that evaluates a prefix unary operation that returns AbstractFloats */
export function abstractFloatUnary(op: string): ShaderBuilder {
  return abstractFloatShaderBuilder(value => `${op}(${value})`);
}

/* @returns a ShaderBuilder that evaluates a prefix unary operation that returns AbstractInts */
export function abstractIntUnary(op: string): ShaderBuilder {
  return abstractIntShaderBuilder(value => `${op}(${value})`);
}
