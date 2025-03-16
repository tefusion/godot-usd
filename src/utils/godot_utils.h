#pragma once

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

godot::TypedArray<godot::String> string_vector_to_typed_array(const godot::Vector<godot::String> &vector) {
	godot::TypedArray<godot::String> array;
	array.resize(vector.size());

	for (int i = 0; i < vector.size(); i++) {
		array[i] = vector[i];
	}

	return array;
}
