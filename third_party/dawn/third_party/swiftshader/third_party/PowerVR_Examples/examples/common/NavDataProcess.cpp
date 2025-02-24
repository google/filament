#include "NavDataProcess.h"
glm::dvec3 NavDataProcess::findIntersect(glm::dvec2 minBounds, glm::dvec2 maxBounds, glm::dvec2 inPoint, glm::dvec2 outPoint) const
{
	double m = (inPoint.y - outPoint.y) / (inPoint.x - outPoint.x);
	double c = inPoint.y - m * inPoint.x;

	if (outPoint.x < minBounds.x) // Check if intersect is with left side
	{
		double y = m * minBounds.x + c;

		if ((y >= minBounds.y) && (y <= maxBounds.y)) { return glm::dvec3(minBounds.x, y, Sides::Left); }
	}

	if (outPoint.y > maxBounds.y) // Check if intersect is with top side
	{
		if (outPoint.x == inPoint.x) { return glm::dvec3(outPoint.x, maxBounds.y, Sides::Top); }

		double x = (maxBounds.y - c) / m;

		if ((x >= minBounds.x) && (x <= maxBounds.x)) { return glm::dvec3(x, maxBounds.y, Sides::Top); }
	}

	if (outPoint.x > maxBounds.x) // Check if intersect is with right sight
	{
		double y = m * maxBounds.x + c;

		if ((y >= minBounds.y) && (y <= maxBounds.y)) { return glm::dvec3(maxBounds.x, y, Sides::Right); }
	}

	if (outPoint.y < minBounds.y) // Check if intersect is with bottom side
	{
		if (outPoint.x == inPoint.x) { return glm::dvec3(outPoint.x, minBounds.y, Sides::Bottom); }

		double x = (minBounds.y - c) / m;

		if ((x >= minBounds.x) && (x <= maxBounds.x)) { return glm::dvec3(x, minBounds.y, Sides::Bottom); }
	}

	Log(LogLevel::Error, "Could not find intersect point, empty vector returned");

	return glm::dvec3(0, 0, Sides::NoSide);
}

pvr::PolygonWindingOrder NavDataProcess::checkWinding(const std::vector<uint64_t>& nodeIds) const
{
	std::vector<glm::dvec2> points;

	for (uint32_t i = 0; i < nodeIds.size(); ++i) { points.emplace_back(_osm.getNodeById(nodeIds[i]).coords); }

	double area = calculateTriangleArea(points);

	if (area <= 0.0) { return pvr::PolygonWindingOrder::FrontFaceCCW; }
	else
	{
		return pvr::PolygonWindingOrder::FrontFaceCW;
	}
}

std::array<glm::dvec2, 2> NavDataProcess::findPerpendicularPoints(glm::dvec2 firstPoint, glm::dvec2 secPoint, double width, int pointNum) const
{
	std::array<glm::dvec2, 2> points;

	if (glm::abs(firstPoint.y - secPoint.y) <= epsilon) // To avoid division by zero
	{
		points[0] = (pointNum == 1) ? glm::dvec2(firstPoint.x, firstPoint.y + width / 2.0) : glm::dvec2(secPoint.x, secPoint.y + width / 2.0);
		points[1] = (pointNum == 1) ? glm::dvec2(firstPoint.x, firstPoint.y - width / 2.0) : glm::dvec2(secPoint.x, secPoint.y - width / 2.0);
	}
	else // All other cases give a valid gradient
	{
		double m = -(secPoint.x - firstPoint.x) / (secPoint.y - firstPoint.y);
		double c = (pointNum == 1) ? firstPoint.y - m * firstPoint.x : secPoint.y - m * secPoint.x;

		// You can use the new gradient and constant to find intersections with a circle with r = width / 2
		points = (pointNum == 1) ? circleIntersects(firstPoint, width / 2.0, m, c) : circleIntersects(secPoint, width / 2.0, m, c);
	}

	// Switch points around if necessary so we know element 0 holds the point that is to the left of the way
	if ((glm::atan(secPoint.y - firstPoint.y, secPoint.x - firstPoint.x) - glm::atan(points[0].y - firstPoint.y, points[0].x - firstPoint.x)) > 0)
	{ std::reverse(points.begin(), points.end()); }
	return points;
}

