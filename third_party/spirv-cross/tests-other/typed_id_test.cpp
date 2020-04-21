#include "spirv_common.hpp"

using namespace SPIRV_CROSS_NAMESPACE;

int main()
{
	// Construct from uint32_t.
	VariableID var_id = 10;
	TypeID type_id = 20;
	ConstantID constant_id = 30;

	// Assign from uint32_t.
	var_id = 100;
	type_id = 40;
	constant_id = 60;

	// Construct generic ID.
	ID generic_var_id = var_id;
	ID generic_type_id = type_id;
	ID generic_constant_id = constant_id;

	// Assign generic id.
	generic_var_id = var_id;
	generic_type_id = type_id;
	generic_constant_id = constant_id;

	// Assign generic ID to typed ID
	var_id = generic_var_id;
	type_id = generic_type_id;
	constant_id = generic_constant_id;

	// Implicit conversion to uint32_t.
	uint32_t a;
	a = var_id;
	a = type_id;
	a = constant_id;
	a = generic_var_id;
	a = generic_type_id;
	a = generic_constant_id;

	// Copy assignment.
	var_id = VariableID(10);
	type_id = TypeID(10);
	constant_id = ConstantID(10);

	// These operations are blocked, assign or construction from mismatched types.
	//var_id = type_id;
	//var_id = TypeID(100);
}