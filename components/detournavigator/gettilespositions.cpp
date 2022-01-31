#include "gettilespositions.hpp"
#include "settings.hpp"
#include "settingsutils.hpp"
#include "tileposition.hpp"
#include "tilebounds.hpp"

#include <components/misc/convert.hpp>

#include <BulletCollision/CollisionShapes/btCollisionShape.h>

namespace DetourNavigator
{
    TilesPositionsRange makeTilesPositionsRange(const osg::Vec2f& aabbMin, const osg::Vec2f& aabbMax,
        const RecastSettings& settings)
    {
        osg::Vec2f min = toNavMeshCoordinates(settings, aabbMin);
        osg::Vec2f max = toNavMeshCoordinates(settings, aabbMax);

        const float border = getBorderSize(settings);
        min -= osg::Vec2f(border, border);
        max += osg::Vec2f(border, border);

        TilePosition minTile = getTilePosition(settings, min);
        TilePosition maxTile = getTilePosition(settings, max);

        if (minTile.x() > maxTile.x())
            std::swap(minTile.x(), maxTile.x());

        if (minTile.y() > maxTile.y())
            std::swap(minTile.y(), maxTile.y());

        return {minTile, maxTile};
    }

    TilesPositionsRange makeTilesPositionsRange(const btCollisionShape& shape, const btTransform& transform,
        const RecastSettings& settings)
    {
        const TileBounds bounds = makeObjectTileBounds(shape, transform);
        return makeTilesPositionsRange(bounds.mMin, bounds.mMax, settings);
    }

    TilesPositionsRange makeTilesPositionsRange(const int cellSize, const btVector3& shift,
        const RecastSettings& settings)
    {
        const int halfCellSize = cellSize / 2;
        const btTransform transform(btMatrix3x3::getIdentity(), shift);
        btVector3 aabbMin = transform(btVector3(-halfCellSize, -halfCellSize, 0));
        btVector3 aabbMax = transform(btVector3(halfCellSize, halfCellSize, 0));

        aabbMin.setX(std::min(aabbMin.x(), aabbMax.x()));
        aabbMin.setY(std::min(aabbMin.y(), aabbMax.y()));

        aabbMax.setX(std::max(aabbMin.x(), aabbMax.x()));
        aabbMax.setY(std::max(aabbMin.y(), aabbMax.y()));

        return makeTilesPositionsRange(Misc::Convert::toOsgXY(aabbMin), Misc::Convert::toOsgXY(aabbMax), settings);
    }
}