void NavDataProcess::triangulate(std::vector<uint64_t>& nodeIds, std::vector<std::array<uint64_t, 3>>& triangles) const
{
	triangles.clear();
	if (nodeIds.front() == nodeIds.back()) { nodeIds.pop_back(); }

	while (nodeIds.size() >= 3)
	{
		size_t size = nodeIds.size();

		for (uint32_t i = 0; i < nodeIds.size(); ++i)
		{
			Vertex prevNode;
			Vertex nextNode;
			Vertex currentNode = _osm.getNodeById(nodeIds[i]);
			std::vector<uint64_t> otherNodes = nodeIds;
			otherNodes.erase(otherNodes.begin() + i);

			if (i == 0)
			{
				prevNode = _osm.getNodeById(nodeIds.back());
				nextNode = _osm.getNodeById(nodeIds[1]);
				otherNodes.pop_back();
				otherNodes.erase(otherNodes.begin());
			}
			else if (i == (nodeIds.size() - 1))
			{
				prevNode = _osm.getNodeById(nodeIds[i - 1]);
				nextNode = _osm.getNodeById(nodeIds[0]);
				otherNodes.pop_back();
				otherNodes.erase(otherNodes.begin());
			}
			else
			{
				prevNode = _osm.getNodeById(nodeIds[i - 1]);
				nextNode = _osm.getNodeById(nodeIds[i + 1]);
				otherNodes.erase(otherNodes.begin() + i);
				otherNodes.erase(otherNodes.begin() + i - 1);
			}

			// Check if the vertex is interior or exterior
			if (checkWinding(std::vector<glm::dvec2>{ prevNode.coords, currentNode.coords, nextNode.coords }) == pvr::PolygonWindingOrder::FrontFaceCW) continue;

			// Determine if any of the other points are inside the triangle
			bool pointInTriangle = false;
			for (uint32_t j = 0; j < otherNodes.size(); ++j)
			{
				glm::dvec2 prevPoint = prevNode.coords - currentNode.coords;
				glm::dvec2 nextPoint = nextNode.coords - currentNode.coords;
				glm::dvec2 currentPoint = _osm.getNodeById(otherNodes[j]).coords - currentNode.coords;
				double d = prevPoint.x * nextPoint.y - nextPoint.x * prevPoint.y;

				double currentWeight =
					(currentPoint.x * (prevPoint.y - nextPoint.y) + currentPoint.y * (nextPoint.x - prevPoint.x) + prevPoint.x * nextPoint.y - nextPoint.x * prevPoint.y) / d;

				double prevWeight = (currentPoint.x * nextPoint.y - currentPoint.y * nextPoint.x) / d;
				double nextWeight = (currentPoint.y * prevPoint.x - currentPoint.x * prevPoint.y) / d;

				if ((currentWeight > 0.0) && (currentWeight < 1.0) && (prevWeight > 0.0) && (prevWeight < 1.0) && (nextWeight > 0.0) && (nextWeight < 1.0))
				{
					pointInTriangle = true;
					break;
				}
			}

			if (pointInTriangle) { continue; }

			// Add the new triangle and remove the necessary point
			triangles.emplace_back(std::array<uint64_t, 3>{ prevNode.id, currentNode.id, nextNode.id });
			nodeIds.erase(nodeIds.begin() + i);
			break;
		}

		if (size == nodeIds.size()) { break; }
	}
}

