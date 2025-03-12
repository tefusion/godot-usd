#include "type_utils.h"

godot::Variant to_variant(const tinyusdz::value::Value &usd_value) {
	godot::Variant result;

	switch (usd_value.type_id()) {
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

		// Handle unsupported types
		default:
			result = godot::Variant(); // Return NIL for unsupported types
			break;
	}

	return result;
}
