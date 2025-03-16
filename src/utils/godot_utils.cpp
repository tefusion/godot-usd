#include "godot_utils.h"
#include "godot_cpp/variant/packed_string_array.hpp"

godot::PackedStringArray string_vector_to_typed_array(const godot::Vector<godot::String> &vector) {
	godot::PackedStringArray array;
	array.resize(vector.size());

	for (int i = 0; i < vector.size(); i++) {
		array[i] = vector[i];
	}

	return array;
}