BuildingType::BuildingType NavDataProcess::getBuildingType(const Tag* tags, uint32_t numTags) const
{
	std::string value = "";

	for (uint32_t i = 0; i < numTags; ++i)
	{
		if (tags[i].key.compare("amenity") == 0 || tags[i].key.compare("shop") == 0)
		{
			value = tags[i].value;
			break;
		}
	}

	if (value.empty()) return BuildingType::None;
	if (value.compare("supermarket") == 0 || value.compare("convenience") == 0) return BuildingType::Shop;
	if (value.compare("bar") == 0) return BuildingType::Bar;
	if (value.compare("cafe") == 0) return BuildingType::Cafe;
	if (value.compare("fast_food") == 0) return BuildingType::FastFood;
	if (value.compare("pub") == 0) return BuildingType::Pub;
	if (value.compare("college") == 0) return BuildingType::College;
	if (value.compare("library") == 0) return BuildingType::Library;
	if (value.compare("university") == 0) return BuildingType::University;
	if (value.compare("atm") == 0) return BuildingType::ATM;
	if (value.compare("bank") == 0) return BuildingType::Bank;
	if (value.compare("restaurant") == 0) return BuildingType::Restaurant;
	if (value.compare("doctors") == 0) return BuildingType::Doctors;
	if (value.compare("dentist") == 0) return BuildingType::Dentist;
	if (value.compare("hospital") == 0) return BuildingType::Hospital;
	if (value.compare("pharmacy") == 0) return BuildingType::Pharmacy;
	if (value.compare("cinema") == 0) return BuildingType::Cinema;
	if (value.compare("casino") == 0) return BuildingType::Casino;
	if (value.compare("theatre") == 0) return BuildingType::Theatre;
	if (value.compare("fire_station") == 0) return BuildingType::FireStation;
	if (value.compare("courthouse") == 0) return BuildingType::Courthouse;
	if (value.compare("police") == 0) return BuildingType::Police;
	if (value.compare("post_office") == 0) return BuildingType::PostOffice;
	if (value.compare("toilets") == 0) return BuildingType::Toilets;
	if (value.compare("place_of_worship") == 0) return BuildingType::PlaceOfWorship;
	if (value.compare("fuel") == 0) return BuildingType::PetrolStation;
	if (value.compare("parking") == 0) return BuildingType::Parking;
	if (value.compare("post_box") == 0) return BuildingType::PostBox;
	if (value.compare("veterinary") == 0 || value.compare("pet") == 0) return BuildingType::Veterinary;
	if (value.compare("embassy") == 0) return BuildingType::Embassy;
	if (value.compare("hairdresser") == 0) return BuildingType::HairDresser;
	if (value.compare("butcher") == 0) return BuildingType::Butcher;
	if (value.compare("florist") == 0) return BuildingType::Florist;
	if (value.compare("optician") == 0) return BuildingType::Optician;
	return BuildingType::Other;
}

/// <summary>Provides two points on the perpendicular line at distance "width" apart for the middle point.</summary>
/// <returns>Return array of 2 new points. Left points (with respect to way direction) is given first.</returns>
/// <param name="firstPoint"> First point to use </param>
/// <param name="secPoint"> Second point to use </param>
/// <param name="thirdPoint"> Third point to use </param>
/// <param name="width"> Desired width between the new nodes </param>
std::array<glm::dvec2, 2> NavDataProcess::findPerpendicularPoints(glm::dvec2 firstPoint, glm::dvec2 secPoint, glm::dvec2 thirdPoint, double width) const
{
	std::array<glm::dvec2, 2> points;
	std::array<glm::dvec2, 2> first = findPerpendicularPoints(firstPoint, secPoint, width, 1);
	std::array<glm::dvec2, 2> sec1 = findPerpendicularPoints(firstPoint, secPoint, width, 2); // New points using line between first two given points
	std::array<glm::dvec2, 2> sec2 = findPerpendicularPoints(secPoint, thirdPoint, width, 1); // New points using line between second two given points
	std::array<glm::dvec2, 2> third = findPerpendicularPoints(secPoint, thirdPoint, width, 2);

	if (isVectorEqual(sec1[0], sec2[0]) && isVectorEqual(sec1[1], sec2[1])) // If the line section has no bend sec1 and sec2 will be equal
	{ points = sec1; }
	else // Most of the time they will not be equal
	{
		rayIntersect(first[0], sec1[0] - first[0], third[0], sec2[0] - third[0], points[0]);
		rayIntersect(first[1], sec1[1] - first[1], third[1], sec2[1] - third[1], points[1]);
	}
	return points;
}

