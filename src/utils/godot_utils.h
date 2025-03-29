#pragma once

#include "godot_cpp/variant/packed_string_array.hpp"
#include <godot_cpp/classes/ref_counted.hpp>

//can't expose Vector<T> to godot, so this contains conversion utils
template <typename T>
godot::TypedArray<T> ref_vector_to_typed_array(const godot::Vector<godot::Ref<T>> &vector) {
	godot::TypedArray<T> array;
	array.resize(vector.size());

	for (int i = 0; i < vector.size(); i++) {
		array[i] = vector[i];
	}

	return array;
}

template <typename T>
godot::Vector<godot::Ref<T>> typed_array_to_ref_vector(const godot::TypedArray<T> &array) {
	godot::Vector<godot::Ref<T>> vector;
	vector.resize(array.size());

	for (int i = 0; i < array.size(); i++) {
		vector[i] = array[i];
	}

	return vector;
}

template <typename T>
godot::Vector<T> typed_array_to_vector(const godot::TypedArray<T> &array) {
	godot::Vector<T> vector;
	vector.resize(array.size());

	for (int i = 0; i < array.size(); i++) {
		vector[i] = array[i];
	}

	return vector;
}

template <typename T>
godot::TypedArray<T> vector_to_typed_array(const godot::Vector<T> &vector) {
	godot::TypedArray<T> array;
	array.resize(vector.size());

	for (int i = 0; i < vector.size(); i++) {
		array[i] = vector[i];
	}

	return array;
}

godot::PackedStringArray string_vector_to_typed_array(const godot::Vector<godot::String> &vector);
