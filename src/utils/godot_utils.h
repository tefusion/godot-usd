#pragma once

#include "godot_cpp/variant/packed_string_array.hpp"
#include <godot_cpp/classes/ref_counted.hpp>

template <typename T>
godot::TypedArray<T> ref_vector_to_typed_array(const godot::Vector<godot::Ref<T>> &vector) {
	godot::TypedArray<T> array;
	array.resize(vector.size());

	for (int i = 0; i < vector.size(); i++) {
		array[i] = vector[i];
	}

	return array;
}

godot::PackedStringArray string_vector_to_typed_array(const godot::Vector<godot::String> &vector);