std::string NavDataProcess::getIntersectionRoadName(const std::vector<std::vector<Tag>>& tags) const
{
	std::map<std::string, uint32_t> nameCount;
	uint32_t currentCount = 0;
	std::string name = "";

	for (auto& t : tags)
	{
		std::string name2 = getAttributeName(t.data(), t.size());
		if (!name2.empty()) nameCount[name2]++;
	}

	for (auto it = nameCount.begin(); it != nameCount.end(); ++it)
	{
		if (it->second > currentCount)
		{
			name = it->first;
			currentCount = it->second;
		}
	}

	cleanString(name);
	return name;
}

/// <summary>Clears data no longer needed from the osm object.</summary>
void OSM::cleanData()
{
	nodes.clear();
	originalRoadWays.clear();
	parkingWays.clear();
	buildWays.clear();
	convertedRoads.clear();
	original_intersections.clear();
	triangulatedRoads.clear();
	for (uint32_t lod = 0; lod < LOD::Count; ++lod)
	{
		labels[lod].clear();
		amenityLabels[lod].clear();
		icons[lod].clear();
	}
	uniqueIconNames.clear();
	nodes.clear();
	originalRoadWays.clear();
	parkingWays.clear();
	buildWays.clear();
	areaOutlines.clear();
	convertedRoads.clear();
	boundaryNodes.clear();
	intersectionNodes.clear();
	triangulatedRoads.clear();
	uniqueIconNames.clear();
}

/// <summary>Finds the dominant road type for a given intersection (used to colour the intersection based on the road type).</summary>
/// <returns>RoadTypes the type of road.</returns>
/// <param name="ways">Reference to a vector of ways which make up the intersection.</param>
RoadTypes::RoadTypes NavDataProcess::getIntersectionRoadType(const std::vector<Way>& ways) const
{
	std::vector<Way> tempWays = ways;
	std::sort(tempWays.begin(), tempWays.end(), NavDataProcess::compareRoadTypes);

	uint32_t maxCount = 0;
	uint32_t tempCount = 0;
	RoadTypes::RoadTypes current = RoadTypes::None;
	RoadTypes::RoadTypes temp = RoadTypes::Motorway;

	// Iterate through way and find which road type occurs the most.
	for (auto& way : tempWays)
	{
		if (way.roadType == temp)
		{
			tempCount++;
			current = temp;
		}
		else
		{
			if (tempCount > maxCount)
			{
				maxCount = tempCount;
				current = temp;
			}
			temp = way.roadType;
			tempCount = 1;
		}
	}
	return current;
}

double NavDataProcess::getRoadWidth(const std::vector<Tag>& tags, RoadTypes::RoadTypes& outType) const
{
	// Needs extension to include colour
	std::string roadType = "";
	for (Tag tag : tags)
	{
		if (tag.key == "highway")
		{
			roadType = tag.value;
			break;
		}
	}

	// Motorway, Trunk, Primary, Secondary, Other, Service
	if (roadType == "motorway")
	{
		outType = RoadTypes::Motorway;
		return 0.015;
	}
	if ((roadType == "trunk") || (roadType == "motorway_link"))
	{
		outType = RoadTypes::Trunk;
		return 0.01;
	}
	if ((roadType == "primary") || (roadType == "primary_link") || (roadType == "trunk_link"))
	{
		outType = RoadTypes::Primary;
		return 0.007;
	}
	if ((roadType == "secondary") || (roadType == "tertiary") || (roadType == "secondary_link") || (roadType == "tertiary_link"))
	{
		outType = RoadTypes::Secondary;
		return 0.005;
	}
	if ((roadType == "service"))
	{
		outType = RoadTypes::Service;
		return 0.0015;
	}

	outType = RoadTypes::RoadTypes::Other;
	return 0.0025;
}

