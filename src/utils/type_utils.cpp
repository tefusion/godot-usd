#include "utils/type_utils.h"
#include "godot_cpp/variant/variant.hpp"
#include "value-types.hh"
#include <cstdint>

godot::Variant single_value_to_variant(const tinyusdz::value::Value &usd_value, uint32_t type_id);
godot::Variant array_value_to_variant(const tinyusdz::value::Value &usd_value, uint32_t type_id);

godot::Variant to_variant(const tinyusdz::value::Value &usd_value) {
	godot::Variant result;

	uint32_t type_id = usd_value.type_id();
	if (type_id == tinyusdz::value::TYPE_ID_INVALID) {
		return result;
	}
	if (type_id & tinyusdz::value::TYPE_ID_1D_ARRAY_BIT) {
		return array_value_to_variant(usd_value, type_id & ~tinyusdz::value::TYPE_ID_1D_ARRAY_BIT);
	} else {
		return single_value_to_variant(usd_value, type_id);
	}
}

godot::Variant single_value_to_variant(const tinyusdz::value::Value &usd_value, uint32_t type_id) {
	godot::Variant result;
	switch (type_id) {
		// Boolean
		case tinyusdz::value::TYPE_ID_BOOL: {
			auto value = usd_value.get_value<bool>();
			if (value.has_value()) {
				result = godot::Variant(value.value());
			}
			break;
		}

		// Integer types
		case tinyusdz::value::TYPE_ID_INT32: {
			auto value = usd_value.get_value<int32_t>();
			if (value.has_value()) {
				result = godot::Variant(static_cast<int64_t>(value.value()));
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_INT64: {
			auto value = usd_value.get_value<int64_t>();
			if (value.has_value()) {
				result = godot::Variant(value.value());
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_UINT32: {
			auto value = usd_value.get_value<uint32_t>();
			if (value.has_value()) {
				result = godot::Variant(static_cast<int64_t>(value.value()));
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_UINT64: {
			auto value = usd_value.get_value<uint64_t>();
			if (value.has_value()) {
				result = godot::Variant(static_cast<int64_t>(value.value()));
			}
			break;
		}

		// Floating point types
		case tinyusdz::value::TYPE_ID_FLOAT: {
			auto value = usd_value.get_value<float>();
			if (value.has_value()) {
				result = godot::Variant(static_cast<double>(value.value()));
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_DOUBLE: {
			auto value = usd_value.get_value<double>();
			if (value.has_value()) {
				result = godot::Variant(value.value());
			}
			break;
		}

		// String types
		case tinyusdz::value::TYPE_ID_TOKEN: {
			auto value = usd_value.get_value<tinyusdz::value::token>();
			if (value.has_value()) {
				result = godot::Variant(godot::String(value.value().str().c_str()));
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_STRING: {
			auto value = usd_value.get_value<std::string>();
			if (value.has_value()) {
				result = godot::Variant(godot::String(value.value().c_str()));
			}
			break;
		}

		// Vector types
		case tinyusdz::value::TYPE_ID_FLOAT2: {
			auto value = usd_value.get_value<tinyusdz::value::float2>();
			if (value.has_value()) {
				const auto &v = value.value();
				result = godot::Variant(godot::Vector2(v[0], v[1]));
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_DOUBLE2: {
			auto value = usd_value.get_value<tinyusdz::value::double2>();
			if (value.has_value()) {
				const auto &v = value.value();
				result = godot::Variant(godot::Vector2(v[0], v[1]));
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_FLOAT3:
		case tinyusdz::value::TYPE_ID_VECTOR3F:
		case tinyusdz::value::TYPE_ID_POINT3F:
		case tinyusdz::value::TYPE_ID_NORMAL3F: {
			auto value = usd_value.get_value<tinyusdz::value::float3>();
			if (value.has_value()) {
				const auto &v = value.value();
				result = godot::Variant(godot::Vector3(v[0], v[1], v[2]));
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_DOUBLE3:
		case tinyusdz::value::TYPE_ID_VECTOR3D:
		case tinyusdz::value::TYPE_ID_POINT3D:
		case tinyusdz::value::TYPE_ID_NORMAL3D: {
			auto value = usd_value.get_value<tinyusdz::value::double3>();
			if (value.has_value()) {
				const auto &v = value.value();
				result = godot::Variant(godot::Vector3(v[0], v[1], v[2]));
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_FLOAT4: {
			auto value = usd_value.get_value<tinyusdz::value::float4>();
			if (value.has_value()) {
				const auto &v = value.value();
				result = godot::Variant(godot::Vector4(v[0], v[1], v[2], v[3]));
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_DOUBLE4: {
			auto value = usd_value.get_value<tinyusdz::value::double4>();
			if (value.has_value()) {
				const auto &v = value.value();
				result = godot::Variant(godot::Vector4(v[0], v[1], v[2], v[3]));
			}
			break;
		}

		// Color
		case tinyusdz::value::TYPE_ID_COLOR3F: {
			auto value = usd_value.get_value<tinyusdz::value::float3>();
			if (value.has_value()) {
				const auto &c = value.value();
				result = godot::Variant(godot::Color(c[0], c[1], c[2]));
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_COLOR3D: {
			auto value = usd_value.get_value<tinyusdz::value::double3>();
			if (value.has_value()) {
				const auto &c = value.value();
				result = godot::Variant(godot::Color(c[0], c[1], c[2]));
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_COLOR4F: {
			auto value = usd_value.get_value<tinyusdz::value::float4>();
			if (value.has_value()) {
				const auto &c = value.value();
				result = godot::Variant(godot::Color(c[0], c[1], c[2], c[3]));
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_COLOR4D: {
			auto value = usd_value.get_value<tinyusdz::value::double4>();
			if (value.has_value()) {
				const auto &c = value.value();
				result = godot::Variant(godot::Color(c[0], c[1], c[2], c[3]));
			}
			break;
		}

		// Quaternion
		case tinyusdz::value::TYPE_ID_QUATF: {
			auto value = usd_value.get_value<tinyusdz::value::quatf>();
			if (value.has_value()) {
				const auto &q = value.value();
				result = godot::Variant(godot::Quaternion(q[0], q[1], q[2], q[3]));
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_QUATD: {
			auto value = usd_value.get_value<tinyusdz::value::quatd>();
			if (value.has_value()) {
				const auto &q = value.value();
				result = godot::Variant(godot::Quaternion(q[0], q[1], q[2], q[3]));
			}
			break;
		}

		// Matrix types
		case tinyusdz::value::TYPE_ID_MATRIX4D: {
			auto value = usd_value.get_value<tinyusdz::value::matrix4d>();
			if (value.has_value()) {
				const auto &m = value.value();
				godot::Transform3D transform;

				// Set basis (3x3 rotation/scale part)
				for (int i = 0; i < 3; i++) {
					for (int j = 0; j < 3; j++) {
						transform.basis[i][j] = m.m[i][j];
					}
				}

				// Set origin (translation part)
				transform.origin = godot::Vector3(m.m[3][0], m.m[3][1], m.m[3][2]);

				result = godot::Variant(transform);
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_MATRIX4F: {
			auto value = usd_value.get_value<tinyusdz::value::matrix4f>();
			if (value.has_value()) {
				const auto &m = value.value();
				godot::Transform3D transform;

				// Set basis (3x3 rotation/scale part)
				for (int i = 0; i < 3; i++) {
					for (int j = 0; j < 3; j++) {
						transform.basis[i][j] = m.m[i][j];
					}
				}

				// Set origin (translation part)
				transform.origin = godot::Vector3(m.m[3][0], m.m[3][1], m.m[3][2]);

				result = godot::Variant(transform);
			}
			break;
		}

		case tinyusdz::value::TYPE_ID_TEXCOORD2F: {
			auto value = usd_value.get_value<tinyusdz::value::texcoord2f>();
			if (value.has_value()) {
				const auto &v = value.value();
				result = godot::Variant(godot::Vector2(v[0], v[1]));
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_TEXCOORD2D: {
			auto value = usd_value.get_value<tinyusdz::value::texcoord2d>();
			if (value.has_value()) {
				const auto &v = value.value();
				result = godot::Variant(godot::Vector2(v[0], v[1]));
			}
			break;
		}

		// Handle unsupported types
		default:
			result = godot::Variant(); // Return NIL for unsupported types
			break;
	}

	return result;
}

godot::Variant array_value_to_variant(const tinyusdz::value::Value &usd_value, uint32_t type_id) {
	godot::Variant result;

	switch (type_id) {
		// Boolean arrays
		case tinyusdz::value::TYPE_ID_BOOL: {
			auto value = usd_value.get_value<std::vector<bool>>();
			if (value.has_value()) {
				godot::Array array;
				for (const bool &v : value.value()) {
					array.push_back(godot::Variant(v));
				}
				result = array;
			}
			break;
		}

		// Integer array types
		case tinyusdz::value::TYPE_ID_INT32: {
			auto value = usd_value.get_value<std::vector<int32_t>>();
			if (value.has_value()) {
				godot::PackedInt32Array array;
				for (const auto &v : value.value()) {
					array.push_back(v);
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_INT64: {
			auto value = usd_value.get_value<std::vector<int64_t>>();
			if (value.has_value()) {
				godot::PackedInt64Array array;
				for (const auto &v : value.value()) {
					array.push_back(v);
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_UINT32: {
			auto value = usd_value.get_value<std::vector<uint32_t>>();
			if (value.has_value()) {
				godot::PackedInt64Array array;
				for (const auto &v : value.value()) {
					array.push_back(static_cast<int64_t>(v));
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_UINT64: {
			auto value = usd_value.get_value<std::vector<uint64_t>>();
			if (value.has_value()) {
				godot::PackedInt64Array array;
				for (const auto &v : value.value()) {
					array.push_back(static_cast<int64_t>(v));
				}
				result = array;
			}
			break;
		}

		// Floating point array types
		case tinyusdz::value::TYPE_ID_FLOAT: {
			auto value = usd_value.get_value<std::vector<float>>();
			if (value.has_value()) {
				godot::PackedFloat32Array array;
				for (const auto &v : value.value()) {
					array.push_back(v);
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_DOUBLE: {
			auto value = usd_value.get_value<std::vector<double>>();
			if (value.has_value()) {
				godot::Array array;
				for (const auto &v : value.value()) {
					array.push_back(godot::Variant(v));
				}
				result = array;
			}
			break;
		}

		// String array types
		case tinyusdz::value::TYPE_ID_TOKEN: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::token>>();
			if (value.has_value()) {
				godot::PackedStringArray array;
				for (const auto &v : value.value()) {
					array.push_back(godot::String(v.str().c_str()));
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_STRING: {
			auto value = usd_value.get_value<std::vector<std::string>>();
			if (value.has_value()) {
				godot::PackedStringArray array;
				for (const auto &v : value.value()) {
					array.push_back(godot::String(v.c_str()));
				}
				result = array;
			}
			break;
		}

		// Vector array types
		case tinyusdz::value::TYPE_ID_FLOAT2: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::float2>>();
			if (value.has_value()) {
				godot::PackedVector2Array array;
				for (const auto &v : value.value()) {
					array.push_back(godot::Vector2(v[0], v[1]));
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_DOUBLE2: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::double2>>();
			if (value.has_value()) {
				godot::PackedVector2Array array;
				for (const auto &v : value.value()) {
					array.push_back(godot::Vector2(v[0], v[1]));
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_FLOAT3:
		case tinyusdz::value::TYPE_ID_VECTOR3F:
		case tinyusdz::value::TYPE_ID_POINT3F:
		case tinyusdz::value::TYPE_ID_NORMAL3F: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::float3>>();
			if (value.has_value()) {
				godot::PackedVector3Array array;
				for (const auto &v : value.value()) {
					array.push_back(godot::Vector3(v[0], v[1], v[2]));
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_DOUBLE3:
		case tinyusdz::value::TYPE_ID_VECTOR3D:
		case tinyusdz::value::TYPE_ID_POINT3D:
		case tinyusdz::value::TYPE_ID_NORMAL3D: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::double3>>();
			if (value.has_value()) {
				godot::PackedVector3Array array;
				for (const auto &v : value.value()) {
					array.push_back(godot::Vector3(v[0], v[1], v[2]));
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_FLOAT4: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::float4>>();
			if (value.has_value()) {
				godot::Array array;
				for (const auto &v : value.value()) {
					array.push_back(godot::Variant(godot::Vector4(v[0], v[1], v[2], v[3])));
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_DOUBLE4: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::double4>>();
			if (value.has_value()) {
				godot::Array array;
				for (const auto &v : value.value()) {
					array.push_back(godot::Variant(godot::Vector4(v[0], v[1], v[2], v[3])));
				}
				result = array;
			}
			break;
		}

		// Color array types
		case tinyusdz::value::TYPE_ID_COLOR3F: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::float3>>();
			if (value.has_value()) {
				godot::PackedColorArray array;
				for (const auto &c : value.value()) {
					array.push_back(godot::Color(c[0], c[1], c[2]));
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_COLOR3D: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::double3>>();
			if (value.has_value()) {
				godot::PackedColorArray array;
				for (const auto &c : value.value()) {
					array.push_back(godot::Color(c[0], c[1], c[2]));
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_COLOR4F: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::float4>>();
			if (value.has_value()) {
				godot::PackedColorArray array;
				for (const auto &c : value.value()) {
					array.push_back(godot::Color(c[0], c[1], c[2], c[3]));
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_COLOR4D: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::double4>>();
			if (value.has_value()) {
				godot::PackedColorArray array;
				for (const auto &c : value.value()) {
					array.push_back(godot::Color(c[0], c[1], c[2], c[3]));
				}
				result = array;
			}
			break;
		}

		// Quaternion array types
		case tinyusdz::value::TYPE_ID_QUATF: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::quatf>>();
			if (value.has_value()) {
				godot::Array array;
				for (const auto &q : value.value()) {
					array.push_back(godot::Variant(godot::Quaternion(q[0], q[1], q[2], q[3])));
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_QUATD: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::quatd>>();
			if (value.has_value()) {
				godot::Array array;
				for (const auto &q : value.value()) {
					array.push_back(godot::Variant(godot::Quaternion(q[0], q[1], q[2], q[3])));
				}
				result = array;
			}
			break;
		}

		// Matrix array types
		case tinyusdz::value::TYPE_ID_MATRIX4D: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::matrix4d>>();
			if (value.has_value()) {
				godot::Array array;
				for (const auto &m : value.value()) {
					godot::Transform3D transform;

					// Set basis (3x3 rotation/scale part)
					for (int i = 0; i < 3; i++) {
						for (int j = 0; j < 3; j++) {
							transform.basis[i][j] = m.m[i][j];
						}
					}

					// Set origin (translation part)
					transform.origin = godot::Vector3(m.m[3][0], m.m[3][1], m.m[3][2]);

					array.push_back(godot::Variant(transform));
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_MATRIX4F: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::matrix4f>>();
			if (value.has_value()) {
				godot::Array array;
				for (const auto &m : value.value()) {
					godot::Transform3D transform;

					// Set basis (3x3 rotation/scale part)
					for (int i = 0; i < 3; i++) {
						for (int j = 0; j < 3; j++) {
							transform.basis[i][j] = m.m[i][j];
						}
					}

					// Set origin (translation part)
					transform.origin = godot::Vector3(m.m[3][0], m.m[3][1], m.m[3][2]);

					array.push_back(godot::Variant(transform));
				}
				result = array;
			}
			break;
		}

		// Texture coordinate array types
		case tinyusdz::value::TYPE_ID_TEXCOORD2F: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::texcoord2f>>();
			if (value.has_value()) {
				godot::PackedVector2Array array;
				for (const auto &v : value.value()) {
					array.push_back(godot::Vector2(v[0], v[1]));
				}
				result = array;
			}
			break;
		}
		case tinyusdz::value::TYPE_ID_TEXCOORD2D: {
			auto value = usd_value.get_value<std::vector<tinyusdz::value::texcoord2d>>();
			if (value.has_value()) {
				godot::PackedVector2Array array;
				for (const auto &v : value.value()) {
					array.push_back(godot::Vector2(v[0], v[1]));
				}
				result = array;
			}
			break;
		}

		// Handle unsupported types
		default:
			result = godot::Variant(); // Return NIL for unsupported types
			break;
	}

	return result;
}
