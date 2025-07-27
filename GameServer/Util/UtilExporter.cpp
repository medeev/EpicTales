#include "UtilExporter.h"

#include <Core/Collision/Collider.h>
#include <Data/Collision/Unit/CircleUnit.h>
#include <Data/Collision/Unit/DonutUnit.h>
#include <Data/Collision/Unit/FanUnit.h>
#include <Data/Collision/Unit/ObbUnit.h>
#include <Protocol/Struct/PktShape.h>

namespace UtilExporter
{

	void ExportTo(Core::Collision::ColliderPtr collider, PktShapeNotify& notify, const PktColor& color, float duration)
	{
		if (!collider)
			return;

		if (Core::Collision::BoxPtr box = std::dynamic_pointer_cast<Core::Collision::Box> (collider))
		{
			PktBoxShape pkt;
			pkt.setLocation(Vector(box->pos.x, box->pos.y, box->pos.z));
			pkt.setRotator(PktRotator(box->rotator.getRoll(), box->rotator.getPitch(), box->rotator.getYaw()));
			pkt.setBoxExtent(Vector(box->max.x, box->max.y, box->max.z));
			pkt.setDuration(duration);
			pkt.setColor(color);
			notify.getBoxList().emplace_back(std::move(pkt));
		}

		if (Core::Collision::SpherePtr sphere = std::dynamic_pointer_cast<Core::Collision::Sphere> (collider))
		{
			PktSphereShape pkt;
			pkt.setLocation(Vector(sphere->pos.x, sphere->pos.y, sphere->pos.z));
			pkt.setRadius(sphere->r);
			pkt.setDuration(duration);
			pkt.setThickness(2.f);
			pkt.setColor(color);
			notify.getSphereList().emplace_back(std::move(pkt));
		}

		if (Core::Collision::CapsulePtr capsule = std::dynamic_pointer_cast<Core::Collision::Capsule> (collider))
		{
			PktCapsuleShape pkt;
			pkt.setLocation(Vector(capsule->pos.x, capsule->pos.y, capsule->pos.z));
			pkt.setRadius(capsule->r);
			pkt.setHalfHeight(capsule->z_cap + capsule->r);
			pkt.setDuration(duration);
			pkt.setThickness(2.f);
			pkt.setColor(color);
			notify.getCapsuleList().emplace_back(std::move(pkt));
		}

		if (Core::Collision::CylinderPtr cylinder = std::dynamic_pointer_cast<Core::Collision::Cylinder> (collider))
		{
			Vector pos(cylinder->pos.x, cylinder->pos.y, cylinder->pos.z);
			Vector start = pos + Vector(0, 0, cylinder->z_cap);
			Vector end = pos + Vector(0, 0, cylinder->z_base);
			PktCylinderShape pkt;
			pkt.setRadius(cylinder->r);
			pkt.setStart(start);
			pkt.setEnd(end);
			pkt.setDuration(duration);
			pkt.setThickness(2.f);
			pkt.setColor(color);
			notify.getCylinderList().emplace_back(std::move(pkt));
		}

		if (Core::Collision::ArcPtr arc = std::dynamic_pointer_cast<Core::Collision::Arc> (collider))
		{
			Vector pos(arc->pos.x, arc->pos.y, arc->pos.z);
			Vector start = pos + Vector(0, 0, arc->z_cap);
			Vector end = pos + Vector(0, 0, arc->z_base);
			PktCylinderShape pkt;
			pkt.setRadius(arc->r);
			pkt.setStart(start);
			pkt.setEnd(end);
			pkt.setDuration(duration);
			pkt.setThickness(2.f);
			pkt.setColor(color);
			notify.getCylinderList().emplace_back(std::move(pkt));
		}

		//if ( Core::Collision::RayPtr ray = std::dynamic_pointer_cast< Core::Collision::Ray > ( collider ) )
		//{
		//	Vector start( ray->getStart() );
		//	Vector end = start + ray->getdir();
		//	PktDebugLine pkt;
		//	pkt.setStart( start.exportTo() );
		//	pkt.setEnd( end.exportTo() );
		//	pkt.setDuration( duration );
		//	pkt.setThickness( 2.f );
		//	pkt.setColor( color );
		//	notify.getLineList().emplace_back( std::move( pkt ) );
		//}
	}

	void ExportTo(const CollisionUnit* collisionUnit, PktShapeNotify& notify, const PktColor& color, float z, float duration)
	{
		if (!collisionUnit)
			return;

		if (auto unit = dynamic_cast<const CircleUnit*>(collisionUnit))
		{
			PktCircleShape pkt;
			pkt.setLocation(Core::Vector3(unit->getPos().x, unit->getPos().y, z));
			pkt.setRadius(unit->info.radius);
			pkt.setDuration(duration);
			pkt.setColor(color);
			pkt.setThickness(2);
			notify.getCircles().emplace_back(std::move(pkt));
		}
		if (auto unit = dynamic_cast<const DonutUnit*>(collisionUnit))
		{
			PktDonutShape pkt;
			pkt.setLocation(Core::Vector3(unit->getPos().x, unit->getPos().y, z));
			pkt.setMaxRadius(unit->info.maxRadius);
			pkt.setMinRadius(unit->info.minRadius);
			pkt.setDuration(duration);
			pkt.setColor(color);
			pkt.setThickness(2);
			notify.getDonuts().emplace_back(std::move(pkt));
		}
		if (auto unit = dynamic_cast<const FanUnit*>(collisionUnit))
		{
			auto rot = Core::Vector3(unit->getDir().x, unit->getDir().y, 0.f).toOrientationRotator();
			PktFanShape pkt;
			pkt.setLocation(Core::Vector3(unit->getPos().x, unit->getPos().y, z));
			pkt.setRotator(PktRotator(rot.pitch, rot.yaw, rot.roll));
			pkt.setRadius(unit->info.radius);
			pkt.setCentralAngle(unit->info.centralAngle);
			pkt.setDuration(duration);
			pkt.setColor(color);
			pkt.setThickness(2);
			notify.getFans().emplace_back(std::move(pkt));
		}
		if (auto unit = dynamic_cast<const ObbUnit*>(collisionUnit))
		{
			auto rot = Core::Vector3(unit->getDir().x, unit->getDir().y, 0.f).toOrientationRotator();

			PktObbShape pkt;
			pkt.setLocation(Core::Vector3(unit->getPos().x, unit->getPos().y, z));
			pkt.setRotator(PktRotator(rot.pitch, rot.yaw, rot.roll));
			pkt.setFrontSize(unit->info.frontSize);
			pkt.setBackSize(unit->info.backSize);
			pkt.setLeftSize(unit->info.leftSize);
			pkt.setRightSize(unit->info.rightSize);
			pkt.setDuration(duration);
			pkt.setColor(color);
			pkt.setThickness(2);
			notify.getObbs().emplace_back(std::move(pkt));
		}
	}

}