void NavDataProcess::fillLabelTiles(LabelData& label, uint32_t lod)
{
	// Check if label is out of the map bounds
	if (isOutOfBounds(label.coords)) { return; }

	glm::uvec2 tileCoords = findTile2(label.coords);
	processLabelBoundary(label, tileCoords);

	_osm.tiles[tileCoords.x][tileCoords.y].labels[lod].emplace_back(label);
}

/// <summary>Determine the correct array to insert the way node id based on way type.</summary>
/// <param name="tileCoords">The tile co-ordinates where the data should be inserted.</param>
/// <param name="type">The type of way to insert (determines which vector the way will be inserted into).</param>
/// <param name="way">The way to insert.</param>
/// <param name="id">The node ID to insert.</param>
void NavDataProcess::insert(const glm::uvec2& tileCoords, WayTypes::WayTypes type, Way* w, uint64_t id)
{
	switch (type)
	{
	case WayTypes::Road:
	{
		if (w->area) { insertWay(_osm.tiles[tileCoords.x][tileCoords.y].areaWays, *w); }
		else
		{
			insertWay(_osm.tiles[tileCoords.x][tileCoords.y].roadWays, *w);
		}
		break;
	}
	case WayTypes::Parking:
	{
		insertWay(_osm.tiles[tileCoords.x][tileCoords.y].parkingWays, *w);
		break;
	}
	case WayTypes::Building:
	{
		insertWay(_osm.tiles[tileCoords.x][tileCoords.y].buildWays, *w);
		break;
	}
	case WayTypes::Inner:
	{
		insertWay(_osm.tiles[tileCoords.x][tileCoords.y].innerWays, *w);
		break;
	}
	case WayTypes::PolygonOutline:
	{
		_osm.tiles[tileCoords.x][tileCoords.y].polygonOutlineIds.emplace_back(id);
		break;
	}
	case WayTypes::AreaOutline:
	{
		if (w->area) _osm.tiles[tileCoords.x][tileCoords.y].areaOutlineIds.emplace_back(id);
		break;
	}
	default:
	{
		Log(LogLevel::Information, "Unrecognised way type.");
		break;
	}
	}
}

/// <summary>Fill tiles with icon data.</summary>
void NavDataProcess::fillIconTiles(IconData& icon, uint32_t lod)
{
	// Check if icon is out of the map bounds
	if (isOutOfBounds(icon.coords)) { return; }

	glm::uvec2 tileCoords = findTile2(icon.coords);

	_osm.tiles[tileCoords.x][tileCoords.y].icons[lod].emplace_back(icon);
}

void NavDataProcess::fillAmenityTiles(AmenityLabelData& label, uint32_t lod)
{
	// Check if label is out of the map bounds
	if (isOutOfBounds(label.coords)) { return; }

	glm::uvec2 tileCoords = findTile2(label.coords);
	processLabelBoundary(label, tileCoords);
	_osm.tiles[tileCoords.x][tileCoords.y].amenityLabels[lod].emplace_back(label);
}

/// <summary>Calculates the area of a triangle from a series of given points.</summary>
/// <returns> double The calculated area.</returns>
/// <param name="vector">An array of points (vec2s) that make up the polygon.</param>
double NavDataProcess::calculateTriangleArea(const std::vector<glm::dvec2>& points) const
{
	double area = 0.0;

	for (uint32_t i = 0; i < points.size(); ++i)
	{
		glm::dvec2 currentPoint = points[i];
		glm::dvec2 nextPoint = (i == (points.size() - 1)) ? points[0] : points[i + 1]; // Start and end node of a closed way are the same

		area += ((nextPoint.x - currentPoint.x) * (nextPoint.y + currentPoint.y));
	}
	return area / 2.0;
}
