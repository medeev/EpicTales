////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	UtilExporter 클래스의 헤더 파일
///
/// @date	2024-3-27
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Collision/Collider.h"

class PktShapeNotify;
class CollisionUnit;
class PktColor;
namespace UtilExporter
{
	void ExportTo(
		Core::Collision::ColliderPtr collider, 
		PktShapeNotify& notify,
		const PktColor& color,
		float duration = 2.f);

	void ExportTo(const CollisionUnit* collisionUnit,
		PktShapeNotify& notify,
		const PktColor& color,
		float z,
		float duration = 2.f);

}
