#include "NavDataProcess.h"
#include "pugixml.hpp"

/// <summary>Initialisation of data, calls functions to load data from XML file and triangulate geometry.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result NavDataProcess::loadAndProcessData()
{
	// Set tile scaling parameters
	_osm.lonTileScale = 0.0015;
	_osm.latTileScale = 0.0015;

	pvr::Result result = loadOSMData();

	if (result != pvr::Result::Success) { return result; }

	initialiseTiles();
	calculateRoute();
	triangulateAllRoads();
	processLabels(_windowsDim);
	calculateIntersections();
	convertToTriangleList();

	return pvr::Result::Success;
}

/// <summary>Further initialisation - should be called after LoadAndProcess and once the window width/height is known.
/// This function fills the tiles with data which has been processed.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
void NavDataProcess::initTiles()
{
	sortTiles();

	for (auto& tileCol : _osm.tiles)
	{
		for (auto& tile : tileCol)
		{
			addCornerPoints(tile, tile.areaWays);
			addCornerPoints(tile, tile.buildWays);
			addCornerPoints(tile, tile.innerWays);
			addCornerPoints(tile, tile.parkingWays);
			addCornerPoints(tile, tile.roadWays);
		}
	}

	calculateMapBoundaryTexCoords();
	calculateJunctionTexCoords();

	cleanData();
}

/// <summary>Get map data and load into OSM object.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result NavDataProcess::loadOSMData()
{
#if defined(_WIN32) && defined(_DEBUG)
	// Enable memory-leak reports
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	pugi::xml_document mapData;
	std::vector<char> mapStream = _assetStream->readToEnd<char>();
	pugi::xml_parse_result result = mapData.load_string(mapStream.data(), static_cast<uint32_t>(mapStream.size()));

	Log(LogLevel::Debug, "XML parse result: %s", result.description());
	if (!result) { return pvr::Result::UnknownError; }

	// Get the bounds of the map
	_osm.maxLonLat =
		glm::vec2(mapData.root().child("osm").child("bounds").attribute("maxlon").as_double(), mapData.root().child("osm").child("bounds").attribute("maxlat").as_double());
	_osm.minLonLat =
		glm::vec2(mapData.root().child("osm").child("bounds").attribute("minlon").as_double(), mapData.root().child("osm").child("bounds").attribute("minlat").as_double());
	_osm.bounds.min = glm::dvec2(0, 0);
	_osm.bounds.max = lonLatToMetres(_osm.minLonLat, _osm.maxLonLat);

	// Collect the nodes
	pugi::xml_object_range<pugi::xml_named_node_iterator> nodes = mapData.root().child("osm").children("node");

	for (pugi::xml_named_node_iterator currentNode = nodes.begin(); currentNode != nodes.end(); ++currentNode)
	{
		if (!currentNode->attribute("visible").empty() && !currentNode->attribute("visible").as_bool()) // Skip node if not visible
			continue;

		Vertex tempNode;
		tempNode.height = 0.0;
		glm::dvec2 coords;

		// Get ID, latitude and longitude
		tempNode.id = currentNode->attribute("id").as_ullong();
		coords = glm::dvec2(currentNode->attribute("lon").as_double(), currentNode->attribute("lat").as_double());
		tempNode.coords = lonLatToMetres(_osm.minLonLat, coords);

		if (coords.x < _osm.minLonLat.x) tempNode.coords.x *= -1;
		if (coords.y < _osm.minLonLat.y) tempNode.coords.y *= -1;

		// Get tags from XML.
		pugi::xml_object_range<pugi::xml_named_node_iterator> tags = currentNode->children("tag");
		std::vector<Tag> tempTags;

		// Collect tags for this node.
		for (pugi::xml_named_node_iterator currentTag = tags.begin(); currentTag != tags.end(); ++currentTag)
		{
			Tag tempTag;
			tempTag.key = currentTag->attribute("k").as_string();
			tempTag.value = currentTag->attribute("v").as_string();
			tempTags.emplace_back(tempTag);
		}

		_osm.nodes[tempNode.id] = tempNode;
		generateIcon(&tempNode.id, 1, tempTags.data(), tempTags.size(), tempNode.id);
	}

	if (_osm.nodes.empty()) return pvr::Result::UnknownError;

	// Collect the ways
	pugi::xml_object_range<pugi::xml_named_node_iterator> ways = mapData.root().child("osm").children("way");

	for (pugi::xml_named_node_iterator currentWay = ways.begin(); currentWay != ways.end(); ++currentWay)
	{
		if (!currentWay->attribute("visible").empty() && !currentWay->attribute("visible").as_bool()) // Skip way if not visible
			continue;

		Way tempWay;
		WayTypes::WayTypes wayType = WayTypes::Default;
		tempWay.inner = false;
		tempWay.tileBoundWay = false;
		tempWay.area = false;
		tempWay.isFork = false;
		tempWay.isIntersection = false;
		tempWay.isRoundabout = false;
		tempWay.width = 0.0;

		// Get ID
		tempWay.id = currentWay->attribute("id").as_ullong();

		// Get tags
		pugi::xml_object_range<pugi::xml_named_node_iterator> tags = currentWay->children("tag");

		for (pugi::xml_named_node_iterator currentTag = tags.begin(); currentTag != tags.end(); ++currentTag)
		{
			Tag tempTag;

			tempTag.key = currentTag->attribute("k").as_string();
			tempTag.value = currentTag->attribute("v").as_string();
			tempWay.tags.emplace_back(tempTag);

			if ((tempTag.key == "highway") && (tempTag.value != "footway") && (tempTag.value != "bus_guideway") && (tempTag.value != "raceway") && (tempTag.value != "bridleway") &&
				(tempTag.value != "steps") && (tempTag.value != "path") && (tempTag.value != "cycleway") && (tempTag.value != "proposed") && (tempTag.value != "construction") &&
				(tempTag.value != "track") && (tempTag.value != "pedestrian"))
				wayType = WayTypes::Road;
			else if ((tempTag.key == "amenity") && (tempTag.value == "parking"))
				wayType = WayTypes::Parking;
			else if ((tempTag.key == "building") || (tempTag.key == "shop") || (tempTag.key == "landuse" && (tempTag.value == "retail")))
				wayType = WayTypes::Building;
			else if ((tempTag.key == "area") && (tempTag.value == "yes"))
				tempWay.area = true;
		}

		// Get node IDs
		pugi::xml_object_range<pugi::xml_named_node_iterator> nodeIds = currentWay->children("nd");

		for (pugi::xml_named_node_iterator currentNodeId = nodeIds.begin(); currentNodeId != nodeIds.end(); ++currentNodeId)
		{
			tempWay.nodeIds.emplace_back(currentNodeId->attribute("ref").as_ullong());

			if ((wayType == WayTypes::Road) && !tempWay.area)
			{
				Vertex& currentNode = _osm.nodes.find(tempWay.nodeIds.back())->second;
				currentNode.wayIds.emplace_back(tempWay.id);

				if (currentNode.wayIds.size() == 2) _osm.original_intersections.emplace_back(currentNode.id);
			}
		}

		// Add way to data structure based on type.
		switch (wayType)
		{
		case WayTypes::Road:
		{
			RoadTypes::RoadTypes type;
			tempWay.width = getRoadWidth(tempWay.tags, type);
			tempWay.roadType = type;
			tempWay.isRoundabout = isRoadRoundabout(tempWay.tags);

			std::string roadName = getAttributeName(tempWay.tags.data(), tempWay.tags.size());

			// Add a road name if none was available from the XML.
			if (roadName.empty())
			{
				Tag name;
				name.key = "name";
				name.value = "Unnamed Street";
				tempWay.tags.emplace_back(name);
			}
			else if (!roadName.empty() && !tempWay.isRoundabout)
			{
				LabelData label;
				for (uint32_t i = 0; i < tempWay.nodeIds.size(); ++i)
				{
					label.coords = _osm.nodes.find(tempWay.nodeIds[i])->second.coords;
					label.name = roadName;
					label.scale = static_cast<float>(tempWay.width + tempWay.width / 2.0);
					label.id = tempWay.id;
					label.isAmenityLabel = false;
					_osm.labels[LOD::LabelLOD].emplace_back(label);
				}
			}
			_osm.originalRoadWays[tempWay.id] = tempWay;
			break;
		}
		case WayTypes::Parking:
		{
			generateIcon(tempWay.nodeIds.data(), tempWay.nodeIds.size(), tempWay.tags.data(), tempWay.tags.size(), tempWay.id);
			_osm.parkingWays[tempWay.id] = tempWay;
			break;
		}
		case WayTypes::Building:
		{
			generateIcon(tempWay.nodeIds.data(), tempWay.nodeIds.size(), tempWay.tags.data(), tempWay.tags.size(), tempWay.id);
			_osm.buildWays[tempWay.id] = tempWay;
			break;
		}
		default: break;
		}
	}

	if (_osm.originalRoadWays.empty() && _osm.buildWays.empty() && _osm.parkingWays.empty()) { return pvr::Result::UnknownError; }

	// Use relation data to sort inner ways
	pugi::xml_object_range<pugi::xml_named_node_iterator> relations = mapData.root().child("osm").children("relation");

	for (pugi::xml_named_node_iterator currentRelation = relations.begin(); currentRelation != relations.end(); ++currentRelation)
	{
		if (!currentRelation->attribute("visible").empty() && !currentRelation->attribute("visible").as_bool()) // Skip way if not visible
		{ continue; } // Check tags to see if it describes a multi-polygon
		pugi::xml_object_range<pugi::xml_named_node_iterator> tags = currentRelation->children("tag");
		bool multiPolygon = false;

		for (pugi::xml_named_node_iterator currentTag = tags.begin(); currentTag != tags.end(); ++currentTag)
		{
			std::string key = currentTag->attribute("k").as_string();
			std::string value = currentTag->attribute("v").as_string();

			if ((key == "type") && (value == "multi-polygon")) { multiPolygon = true; }
		}

		if (!multiPolygon) { continue; }

		// Iterate through members to find outer way type
		pugi::xml_object_range<pugi::xml_named_node_iterator> members = currentRelation->children("member");

		WayTypes::WayTypes outerType = WayTypes::Default;
		for (pugi::xml_named_node_iterator currentMember = members.begin(); currentMember != members.end(); ++currentMember)
		{
			std::string type = currentMember->attribute("type").as_string();
			std::string role = currentMember->attribute("role").as_string();

			if ((type == "way") && (role == "outer"))
			{
				uint64_t wayId = currentMember->attribute("ref").as_ullong();

				if (_osm.parkingWays.find(wayId) != _osm.parkingWays.end()) { outerType = WayTypes::Parking; }
				else if (_osm.buildWays.find(wayId) != _osm.buildWays.end())
				{
					outerType = WayTypes::Building;
				}
			}
		}

		// Iterate through members again to find inner ways
		for (pugi::xml_named_node_iterator currentMember = members.begin(); currentMember != members.end(); ++currentMember)
		{
			std::string type = currentMember->attribute("type").as_string();
			std::string role = currentMember->attribute("role").as_string();

			if ((type == "way") && (role == "inner"))
			{
				uint64_t wayId = currentMember->attribute("ref").as_ullong();

				const auto& parkingTemp = _osm.parkingWays.find(wayId);
				const auto& buildTemp = _osm.buildWays.find(wayId);

				if ((parkingTemp != _osm.parkingWays.end()) && (outerType == WayTypes::Parking)) { parkingTemp->second.inner = true; }
				else if ((buildTemp != _osm.buildWays.end()) && (outerType == WayTypes::Building))
				{
					buildTemp->second.inner = true;
				}
			}
		}
	}
	return pvr::Result::Success;
}

void NavDataProcess::convertRoute(const glm::dvec2&, uint32_t numCols, uint32_t numRows, float& totalRouteDistance)
{
	(void)numRows;
	(void)numCols;
	for (uint32_t i = 0; i < getRouteData().size(); ++i)
	{
		getRouteData()[i].point = remap(getRouteData()[i].point, getTiles()[0][0].min, getTiles()[0][0].max, glm::dvec2(-5, -5), glm::dvec2(5, 5));

		if (i > 0)
		{
			glm::vec2 previousPoint = getRouteData()[i - 1].point;
			glm::vec2 currentPoint = getRouteData()[i].point;

			float partialDistance = glm::distance(currentPoint, previousPoint);
			getRouteData()[i - 1].distanceToNext = partialDistance;
			totalRouteDistance += partialDistance; // The total 'true' distance of the path

			float angle = -static_cast<float>(calculateAngleBetweenPoints(previousPoint, currentPoint));
			getRouteData()[i - 1].rotation = angle;
			glm::vec3 currentDir = glm::vec3(getRouteData()[i].point - getRouteData()[i - 1].point, 0.0f);

			currentDir = glm::normalize(currentDir);
			getRouteData()[i - 1].dir = glm::dvec2(currentDir);
			std::swap(currentDir.y, currentDir.z);
		}
	}
}

/// <summary>Iterates over available intersections and calculates a 'random' route through the available data set,
/// if no intersections are available no route will be calculated.</summary>
void NavDataProcess::calculateRoute()
{
	if (_osm.original_intersections.size() == 0)
	{
		Log(LogLevel::Information, "No Route Calculated - No intersections.");
		return;
	}

	uint32_t count = 0;
	// Hold previously visited IDs to prevent going back on ourselves.
	std::set<uint64_t> previousIntersectIDs;
	std::set<uint64_t> previousWayIDs;
	uint64_t nextID = _osm.original_intersections[0];
	uint64_t lastID = 0;
	std::vector<std::pair<uint64_t, glm::dvec2>> tempCoords;

	while (previousIntersectIDs.size() < _osm.original_intersections.size())
	{
		bool nextJunctionFound = false;
		Vertex node = _osm.nodes.find(nextID)->second;

		// Find the next way for the route.
		for (uint32_t i = 0; i < node.wayIds.size(); ++i)
		{
			Way way = _osm.originalRoadWays.find(node.wayIds[i])->second;

			// Make sure we have not visited this way in the past.
			if (previousWayIDs.find(way.id) == previousWayIDs.end())
			{
				previousWayIDs.insert(way.id);

				for (auto id : way.nodeIds)
				{
					glm::dvec2 coords = _osm.nodes.find(id)->second.coords;

					// Check the node is not outside the map boundary.
					if (isOutOfBounds(coords))
					{
						if (tempCoords.size() > 0) tempCoords.clear();

						continue;
					}

					tempCoords.emplace_back(std::pair<uint64_t, glm::dvec2>(id, coords));

					if (!nextJunctionFound)
					{
						// Find the next node that is an intersection.
						for (uint32_t j = 0; j < _osm.original_intersections.size(); ++j)
						{
							if (id == _osm.original_intersections[j] && previousIntersectIDs.find(id) == previousIntersectIDs.end())
							{
								previousIntersectIDs.insert(id);
								nextID = id;
								nextJunctionFound = true;
								break;
							}
						}
					}

					if (nextJunctionFound)
					{
						if (way.nodeIds[0] == nextID && way.nodeIds.back() == node.id && way.nodeIds.size() > 2)
						{
							glm::dvec2 p1 = _osm.nodes.find(node.id)->second.coords;
							glm::dvec2 p2 = _osm.nodes.find(way.nodeIds[0])->second.coords;
							glm::dvec2 p3 = _osm.nodes.find(way.nodeIds[way.nodeIds.size() / 2])->second.coords;

							float a1 = static_cast<float>(glm::atan(p1.y - p2.y, p1.x - p2.x));
							float a2 = static_cast<float>(glm::atan(p1.y - p3.y, p1.x - p3.x));

							if (glm::abs(a1 - a2) > 0.25f)
							{
								std::reverse(way.nodeIds.begin(), way.nodeIds.end());
								tempCoords.clear();

								for (uint32_t j = 0; j < way.nodeIds.size(); ++j)
									tempCoords.emplace_back(std::pair<uint64_t, glm::dvec2>(way.nodeIds[j], _osm.nodes.find(way.nodeIds[j])->second.coords));
							}
						}
						break;
					}
				}
			}
			// Add the found nodes to the route.
			if (nextJunctionFound)
			{
				for (uint32_t j = 0; j < tempCoords.size(); ++j)
				{
					if (lastID == tempCoords[j].first) { continue; }

					RouteData data;
					data.distanceToNext = 0.0f;
					data.point = tempCoords[j].second;
					data.name = getAttributeName(way.tags.data(), way.tags.size());
					_osm.route.emplace_back(data);
				}

				lastID = tempCoords.back().first;
				tempCoords.clear();
				break;
			}
			tempCoords.clear();
		}

		// If no junction was found end the route.
		if (!nextJunctionFound) break;

		count++;
	}
}

/// <summary>Check if the incoming entity is an amenity or service, if it is create an icon for it, and possibly a label if a name is present.</summary>
/// <param name="nodeIds">The nodes IDs that make up this entity.</param>
/// <param name="tags">The tags associated with this entity, which may contain the type of amenity / service and name.</param>
/// <param name="id">The id to use to add this icon to the array of icons.</param>
void NavDataProcess::generateIcon(const uint64_t* nodeIds, size_t numNodeIds, const Tag* tags, size_t numTags, uint64_t id)
{
	static const uint32_t maxLineLen = 10;

	BuildingType::BuildingType type = getBuildingType(tags, static_cast<uint32_t>(numTags));
	if (type != BuildingType::None)
	{
		std::string name = getAttributeName(tags, numTags);
		bool nameEmpty = name.empty();

		if (_osm.uniqueIconNames.find(name) != _osm.uniqueIconNames.end() || (type == BuildingType::Other && nameEmpty)) { return; }

		// Calculate the icons co-ordinates by averaging the nodes co-ordinates that make up the building.
		glm::dvec2 coord = glm::dvec2(0, 0);
		for (uint32_t i = 0; i < numNodeIds; ++i) coord += _osm.nodes.find(nodeIds[i])->second.coords;

		coord /= double(numNodeIds);

		IconData icon;
		icon.buildingType = type;
		icon.coords = coord;
		icon.scale = 0.005f;
		icon.lodLevel = LOD::L2;
		_osm.icons[LOD::IconLOD].emplace_back(icon);

		// Check if this building has a name, if it does create a label for it.
		if (!nameEmpty)
		{
			_osm.uniqueIconNames.insert(name);

			AmenityLabelData label;
			label.scale = 0.003f;
			// move the amenity label below the icon
			label.coords = coord - glm::dvec2(0, 0.00175);
			label.name = name;
			label.id = id;
			label.rotation = 0.0f;
			label.iconData = icon;
			label.maxLodLevel = LOD::L1;
			// Split long names
			if (name.length() > maxLineLen)
			{
				std::size_t pos = name.find_first_of(' ', maxLineLen);
				pos = (pos == std::string::npos ? name.find_last_of(' ') : pos);

				if (pos != std::string::npos) { name.insert(pos + 1, "\n"); }
			}
			_osm.amenityLabels[LOD::AmenityLabelLOD].emplace_back(label);
		}
	}
}

/// <summary>Calculate actual label position based on the average of two nodes, also calculates the rotation that will be
/// applied to the text based on the slope of the road segment i.e. the line between the two nodes. </summary>
void NavDataProcess::processLabels(const glm::dvec2& mapWorldDim)
{
	for (int lod = 0; lod <= LOD::Count; ++lod)
	{
		auto& osmlodlabels = _osm.labels[lod];
		if (osmlodlabels.size() == 0) { continue; }
		static const float minDistLabels = 0.03f; // Minimum distance two labels can be apart, to prevent crowding / overlaps.
		std::vector<LabelData> temp;

		for (uint32_t i = 0; i < osmlodlabels.size() - 1; ++i)
		{
			if (i > 0)
			{
				LabelData label;
				// Check labels came from the same way.
				if (osmlodlabels[i].id == osmlodlabels[i + 1].id)
				{
					if (glm::distance(osmlodlabels[i].coords, osmlodlabels[i + 1].coords) < 0.01) { continue; }

					label = osmlodlabels[i];

					glm::dvec2 pos = (osmlodlabels[i].coords + osmlodlabels[i + 1].coords) / 2.0;
					label.distToEndOfSegment = static_cast<float>(glm::distance(pos, osmlodlabels[i].coords));

					if (temp.size() > 0)
					{
						double dist = glm::distance(temp.back().coords, pos);
						if (dist < minDistLabels) { continue; }
					}

					// Remap co-ordinates into screen space to calculate the accurate angle of the line.
					glm::vec2 remappedPos1 = glm::vec2(remap(osmlodlabels[i + 1].coords, _osm.tiles[0][0].min, _osm.tiles[0][0].max,
						glm::dvec2(-double(mapWorldDim.x) / 2.0, -double(mapWorldDim.y) / 2.0), glm::dvec2(double(mapWorldDim.x) / 2.0, double(mapWorldDim.y) / 2.0)));

					glm::vec2 remappedPos2 = glm::vec2(remap(osmlodlabels[i].coords, _osm.tiles[0][0].min, _osm.tiles[0][0].max,
						glm::dvec2(-double(mapWorldDim.x) / 2.0, -double(mapWorldDim.y) / 2.0), glm::dvec2(double(mapWorldDim.x) / 2.0, double(mapWorldDim.y) / 2.0)));

					// Compute rotation (in radians) for label based on slope of line y / x
					float angle = glm::atan(remappedPos1.y - remappedPos2.y, remappedPos1.x - remappedPos2.x);

					// Correct for angle being beyond (+/-) 90 degrees, this prevents text being displayed upside down.
					if (angle <= -glm::half_pi<float>()) { angle += glm::pi<float>(); }
					else if (angle >= glm::half_pi<float>())
					{
						angle -= glm::pi<float>();
					}

					label.coords = pos;
					label.rotation = angle;
					label.maxLodLevel = LOD::L4;
					temp.emplace_back(label);
				}
			}
		}

		osmlodlabels.clear();
		osmlodlabels.insert(osmlodlabels.begin(), temp.begin(), temp.end());
	}
}

/// <summary>Convert longitude and latitude to x and y from a given origin.</summary>
/// <param name ="origin">Longitude then latitude to use as origin.</param>
/// <param name ="point">Longitude then latitude of point to convert.</param>
/// <returns>The point in terms of x and y from the origin.</returns>
glm::dvec2 NavDataProcess::lonLatToMetres(const glm::dvec2 origin, const glm::dvec2 point) const
{
	glm::dvec2 coords;
	// Approx radius of Earth
	static const double radius = 6371.0;
	static const double pi = glm::pi<double>();

	// Determine the x coordinate
	double v = glm::sin((point.x * pi / 180.0 - origin.x * pi / 180.0) / 2.0);
	coords.x = 2.0 * radius * glm::asin(glm::sqrt(glm::cos(origin.y * pi / 180.0) * glm::cos(origin.y * pi / 180.0) * v * v));

	// Determine the y coordinate
	double u = sin((point.y * pi / 180.0 - origin.y * pi / 180.0) / 2.0);
	coords.y = 2.0 * radius * glm::asin(glm::sqrt(u * u));

	return coords;
}

/// <summary>Convert all roads to triangles.</summary>
void NavDataProcess::triangulateAllRoads()
{
	// Triangulate the roads
	for (auto wayIterator = _osm.originalRoadWays.begin(); wayIterator != _osm.originalRoadWays.end(); ++wayIterator)
	{
		if (wayIterator->second.area)
			_osm.triangulatedRoads[wayIterator->first] = wayIterator->second;
		else
		{
			uint32_t breakIndex = 0;
			// Increases node density around sharp bends, which in turn increases the number of triangles produced by the triangulation function,
			// which improves visual quality for sharp bends, at the cost of memory usage, initialisation time and potentially frame times
			if (wayIterator->second.nodeIds.size() > 2) wayIterator->second.nodeIds = tessellate(wayIterator->second.nodeIds, breakIndex);

			// This loop breaks a way if the start or end intersects with another part of the way
			for (uint32_t i = 1; i < (wayIterator->second.nodeIds.size() - 1); ++i)
			{
				if ((wayIterator->second.nodeIds[i] == wayIterator->second.nodeIds.front()) || (wayIterator->second.nodeIds[i] == wayIterator->second.nodeIds.back()))
				{
					if (wayIterator->second.nodeIds[i] == wayIterator->second.nodeIds.back()) { i = static_cast<uint32_t>(wayIterator->second.nodeIds.size() - i - 1); }

					Way newWay = wayIterator->second;
					newWay.id = _osm.originalRoadWays.rbegin()->first + 1;

					std::vector<uint64_t> newIds;
					newIds.insert(newIds.end(), wayIterator->second.nodeIds.begin() + i, wayIterator->second.nodeIds.end());
					wayIterator->second.nodeIds.erase(wayIterator->second.nodeIds.begin() + i + 1, wayIterator->second.nodeIds.end());
					uint32_t intersectSize = static_cast<uint32_t>(_osm.nodes.find(newIds[0])->second.wayIds.size());
					_osm.nodes.find(newIds[0])->second.wayIds.emplace_back(newWay.id);

					// Update the intersections for the nodes
					for (uint32_t j = 1; j < newIds.size(); ++j)
					{
						std::vector<uint64_t>& wayIds = _osm.nodes.find(newIds[j])->second.wayIds;
						for (uint32_t k = 0; k < wayIds.size(); ++k)
						{
							if (wayIds[k] == wayIterator->first)
							{
								wayIds.erase(wayIds.begin() + k);
								break;
							}
						}
						wayIds.emplace_back(newWay.id);
					}

					newWay.nodeIds = newIds;
					_osm.originalRoadWays[newWay.id] = newWay;

					if (intersectSize == 2) _osm.original_intersections.emplace_back(newIds[0]);

					break;
				}
			}
			// This breaks a closed way
			if (wayIterator->second.nodeIds.front() == wayIterator->second.nodeIds.back())
			{
				// If the start or end of a closed road is next to an intersection, move it onto the intersection to avoid artefacts
				if ((_osm.nodes.find(wayIterator->second.nodeIds[1])->second.wayIds.size() > 1) && (_osm.nodes.find(wayIterator->second.nodeIds[0])->second.wayIds.size() == 2))
				{
					_osm.nodes.find(wayIterator->second.nodeIds[0])->second.wayIds.pop_back();
					wayIterator->second.nodeIds.erase(wayIterator->second.nodeIds.begin());
					wayIterator->second.nodeIds.emplace_back(wayIterator->second.nodeIds[0]);
					_osm.nodes.find(wayIterator->second.nodeIds[0])->second.wayIds.emplace_back(wayIterator->first);
				}
				else if ((_osm.nodes.find(wayIterator->second.nodeIds[wayIterator->second.nodeIds.size() - 2])->second.wayIds.size() > 1) &&
					(_osm.nodes.find(wayIterator->second.nodeIds.back())->second.wayIds.size() == 2))
				{
					_osm.nodes.find(wayIterator->second.nodeIds.back())->second.wayIds.pop_back();
					wayIterator->second.nodeIds.pop_back();
					wayIterator->second.nodeIds.insert(wayIterator->second.nodeIds.begin(), wayIterator->second.nodeIds.back());
					_osm.nodes.find(wayIterator->second.nodeIds.back())->second.wayIds.emplace_back(wayIterator->first);
				}

				// If the break point of the way is next to an intersection, move it onto the intersection
				if ((_osm.nodes.find(wayIterator->second.nodeIds[breakIndex + 1])->second.wayIds.size() > 1) &&
					(_osm.nodes.find(wayIterator->second.nodeIds[breakIndex])->second.wayIds.size() == 1))
					breakIndex++;
				else if ((_osm.nodes.find(wayIterator->second.nodeIds[breakIndex - 1])->second.wayIds.size() > 1) &&
					(_osm.nodes.find(wayIterator->second.nodeIds[breakIndex])->second.wayIds.size() == 1))
					breakIndex--;

				Way newWay = wayIterator->second;
				newWay.id = _osm.originalRoadWays.rbegin()->first + 1;

				std::vector<uint64_t> newIds;
				newIds.insert(newIds.end(), wayIterator->second.nodeIds.begin() + breakIndex, wayIterator->second.nodeIds.end());
				wayIterator->second.nodeIds.erase(wayIterator->second.nodeIds.begin() + breakIndex + 1, wayIterator->second.nodeIds.end());
				uint32_t intersectSize = static_cast<uint32_t>(_osm.nodes.find(newIds[0])->second.wayIds.size());
				_osm.nodes.find(newIds[0])->second.wayIds.emplace_back(newWay.id);

				// Update the intersections for the nodes
				for (uint32_t i = 1; i < newIds.size(); ++i)
				{
					std::vector<uint64_t>& wayIds = _osm.nodes.find(newIds[i])->second.wayIds;
					for (uint32_t j = 0; j < wayIds.size(); ++j)
					{
						if (wayIds[j] == wayIterator->first)
						{
							wayIds.erase(wayIds.begin() + j);
							break;
						}
					}
					wayIds.emplace_back(newWay.id);
				}

				newWay.nodeIds = newIds;
				_osm.originalRoadWays[newWay.id] = newWay;

				if (intersectSize == 1) _osm.original_intersections.emplace_back(newIds[0]);
			}

			_osm.triangulatedRoads[wayIterator->first] = wayIterator->second;
			_osm.triangulatedRoads.find(wayIterator->first)->second.nodeIds = triangulateRoad(wayIterator->second.nodeIds, wayIterator->second.width);
		}
	}
}

/// <summary>Calculate road intersections.</summary>
void NavDataProcess::calculateIntersections()
{
	// Calculate intersections
	for (uint32_t i = 0; i < _osm.original_intersections.size(); ++i)
	{
		Vertex n = _osm.nodes.find(_osm.original_intersections[i])->second;
		if (n.wayIds.size() < 2) continue;

		bool endsOnly = true;
		std::vector<Way> originalWays;
		std::vector<Way> newWays;

		// Determine if the junction involves only the start or end of a way
		for (uint32_t j = 0; j < n.wayIds.size(); ++j)
		{
			originalWays.emplace_back(_osm.originalRoadWays.find(n.wayIds[j])->second);
			newWays.emplace_back(_osm.triangulatedRoads.find(n.wayIds[j])->second);

			if ((originalWays.back().nodeIds.front() != _osm.original_intersections[i]) && (originalWays.back().nodeIds.back() != _osm.original_intersections[i])) endsOnly = false;
		}

		if (!endsOnly) // If there is a junction part way through a road
		{
			uint64_t midWayId;
			for (uint32_t j = 0; j < originalWays.size(); ++j)
			{
				if ((originalWays[j].nodeIds.front() != _osm.original_intersections[i]) && (originalWays[j].nodeIds.back() != _osm.original_intersections[i]))
				{
					midWayId = originalWays[j].id;
					break;
				}
			}

			Way& originalWay = _osm.originalRoadWays.find(midWayId)->second;
			Way& newWay = _osm.triangulatedRoads.find(midWayId)->second;

			uint32_t intersectIndex = 0;
			for (uint32_t j = 1; j < (originalWay.nodeIds.size() - 1); ++j)
			{
				if (originalWay.nodeIds[j] == _osm.original_intersections[i])
				{
					intersectIndex = j;
					break;
				}
			}

			uint32_t newIntersectIndex = intersectIndex * 2 + 1;

			uint64_t newId = _osm.originalRoadWays.rbegin()->first + 1;
			Way newLineStrip = originalWay;
			Way newTriStrip = newWay;
			newLineStrip.id = newId;
			newTriStrip.id = newId;
			newLineStrip.nodeIds.clear();
			newTriStrip.nodeIds.clear();

			// Break the way in two
			newLineStrip.nodeIds.insert(newLineStrip.nodeIds.end(), originalWay.nodeIds.begin() + intersectIndex, originalWay.nodeIds.end());
			originalWay.nodeIds.erase(originalWay.nodeIds.begin() + intersectIndex + 1, originalWay.nodeIds.end());
			_osm.nodes.find(originalWay.nodeIds.back())->second.wayIds.emplace_back(newId);

			for (uint32_t j = 1; j < newLineStrip.nodeIds.size(); ++j)
			{
				std::vector<uint64_t>& wayIds = _osm.nodes.find(newLineStrip.nodeIds[j])->second.wayIds;
				for (size_t k = 0; k < wayIds.size(); ++k)
				{
					if (wayIds[k] == originalWay.id)
					{
						wayIds.erase(wayIds.begin() + k);
						break;
					}
				}
				wayIds.emplace_back(newId);
			}

			newTriStrip.nodeIds.insert(newTriStrip.nodeIds.end(), newWay.nodeIds.begin() + newIntersectIndex - 1, newWay.nodeIds.end());
			newWay.nodeIds.erase(newWay.nodeIds.begin() + newIntersectIndex + 1, newWay.nodeIds.end());

			Vertex newNode0 = _osm.nodes.find(newTriStrip.nodeIds[0])->second;
			Vertex newNode1 = _osm.nodes.find(newTriStrip.nodeIds[1])->second;
			Vertex newNode2 = _osm.nodes.find(newTriStrip.nodeIds[2])->second;
			newNode0.id = _osm.nodes.rbegin()->first + 1;
			newNode1.id = _osm.nodes.rbegin()->first + 2;
			newNode2.id = _osm.nodes.rbegin()->first + 3;

			_osm.nodes[newNode0.id] = newNode0;
			_osm.nodes[newNode1.id] = newNode1;
			_osm.nodes[newNode2.id] = newNode2;

			newTriStrip.nodeIds[0] = newNode0.id;
			newTriStrip.nodeIds[1] = newNode1.id;
			newTriStrip.nodeIds[2] = newNode2.id;

			_osm.originalRoadWays[newId] = newLineStrip;
			_osm.triangulatedRoads[newId] = newTriStrip;
			_osm.original_intersections.emplace_back(originalWay.nodeIds.back());
		}
		else // If only the ends of ways are involved
		{
			for (uint32_t j = 0; j < originalWays.size(); ++j)
			{
				if (originalWays[j].nodeIds.front() != _osm.original_intersections[i])
				{
					std::reverse(originalWays[j].nodeIds.begin(), originalWays[j].nodeIds.end());
					std::reverse(newWays[j].nodeIds.begin(), newWays[j].nodeIds.end());
				}
			}

			std::vector<Way> orderedWays;
			orderedWays.emplace_back(newWays[0]);
			glm::dvec2 centrePoint = n.coords;
			glm::dvec2 currentPoint = _osm.nodes.find(originalWays[0].nodeIds[1])->second.coords;
			originalWays.erase(originalWays.begin());
			newWays.erase(newWays.begin());

			// Determine the order in which to find intersections
			while (originalWays.size() > 1)
			{
				float angle = 2.0f * glm::pi<float>();
				uint32_t wayNum = 0;

				for (uint32_t j = 0; j < originalWays.size(); ++j)
				{
					glm::dvec2 nextPoint = _osm.nodes.find(originalWays[j].nodeIds[1])->second.coords;
					float currentAngle = static_cast<float>(
						glm::atan(nextPoint.y - centrePoint.y, nextPoint.x - centrePoint.x) - glm::atan(currentPoint.y - centrePoint.y, currentPoint.x - centrePoint.x));

					if (currentAngle < 0.0f) currentAngle = 2.0f * glm::pi<float>() + currentAngle;

					if (currentAngle < angle)
					{
						angle = currentAngle;
						wayNum = j;
					}
				}

				currentPoint = _osm.nodes.find(originalWays[wayNum].nodeIds[1])->second.coords;
				orderedWays.emplace_back(newWays[wayNum]);
				newWays.erase(newWays.begin() + wayNum);
				originalWays.erase(originalWays.begin() + wayNum);
			}
			orderedWays.emplace_back(newWays[0]);

			// Find intersections
			std::vector<uint64_t> newIds;
			for (uint32_t j = 0; j < orderedWays.size(); ++j)
			{
				bool done = false;
				uint32_t wayNum = (j < (orderedWays.size() - 1)) ? (j + 1) : 0;
				uint32_t prevWayNum = static_cast<uint32_t>((j > 0) ? (j - 1) : (orderedWays.size() - 1));

				if (orderedWays.size() > 2)
				{
					uint32_t firstSize = static_cast<uint32_t>(orderedWays[j].nodeIds.size());
					uint32_t secSize = static_cast<uint32_t>(orderedWays[wayNum].nodeIds.size());
					uint32_t prevSize = static_cast<uint32_t>(orderedWays[prevWayNum].nodeIds.size());

					// Run through left hand node IDs for the first way
					for (uint32_t k = 0; k < (firstSize - 2); k += 2)
					{
						glm::dvec2& point1 = _osm.nodes.find(orderedWays[j].nodeIds[k])->second.coords;
						glm::dvec2 point2 = _osm.nodes.find(orderedWays[j].nodeIds[k + 2])->second.coords;

						if (compareReal(point1.x, point2.x) && compareReal(point1.y, point2.y)) continue;

						// Run through right hand node IDs for the next way
						for (uint32_t m = 1; m < (secSize - 2); m += 2)
						{
							glm::dvec2& point3 = _osm.nodes.find(orderedWays[wayNum].nodeIds[m])->second.coords;
							glm::dvec2 point4 = _osm.nodes.find(orderedWays[wayNum].nodeIds[m + 2])->second.coords;

							if (compareReal(point3.x, point4.x) && compareReal(point3.y, point4.y)) continue;

							glm::dvec2 newPoint = lineIntersect(point1, point2 - point1, point3, point4 - point3);
							double minX = glm::max(glm::min(point1.x, point2.x), glm::min(point3.x, point4.x));
							double maxX = glm::min(glm::max(point1.x, point2.x), glm::max(point3.x, point4.x));
							double minY = glm::max(glm::min(point1.y, point2.y), glm::min(point3.y, point4.y));
							double maxY = glm::min(glm::max(point1.y, point2.y), glm::max(point3.y, point4.y));

							if ((newPoint.x >= minX) && (newPoint.x <= maxX) && (newPoint.y >= minY) && (newPoint.y <= maxY))
							{
								point1 = newPoint;
								point3 = newPoint;

								newIds.emplace_back(orderedWays[j].nodeIds[k]);

								for (uint32_t nn = 0; nn < k; nn += 2) { _osm.nodes.find(orderedWays[j].nodeIds[nn])->second.coords = newPoint; }
								for (uint32_t p = 1; p < m; p += 2) { _osm.nodes.find(orderedWays[wayNum].nodeIds[p])->second.coords = newPoint; }

								done = true;
								break;
							}
						}
						if (done) break;

						// Special case: Run through left hand node IDs for previous way
						for (uint32_t m = 0; m < (prevSize - 2); m += 2)
						{
							glm::dvec2 point3 = _osm.nodes.find(orderedWays[prevWayNum].nodeIds[m])->second.coords;
							glm::dvec2 point4 = _osm.nodes.find(orderedWays[prevWayNum].nodeIds[m + 2])->second.coords;

							if (compareReal(point3.x, point4.x) && compareReal(point3.y, point4.y)) continue;

							glm::dvec2 newPoint = lineIntersect(point1, point2 - point1, point3, point4 - point3);
							double minX = glm::max(glm::min(point1.x, point2.x), glm::min(point3.x, point4.x));
							double maxX = glm::min(glm::max(point1.x, point2.x), glm::max(point3.x, point4.x));
							double minY = glm::max(glm::min(point1.y, point2.y), glm::min(point3.y, point4.y));
							double maxY = glm::min(glm::max(point1.y, point2.y), glm::max(point3.y, point4.y));

							if ((newPoint.x >= minX) && (newPoint.x <= maxX) && (newPoint.y >= minY) && (newPoint.y <= maxY))
							{
								std::vector<uint64_t> currentIds = orderedWays[wayNum].nodeIds;
								point1 = newPoint;

								_osm.nodes.find(orderedWays[wayNum].nodeIds[1])->second.coords = newPoint;
								newIds.emplace_back(orderedWays[j].nodeIds[0]);

								for (uint32_t nn = 0; nn < k; nn += 2) { _osm.nodes.find(orderedWays[j].nodeIds[nn])->second.coords = newPoint; }

								done = true;
								break;
							}
						}
						if (done) break;
					}
				}

				// If intersection wasn't found (or if there are only two ways in this junction)
				if (!done)
				{
					glm::dvec2 point1 = _osm.nodes.find(orderedWays[j].nodeIds[2])->second.coords;
					glm::dvec2& point2 = _osm.nodes.find(orderedWays[j].nodeIds[0])->second.coords;
					glm::dvec2 point3 = _osm.nodes.find(orderedWays[wayNum].nodeIds[3])->second.coords;
					glm::dvec2& point4 = _osm.nodes.find(orderedWays[wayNum].nodeIds[1])->second.coords;

					point4 = point2 = lineIntersect(point1, point2 - point1, point3, point4 - point3);
					newIds.emplace_back(orderedWays[j].nodeIds[0]);
				}
			}

			if (n.wayIds.size() > 2)
			{
				std::vector<std::vector<Tag>> temp;
				bool roundabout = false;
				uint32_t oneWayCount = 0;
				double width = 0;

				for (Way& w : orderedWays)
				{
					temp.emplace_back(w.tags);
					if (w.isRoundabout) roundabout = true;
					if (w.width > width) width = w.width;
					if (isRoadOneWay(w.tags)) oneWayCount++;
				}

				Tag t;
				t.key = "name";
				t.value = getIntersectionRoadName(temp);

				ConvertedWay intersection(_osm.originalRoadWays.rbegin()->first + 1, false, std::vector<Tag>() = { t }, getIntersectionRoadType(orderedWays));

				intersection.isIntersection = true;
				intersection.isRoundabout = roundabout;
				intersection.width = width;
				intersection.isFork = oneWayCount == 2;

				for (uint32_t j = 1; j < (newIds.size() - 1); ++j) intersection.triangulatedIds.emplace_back(std::array<uint64_t, 3>{ newIds[0], newIds[j], newIds[j + 1] });

				_osm.convertedRoads[intersection.id] = intersection;
				_osm.originalRoadWays[intersection.id] = intersection; // To keep track of the new IDs
			}
		}
	}
}

/// <summary>Convert triangles into an ordered triangle list.</summary>
/// <param name="newRoads" Reference to temporary data shared between TriangulateAllRoads, CalculateIntersections and ConvertToTriangleList.</param>
void NavDataProcess::convertToTriangleList()
{
	std::vector<std::array<uint64_t, 3>> triangles;
	// Finally sort into triangle lists and get outlines ready for tiling
	for (auto wayIterator = _osm.triangulatedRoads.begin(); wayIterator != _osm.triangulatedRoads.end(); ++wayIterator)
	{
		ConvertedWay convertedRoad(wayIterator->first, wayIterator->second.area, wayIterator->second.tags, wayIterator->second.roadType, wayIterator->second.width,
			wayIterator->second.isIntersection, wayIterator->second.isRoundabout, wayIterator->second.isFork);

		if (wayIterator->second.area) // Handle road areas
		{
			for (uint32_t i = 0; i < (wayIterator->second.nodeIds.size() - 1); ++i)
			{
				_osm.areaOutlines.emplace_back(wayIterator->second.nodeIds[i]);
				_osm.areaOutlines.emplace_back(wayIterator->second.nodeIds[i + 1]);
			}

			if (checkWinding(wayIterator->second.nodeIds) == pvr::PolygonWindingOrder::FrontFaceCW)
				std::reverse(wayIterator->second.nodeIds.begin(), wayIterator->second.nodeIds.end());

			triangulate(wayIterator->second.nodeIds, triangles);

			for (uint32_t i = 0; i < triangles.size(); ++i) { convertedRoad.triangulatedIds.emplace_back(triangles[i]); }
		}
		else
		{
			Way& way = _osm.originalRoadWays.find(wayIterator->first)->second;

			// Calculate end caps for roads which are dead ends.
			if (way.nodeIds.size() > 1)
			{
				// End of road segment.
				if (_osm.getNodeById(way.nodeIds.back()).wayIds.size() == 1)
				{
					Vertex& n1 = _osm.getNodeById(wayIterator->second.nodeIds.back());
					Vertex& n2 = _osm.getNodeById(wayIterator->second.nodeIds.back() - 1);

					// Check both nodes are within map limits to prevent artefacts.
					if (!isOutOfBounds(n1.coords) && !isOutOfBounds(n2.coords))
					{
						auto nodes = calculateEndCaps(n1, n2, wayIterator->second.width);

						wayIterator->second.nodeIds.emplace_back(nodes[0]);
						wayIterator->second.nodeIds.emplace_back(n2.id); // Need a repeated node to complete triangle list.
						wayIterator->second.nodeIds.emplace_back(nodes[1]);
					}
				}
				// Start of road segment.
				if (_osm.getNodeById(way.nodeIds[0]).wayIds.size() == 1)
				{
					Vertex& n1 = _osm.getNodeById(wayIterator->second.nodeIds[0]);
					Vertex& n2 = _osm.getNodeById(wayIterator->second.nodeIds[1]);

					// Check both nodes are within map limits to prevent artefacts.
					if (!isOutOfBounds(n1.coords) && !isOutOfBounds(n2.coords))
					{
						auto nodes = calculateEndCaps(n1, n2, wayIterator->second.width);

						wayIterator->second.nodeIds.insert(wayIterator->second.nodeIds.begin(), nodes[0]);
						wayIterator->second.nodeIds.insert(wayIterator->second.nodeIds.begin(), n2.id); // Need a repeated node to complete triangle list.
						wayIterator->second.nodeIds.insert(wayIterator->second.nodeIds.begin(), nodes[1]);
					}
				}
			}

			for (uint32_t i = 0; i < (wayIterator->second.nodeIds.size() - 2); ++i)
			{
				const Vertex& node0 = i % 2 == 0 ? _osm.getNodeById(wayIterator->second.nodeIds[i]) : _osm.getNodeById(wayIterator->second.nodeIds[i + 1]);
				const Vertex& node1 = i % 2 == 0 ? _osm.getNodeById(wayIterator->second.nodeIds[i + 1]) : _osm.getNodeById(wayIterator->second.nodeIds[i]);
				const Vertex& node2 = _osm.getNodeById(wayIterator->second.nodeIds[i + 2]);
				convertedRoad.triangulatedIds.emplace_back(std::array<uint64_t, 3>{ node0.id, node1.id, node2.id });
			}
		}
		_osm.convertedRoads[convertedRoad.id] = convertedRoad;
	}
}

/// <summary>Sort the ways into the tiles.</summary>
void NavDataProcess::sortTiles()
{
	uint64_t id = 0;
	bool multiJunct = false;
	// Tile roads
	for (auto&& wayIterator : _osm.convertedRoads)
	{
		auto& way = wayIterator.second;
		for (uint32_t i = 0; i < way.triangulatedIds.size(); ++i)
		{
			if (way.isIntersection)
			{
				if (multiJunct && !way.isRoundabout)
				{
					_osm.intersectionNodes.rbegin()->second.nodes.insert(
						_osm.intersectionNodes.rbegin()->second.nodes.end(), way.triangulatedIds[i].begin(), way.triangulatedIds[i].end());
				}
				else
				{
					IntersectionData data;
					data.nodes.insert(data.nodes.end(), way.triangulatedIds[i].begin(), way.triangulatedIds[i].end());
					data.isBound = false;
					_osm.intersectionNodes[id] = data;
				}
			}

			Vertex node0 = _osm.nodes.find(way.triangulatedIds[i][0])->second;
			Vertex node1 = _osm.nodes.find(way.triangulatedIds[i][1])->second;
			Vertex node2 = _osm.nodes.find(way.triangulatedIds[i][2])->second;

			fillTiles(node0, node1, id, way.tags, WayTypes::Road, 0, false, way.area, way.roadType, way.width, way.isIntersection, way.isRoundabout, way.isFork);

			fillTiles(node1, node2, id, way.tags, WayTypes::Road, 0, false, way.area, way.roadType, way.width, way.isIntersection, way.isRoundabout, way.isFork);

			fillTiles(node2, node0, id, way.tags, WayTypes::Road, 0, false, way.area, way.roadType, way.width, way.isIntersection, way.isRoundabout, way.isFork);
			id++;
			multiJunct = true;
		}
		multiJunct = false;
	}

	for (uint32_t lod = 0; lod < LOD::Count; ++lod)
	{
		// Labels
		for (uint32_t i = 0; i < _osm.labels[lod].size(); ++i) { fillLabelTiles(_osm.labels[lod][i], lod); }

		// Icons
		for (uint32_t i = 0; i < _osm.icons[lod].size(); ++i) { fillIconTiles(_osm.icons[lod][i], lod); }

		// Amenity Labels
		for (uint32_t i = 0; i < _osm.amenityLabels[lod].size(); ++i) { fillAmenityTiles(_osm.amenityLabels[lod][i], lod); }

		// Tile area outlines
		if (!_osm.areaOutlines.empty())
		{
			for (uint32_t i = 0; i < (_osm.areaOutlines.size() - 1); i += 2)
			{
				Vertex currentNode = _osm.nodes.find(_osm.areaOutlines[i])->second;
				Vertex nextNode = _osm.nodes.find(_osm.areaOutlines[i + 1])->second;

				fillTiles(currentNode, nextNode, 0, std::vector<Tag>{}, WayTypes::AreaOutline, 0, true, true);
			}
		}
	}

	// Tile car parking
	id = 0;
	std::vector<Way> innerWays;
	for (auto wayIterator = _osm.parkingWays.begin(); wayIterator != _osm.parkingWays.end(); ++wayIterator)
	{
		for (uint32_t i = 0; i < (wayIterator->second.nodeIds.size() - 1); ++i)
		{
			Vertex currentNode = _osm.nodes.find(wayIterator->second.nodeIds[i])->second;
			Vertex nextNode = _osm.nodes.find(wayIterator->second.nodeIds[i + 1])->second;

			fillTiles(currentNode, nextNode, wayIterator->first, wayIterator->second.tags, WayTypes::PolygonOutline, 0, true);
		}

		if (checkWinding(wayIterator->second.nodeIds) == pvr::PolygonWindingOrder::FrontFaceCW)
			std::reverse(wayIterator->second.nodeIds.begin(), wayIterator->second.nodeIds.end());

		if (wayIterator->second.inner)
		{
			innerWays.emplace_back(wayIterator->second);
			continue;
		}

		std::vector<std::array<uint64_t, 3>> triangles;
		triangulate(wayIterator->second.nodeIds, triangles);

		for (uint32_t i = 0; i < triangles.size(); ++i)
		{
			Vertex node0 = _osm.nodes.find(triangles[i][0])->second;
			Vertex node1 = _osm.nodes.find(triangles[i][1])->second;
			Vertex node2 = _osm.nodes.find(triangles[i][2])->second;

			fillTiles(node0, node1, id, wayIterator->second.tags, WayTypes::Parking);
			fillTiles(node1, node2, id, wayIterator->second.tags, WayTypes::Parking);
			fillTiles(node2, node0, id, wayIterator->second.tags, WayTypes::Parking);
			id++;
		}
	}

	// Tile buildings
	id = 0;
	for (auto wayIterator = _osm.buildWays.begin(); wayIterator != _osm.buildWays.end(); ++wayIterator)
	{
		glm::dvec2 avgPos = glm::dvec2(0, 0);

		for (uint32_t i = 0; i < (wayIterator->second.nodeIds.size() - 1); ++i)
		{
			Vertex currentNode = _osm.nodes.find(wayIterator->second.nodeIds[i])->second;
			Vertex nextNode = _osm.nodes.find(wayIterator->second.nodeIds[i + 1])->second;

			fillTiles(currentNode, nextNode, wayIterator->first, wayIterator->second.tags, WayTypes::PolygonOutline, 0, true);
			avgPos += currentNode.coords;
		}

		if (checkWinding(wayIterator->second.nodeIds) == pvr::PolygonWindingOrder::FrontFaceCW)
			std::reverse(wayIterator->second.nodeIds.begin(), wayIterator->second.nodeIds.end());

		if (wayIterator->second.inner)
		{
			innerWays.emplace_back(wayIterator->second);
			continue;
		}

		std::vector<std::array<uint64_t, 3>> triangles;
		triangulate(wayIterator->second.nodeIds, triangles);

		avgPos /= (wayIterator->second.nodeIds.size() - 1);
		// Deterministic height based on average position
		uint32_t positionHash = pvr::hash32_bytes(&avgPos, sizeof(glm::dvec2));

		double MIN_BUILDING_HEIGHT = 1.0;
		double MAX_BUILDING_HEIGHT = 15.0;
		double BUILDING_HEIGHT_RATIO = .1;

		double buildingHeight =
			(MIN_BUILDING_HEIGHT + (MAX_BUILDING_HEIGHT - MIN_BUILDING_HEIGHT) * (double(positionHash) / double(std::numeric_limits<uint32_t>().max()))) * BUILDING_HEIGHT_RATIO;

		for (uint32_t i = 0; i < triangles.size(); ++i)
		{
			Vertex node0 = _osm.nodes.find(triangles[i][0])->second;
			Vertex node1 = _osm.nodes.find(triangles[i][1])->second;
			Vertex node2 = _osm.nodes.find(triangles[i][2])->second;
			node0.height = node1.height = node2.height = 0.0;

			Vertex _node0 = node0;
			_node0.height = buildingHeight;
			_node0.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node0.id] = _node0;

			Vertex _node1 = node1;
			_node1.height = buildingHeight;
			_node1.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node1.id] = _node1;

			Vertex _node2 = node2;
			_node2.height = buildingHeight;
			_node2.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node2.id] = _node2;

			/******* Cuboid Faces ***********/
			fillTiles(_node0, _node1, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			fillTiles(_node1, _node2, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			fillTiles(_node2, _node0, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			id++;

			node0.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node0.id] = node0;
			node1.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node1.id] = node1;
			node2.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node2.id] = node2;

			_node0.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node0.id] = _node0;
			_node1.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node1.id] = _node1;
			_node2.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node2.id] = _node2;

			fillTiles(node0, _node0, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			fillTiles(_node0, node1, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			fillTiles(node1, node0, id, wayIterator->second.tags, WayTypes::Building);
			id++;

			node0.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node0.id] = node0;
			node1.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node1.id] = node1;
			node2.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node2.id] = node2;

			_node0.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node0.id] = _node0;
			_node1.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node1.id] = _node1;
			_node2.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node2.id] = _node2;

			fillTiles(node1, _node0, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			fillTiles(_node0, _node1, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			fillTiles(_node1, node1, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			id++;

			node0.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node0.id] = node0;
			node1.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node1.id] = node1;
			node2.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node2.id] = node2;

			_node0.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node0.id] = _node0;
			_node1.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node1.id] = _node1;
			_node2.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node2.id] = _node2;

			fillTiles(node2, _node2, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			fillTiles(_node2, node0, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			fillTiles(node0, node2, id, wayIterator->second.tags, WayTypes::Building);
			id++;

			node0.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node0.id] = node0;
			node1.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node1.id] = node1;
			node2.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node2.id] = node2;

			_node0.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node0.id] = _node0;
			_node1.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node1.id] = _node1;
			_node2.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node2.id] = _node2;

			fillTiles(node0, _node2, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			fillTiles(_node2, _node0, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			fillTiles(_node0, node0, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			id++;

			node0.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node0.id] = node0;
			node1.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node1.id] = node1;
			node2.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node2.id] = node2;

			_node0.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node0.id] = _node0;
			_node1.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node1.id] = _node1;
			_node2.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node2.id] = _node2;

			fillTiles(node1, _node2, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			fillTiles(_node2, node2, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			fillTiles(node2, node1, id, wayIterator->second.tags, WayTypes::Building);
			id++;

			node0.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node0.id] = node0;
			node1.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node1.id] = node1;
			node2.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[node2.id] = node2;

			_node0.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node0.id] = _node0;
			_node1.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node1.id] = _node1;
			_node2.id = _osm.nodes.rbegin()->first + 1;
			_osm.nodes[_node2.id] = _node2;

			fillTiles(node1, _node1, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			fillTiles(_node1, _node2, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			fillTiles(_node2, node1, id, wayIterator->second.tags, WayTypes::Building, buildingHeight);
			id++;
		}
	}

	// Tile inner ways
	id = 0;
	for (auto way : innerWays)
	{
		std::vector<std::array<uint64_t, 3>> triangles;
		triangulate(way.nodeIds, triangles);

		for (uint32_t i = 0; i < triangles.size(); ++i)
		{
			Vertex node0 = _osm.nodes.find(triangles[i][0])->second;
			Vertex node1 = _osm.nodes.find(triangles[i][1])->second;
			Vertex node2 = _osm.nodes.find(triangles[i][2])->second;

			fillTiles(node0, node1, id, way.tags, WayTypes::Inner);
			fillTiles(node1, node2, id, way.tags, WayTypes::Inner);
			fillTiles(node2, node0, id, way.tags, WayTypes::Inner);
			id++;
		}
	}
}

/// <summary>Insert a way (or a node ID) into a given array of ways.</summary>
/// <param name="insertIn">The vector of ways in which to insert.</param>
/// <param name="way">The way to insert.</param>
void NavDataProcess::insertWay(std::vector<Way>& insertIn, Way& way)
{
	if ((!insertIn.empty() && (insertIn.rbegin()->id == way.id)))
	{
		insertIn.rbegin()->nodeIds.insert(insertIn.rbegin()->nodeIds.end(), way.nodeIds.begin(), way.nodeIds.end());

		if (way.tileBoundWay) insertIn.rbegin()->tileBoundWay = true;
	}
	else
	{
		insertIn.emplace_back(way);
	}
}

/// <summary>Find the tile the given point belongs to.</summary>
/// <param name="point">Find the tile that this point belongs to.</param>
/// <returns>Return a vector with the tile coordinates as long as the point is within the map bounds.</returns>
glm::ivec2 NavDataProcess::findTile2(glm::dvec2& point) const
{
	glm::uvec2 tileCoords(0, 0);
	glm::dvec2 tmpPoint = point;
	for (uint32_t i = 0; i < _osm.numCols; ++i)
	{
		if (tmpPoint.x <= _osm.tiles[i][0].max.x)
		{
			if ((tmpPoint.x == _osm.tiles[i][0].max.x) && (i != (_osm.numCols - 1))) tmpPoint.x -= 0.0000001; // Move node off of tile border
			tileCoords.x = i;
			break;
		}
	}

	for (uint32_t i = 0; i < _osm.numRows; ++i)
	{
		if (tmpPoint.y <= _osm.tiles[0][i].max.y)
		{
			if ((tmpPoint.y == _osm.tiles[0][i].max.y) && (i != (_osm.numRows - 1))) tmpPoint.y -= 0.0000001; // Move node off of tile border
			tileCoords.y = i;
			break;
		}
	}
	return tileCoords;
}

// The range of angles at which a bend should be tessellated - no need to tessellate almost flat road segments.
static const float lowerThreshold = 15.0f;

/// <summary>Fill the tiles with the way data.</summary>
/// <param name="startNode">Node to start filling from.</param>
/// <param name="endNode">Final node.</param>
/// <param name="wayId">ID of way being filled.</param>
/// <param name="wayTags">Tags of way being filled.</param>
/// <param name="wayType">Type of way being filled.</param>
/// <param name="addEnd">Whether to add endNode.</param>
/// <param name="addStart">Whether to add startNode.</param>
/// <param name="area">If the way is a road and also an area.</param>
void NavDataProcess::fillTiles(Vertex startNode, Vertex endNode, const uint64_t wayId, const std::vector<Tag>& wayTags, WayTypes::WayTypes wayType, double height, bool addEnd,
	bool area, RoadTypes::RoadTypes type, double width, bool isIntersection, bool isRoundabout, bool isFork)
{
	Way startWay;
	glm::uvec2 startTile;
	glm::uvec2 endTile;

	// Check for nodes out of the map bounds
	if (isOutOfBounds(startNode.coords))
	{
		if (isOutOfBounds(endNode.coords))
		{
			if (!findMapIntersect(startNode.coords, endNode.coords))
			{
				if (isIntersection) _osm.intersectionNodes.erase(wayId);
				return;
			}
			else
				endNode.tileBoundNode = true;
		}
		else
		{
			glm::dvec3 result = findIntersect(_osm.bounds.min, _osm.bounds.max, endNode.coords, startNode.coords);
			startNode.coords = glm::dvec2(result.x, result.y);
		}

		startNode.id = _osm.nodes.rbegin()->first + 1;
		startNode.tileBoundNode = true;
		_osm.nodes[startNode.id] = startNode;
	}
	else if (isOutOfBounds(endNode.coords))
	{
		glm::dvec3 result = findIntersect(_osm.bounds.min, _osm.bounds.max, startNode.coords, endNode.coords);
		endNode.coords = glm::dvec2(result.x, result.y);
		endNode.id = _osm.nodes.rbegin()->first + 1;
		endNode.tileBoundNode = true;
		_osm.nodes[endNode.id] = endNode;
	}

	// Find start and end tiles
	startTile = findTile2(startNode.coords);
	endTile = findTile2(endNode.coords);

	startNode.wayIds.emplace_back(wayId);
	endNode.wayIds.emplace_back(wayId);

	// Add start node to tile
	_osm.tiles[startTile.x][startTile.y].nodes[startNode.id] = startNode;
	startWay.nodeIds.emplace_back(startNode.id);
	startWay.id = wayId;
	startWay.tags = wayTags;
	startWay.tileBoundWay = startNode.tileBoundNode;
	startWay.roadType = type;
	startWay.area = area;
	startWay.width = width;
	startWay.isIntersection = isIntersection;
	startWay.isRoundabout = isRoundabout;
	startWay.isFork = isFork;

	// Add start node ID to existing way or create a new way if necessary
	insert(startTile, wayType, &startWay, startNode.id);

	if (isIntersection)
	{
		if (_osm.intersectionNodes.rbegin()->second.junctionWays.size() == 0 ||
			_osm.intersectionNodes.rbegin()->second.junctionWays.rbegin()->first != static_cast<uint32_t>(_osm.tiles[startTile.x][startTile.y].roadWays.size()) - 1)
			_osm.intersectionNodes.rbegin()->second.junctionWays.emplace_back(
				std::pair<uint32_t, glm::uvec2>(static_cast<uint32_t>(_osm.tiles[startTile.x][startTile.y].roadWays.size()) - 1, startTile));
	}

	// Add start node to boundaryNode vector.
	if (wayType == WayTypes::Road && startNode.tileBoundNode)
	{
		BoundaryData data;
		data.index = static_cast<uint32_t>(_osm.tiles[startTile.x][startTile.y].roadWays.size() - 1);
		data.consumed = false;
		_osm.boundaryNodes[startTile.x][startTile.y][wayId] = data;
	}

	// Deal with tile crossings
	glm::uvec2 currentTile = startTile;
	Vertex currentNode = startNode;

	double t_dist = glm::distance(startNode.coords, endNode.coords);
	while (currentTile != endTile)
	{
		Way newWay;
		glm::dvec3 result = findIntersect(_osm.tiles[currentTile.x][currentTile.y].min, _osm.tiles[currentTile.x][currentTile.y].max, currentNode.coords, endNode.coords);

		// Find the node on the tile boundary
		Vertex newNode = Vertex(_osm.nodes.rbegin()->first + 1, glm::dvec2(result.x, result.y), true);

		double weight = glm::distance(startNode.coords, newNode.coords) / t_dist;
		glm::vec2 weightedTexCoord = glm::mix(currentNode.texCoords, endNode.texCoords, weight);

		newNode.texCoords = weightedTexCoord;
		newNode.wayIds.emplace_back(wayId);
		newNode.height = 0.0;

		if (wayType == WayTypes::Building && startNode.height > 0.0 && endNode.height > 0.0)
			newNode.height = height;
		else if (wayType == WayTypes::Building && (!compareReal(startNode.height, 0.0) || !compareReal(endNode.height, 0.0)))
			newNode.height = glm::mix(currentNode.height, endNode.height, weight);

		_osm.nodes[newNode.id] = newNode;
		_osm.tiles[currentTile.x][currentTile.y].nodes[newNode.id] = newNode;

		newWay.nodeIds.emplace_back(newNode.id);
		newWay.id = wayId;
		newWay.tags = wayTags;
		newWay.tileBoundWay = true;
		newWay.roadType = type;
		newWay.area = area;
		newWay.width = width;
		newWay.isIntersection = isIntersection;
		newWay.isRoundabout = isRoundabout;
		newWay.isFork = isFork;

		// Add new node ID to way
		insert(currentTile, wayType, &newWay, newNode.id);

		switch (int32_t(result.z))
		{
		case Sides::Left: currentTile.x -= 1; break;
		case Sides::Top: currentTile.y += 1; break;
		case Sides::Right: currentTile.x += 1; break;
		case Sides::Bottom: currentTile.y -= 1; break;
		case Sides::NoSide: return; // Prevent infinite loop (no intersect point found)
		}

		_osm.tiles[currentTile.x][currentTile.y].nodes[newNode.id] = newNode;

		// Add the new node to the next tile (creating a new way if necessary)
		insert(currentTile, wayType, &newWay, newNode.id);

		if (isIntersection && !_osm.intersectionNodes.rbegin()->second.isBound)
		{
			_osm.intersectionNodes.rbegin()->second.isBound = true;
			_osm.intersectionNodes.rbegin()->second.junctionWays.emplace_back(
				std::pair<uint32_t, glm::uvec2>(static_cast<uint32_t>(_osm.tiles[currentTile.x][currentTile.y].roadWays.size()) - 1, currentTile));
		}

		currentNode = newNode;
	}

	// Add the end node if needed
	if (addEnd || endNode.tileBoundNode)
	{
		Way endWay;

		_osm.tiles[endTile.x][endTile.y].nodes[endNode.id] = endNode;
		_osm.nodes[endNode.id] = endNode;
		endWay.nodeIds.emplace_back(endNode.id);
		endWay.id = wayId;
		endWay.tags = wayTags;
		endWay.tileBoundWay = endNode.tileBoundNode;
		endWay.roadType = type;
		endWay.area = area;
		endWay.width = width;
		endWay.isIntersection = isIntersection;
		endWay.isRoundabout = isRoundabout;
		endWay.isFork = isFork;

		insert(endTile, wayType, &endWay, endNode.id);

		// Add end node to boundaryNode vector.
		if (wayType == WayTypes::Road)
		{
			BoundaryData data;
			data.index = static_cast<uint32_t>(_osm.tiles[endTile.x][endTile.y].roadWays.size() - 1);
			data.consumed = false;
			_osm.boundaryNodes[endTile.x][endTile.y][wayId] = data;
		}
	}
}

/// <summary>Add tile corner points if necessary.</summary>
void NavDataProcess::addCornerPoints(Tile& tile, std::vector<Way>& way)
{
	for (uint32_t i = 0; i < way.size(); i++)
	{
		if (way[i].tileBoundWay)
		{
			std::vector<uint64_t>& nodeIds = way[i].nodeIds;

			if (nodeIds.size() == 2)
			{
				glm::dvec2 point0 = tile.nodes.find(nodeIds[0])->second.coords;
				glm::dvec2 point1 = tile.nodes.find(nodeIds[1])->second.coords;

				Vertex newNode(_osm.nodes.rbegin()->first + 1);
				newNode.coords.x = ((point0.x == tile.min.x) || (point0.x == tile.max.x)) ? point0.x : point1.x;
				newNode.coords.y = ((point0.y == tile.min.y) || (point0.y == tile.max.y)) ? point0.y : point1.y;

				_osm.nodes[newNode.id] = newNode;
				tile.nodes[newNode.id] = newNode;
				nodeIds.emplace_back(newNode.id);
			}
			else
			{
				std::vector<uint64_t> newTriangles;
				bool firstBound = tile.nodes.find(nodeIds[0])->second.tileBoundNode;
				bool secBound = tile.nodes.find(nodeIds[1])->second.tileBoundNode;

				for (uint32_t j = 0; j < nodeIds.size(); ++j)
				{
					Vertex currentNode = tile.nodes.find(nodeIds[j])->second;
					Vertex nextNode = (j < (nodeIds.size() - 1)) ? tile.nodes.find(nodeIds[j + 1])->second : tile.nodes.find(nodeIds[0])->second;

					if (currentNode.tileBoundNode && (((float(j) / 2) != (j / 2)) || (nodeIds.size() == 3) || (!firstBound && !secBound)) &&
						((((currentNode.coords.x == tile.min.x) || (currentNode.coords.x == tile.max.x)) && ((nextNode.coords.y == tile.min.y) || (nextNode.coords.y == tile.max.y))) ||
							(((currentNode.coords.y == tile.min.y) || (currentNode.coords.y == tile.max.y)) && ((nextNode.coords.x == tile.min.x) || (nextNode.coords.x == tile.max.x)))))
					{
						glm::dvec2 newPoint;
						newPoint.x = ((currentNode.coords.x == tile.min.x) || (currentNode.coords.x == tile.max.x)) ? currentNode.coords.x : nextNode.coords.x;
						newPoint.y = ((currentNode.coords.y == tile.min.y) || (currentNode.coords.y == tile.max.y)) ? currentNode.coords.y : nextNode.coords.y;

						Vertex newNode(_osm.nodes.rbegin()->first + 1, newPoint);

						_osm.nodes[newNode.id] = newNode;
						tile.nodes[newNode.id] = newNode;
						newTriangles.emplace_back(currentNode.id);
						newTriangles.emplace_back(newNode.id);
						newTriangles.emplace_back(nextNode.id);
					}
				}

				if ((float(nodeIds.size()) / 3) != (nodeIds.size() / 3))
				{
					std::vector<uint64_t> newNodeIds;
					for (uint32_t j = 1; j < (nodeIds.size() - 1); ++j)
					{
						newNodeIds.emplace_back(nodeIds[0]);
						newNodeIds.emplace_back(nodeIds[j]);
						newNodeIds.emplace_back(nodeIds[j + 1]);
					}
					nodeIds = newNodeIds;
				}

				if (!newTriangles.empty()) nodeIds.insert(nodeIds.end(), newTriangles.begin(), newTriangles.end());
			}
		}
	}
}

/// <summary>Increases the complexity of the geometry to smooth out harsh corners - the min/max angle is configurable.
/// The algorithm iterates over the oldNodeIDs vector and selects 3 nodes (start, control, end) on each pass,
/// a Bezier curve is then generated with x number of steps. Note that the algorithm may return the exact same nodes if the road is fairly straight.</summary>
/// <param name="oldNodeIDs" >Node IDs of the incoming road line strip.</param>
/// <returns>A vector of old + newly generated node IDs - line strip.</returns>
std::vector<uint64_t> NavDataProcess::tessellate(const std::vector<uint64_t>& oldNodeIDs, uint32_t& index, bool splitWay)
{
	std::vector<uint64_t> newIds;
	/*How detailed the curve will be. A smaller number will increase the geometry's complexity.
	(This could negatively impact performance)*/
	static const float stepValue = 0.1f;
	// The range of angles at which a bend should be tessellated - no need to tessellate almost flat road segments.
	static const float angleLowerThreshold = 15.0f;
	static const float angleUpperThreshold = 165.0f;

	glm::dvec2 offset = glm::dvec2(0, 0);
	glm::dvec2 lastPointOnCurve = glm::dvec2(0, 0);
	bool middleNodeAdded = false;
	uint32_t middleIndex = static_cast<uint32_t>(oldNodeIDs.size() / 2);

	for (uint32_t i = 1; i < (oldNodeIDs.size() - 1); ++i)
	{
		Vertex node0 = _osm.nodes.find(oldNodeIDs[i - 1])->second;
		Vertex node1 = _osm.nodes.find(oldNodeIDs[i])->second;
		Vertex node2 = _osm.nodes.find(oldNodeIDs[i + 1])->second;

		glm::dvec2 v1 = (middleNodeAdded ? lastPointOnCurve : node0.coords) - node1.coords;
		glm::dvec2 v2 = node2.coords - node1.coords /*(middleNodeAdded ? (node1.coords + offset) : node1.coords)*/;

		// Calculate angle between road segments (v1, v2).
		double angle = glm::degrees(glm::acos(glm::dot(glm::normalize(v1), glm::normalize(v2))));

		// Check angle is within thresholds and node1 is not outside of map boundary and it is not an intersection (to prevent artefacts).
		if (angle > angleLowerThreshold && angle < angleUpperThreshold && !isOutOfBounds(node1.coords) && (node1.wayIds.size() == 1 || splitWay))
		{
			middleNodeAdded = false;
			lastPointOnCurve = glm::dvec2(0, 0);

			// Add first node if vector is empty.
			if (newIds.size() == 0) newIds.emplace_back(node0.id);

			// Compute projection distance for each road segment.
			double projLenA = glm::length(v1) / 10.0;
			double projLenB = glm::length(v2) / 10.0;

			// Compute the start and end locations for the Bezier curve.
			glm::dvec2 startPos = node1.coords + (glm::normalize(v1) * projLenA);
			glm::dvec2 endPos = node1.coords + (glm::normalize(v2) * projLenB);

			for (float interpolant = 0.0; interpolant <= 1.0; interpolant += stepValue)
			{
				// Calculate new point on the curve.
				glm::dvec2 a = glm::mix(startPos, node1.coords, interpolant);
				glm::dvec2 b = glm::mix(node1.coords, endPos, interpolant);
				glm::dvec2 newCoords = glm::mix(a, b, interpolant);

				Vertex newNode;
				/*Copy the control node into the newIds when we reach the centre
				of the curve (approx) only updating its position (to preserve intersections).*/
				if (interpolant >= 0.5 && !middleNodeAdded)
				{
					if (i == middleIndex) index = static_cast<uint32_t>(newIds.size());

					newNode = node1;
					middleNodeAdded = true;
					offset = newCoords - node1.coords;
				}
				else // Create a new node
					newNode.id = _osm.nodes.rbegin()->first + 1;

				newNode.coords = lastPointOnCurve = newCoords;
				_osm.nodes[newNode.id] = newNode;
				newIds.emplace_back(newNode.id);
			}
			// Add last node if we have reached the end of oldNodes.
			if (i == oldNodeIDs.size() - 2) newIds.emplace_back(node2.id);
		}
		else
		{
			if (newIds.size() == 0) newIds.emplace_back(node0.id);

			if (i == middleIndex) index = static_cast<uint32_t>(newIds.size());

			newIds.emplace_back(node1.id);

			if (i == oldNodeIDs.size() - 2) newIds.emplace_back(node2.id);

			middleNodeAdded = false;
		}
	}
	return newIds;
}

/// <summary>Triangulates a road line strip into a triangle strip.</summary>
/// <param name"nodeIds"> Node IDs of a road line strip.</param>
/// <returns>A vector of node IDs representing triangle strips.</returns>
std::vector<uint64_t> NavDataProcess::triangulateRoad(const std::vector<uint64_t>& nodeIds, const double width)
{
	std::vector<uint64_t> newNodeIds;

	if (nodeIds.size() == 2)
	{
		uint64_t id = _osm.nodes.rbegin()->first + 1;
		Vertex node0 = _osm.nodes.find(nodeIds[0])->second;
		Vertex node1 = _osm.nodes.find(nodeIds[1])->second;

		// Find coordinates of new points
		std::array<glm::dvec2, 2> firstPerps = findPerpendicularPoints(node0.coords, node1.coords, width, 1);
		std::array<glm::dvec2, 2> secPerps = findPerpendicularPoints(node0.coords, node1.coords, width, 2);

		// Create new nodes
		Vertex newNode0(id, firstPerps[0], false, glm::vec2(-0.05f, 0.245f));
		Vertex newNode1(++id, firstPerps[1], false, glm::vec2(0.55f, 0.245f));
		Vertex newNode2(++id, secPerps[0], false, glm::vec2(-0.05f, 0.245f));
		Vertex newNode3(++id, secPerps[1], false, glm::vec2(0.55f, 0.245f));
		_osm.nodes[newNode0.id] = newNode0;
		_osm.nodes[newNode1.id] = newNode1;
		_osm.nodes[newNode2.id] = newNode2;
		_osm.nodes[newNode3.id] = newNode3;

		// Create triangles
		newNodeIds.emplace_back(newNode0.id);
		newNodeIds.emplace_back(newNode1.id);
		newNodeIds.emplace_back(newNode2.id);
		newNodeIds.emplace_back(newNode3.id);
	}
	else
	{
		for (uint32_t i = 1; i < (nodeIds.size() - 1); ++i)
		{
			uint64_t id = _osm.nodes.rbegin()->first + 1;
			Vertex node0 = _osm.nodes.find(nodeIds[i - 1])->second;
			Vertex node1 = _osm.nodes.find(nodeIds[i])->second;
			Vertex node2 = _osm.nodes.find(nodeIds[i + 1])->second;

			// Find coordinates of new points for the middle node
			std::array<glm::dvec2, 2> secPerps = findPerpendicularPoints(node0.coords, node1.coords, node2.coords, width);

			// If this is the first entry we also need to perform for the first node
			if (i == 1)
			{
				std::array<glm::dvec2, 2> firstPerps = findPerpendicularPoints(node0.coords, node1.coords, width, 1);
				Vertex newNode0(id, firstPerps[0], false, glm::vec2(-0.05f, 0.245f));
				Vertex newNode1(++id, firstPerps[1], false, glm::vec2(0.55f, 0.245f));
				_osm.nodes[newNode0.id] = newNode0;
				_osm.nodes[newNode1.id] = newNode1;

				newNodeIds.emplace_back(newNode0.id);
				newNodeIds.emplace_back(newNode1.id);
			}

			Vertex newNode2(++id, secPerps[0], false, glm::vec2(-0.05f, 0.245f));
			Vertex newNode3(++id, secPerps[1], false, glm::vec2(0.55f, 0.245f));

			_osm.nodes[newNode2.id] = newNode2;
			_osm.nodes[newNode3.id] = newNode3;

			newNodeIds.emplace_back(newNode2.id);
			newNodeIds.emplace_back(newNode3.id);

			// If this is the last entry we also need to perform for the third/last node
			if (i == (nodeIds.size() - 2))
			{
				std::array<glm::dvec2, 2> thirdPerps;
				thirdPerps = findPerpendicularPoints(node1.coords, node2.coords, width, 2);

				Vertex newNode4(++id, thirdPerps[0], false, glm::vec2(-0.05f, 0.245f));
				Vertex newNode5(++id, thirdPerps[1], false, glm::vec2(0.55f, 0.245f));
				_osm.nodes[newNode4.id] = newNode4;
				_osm.nodes[newNode5.id] = newNode5;

				newNodeIds.emplace_back(newNode4.id);
				newNodeIds.emplace_back(newNode5.id);
			}
		}
	}
	return newNodeIds;
}

/// <summary>Calculates the texture co-ordinates for nodes that are on the outer boundary of the map.</summary>
void NavDataProcess::calculateMapBoundaryTexCoords()
{
	// Iterate through 2D array, each 'bin' holds a map which contains the boundary node data.
	for (uint32_t i = 0; i < _osm.boundaryNodes.size(); ++i)
	{
		for (uint32_t j = 0; j < _osm.boundaryNodes[i].size(); ++j)
		{
			// Outer iterator.
			for (auto outerItr = _osm.boundaryNodes[i][j].begin(); outerItr != _osm.boundaryNodes[i][j].end(); ++outerItr)
			{
				// The current way to calculate tex co-ordinates for.
				Way* currentWay = &_osm.tiles[i][j].roadWays[outerItr->second.index];

				// Check if way is valid and has not been 'consumed' i.e. the data has not been used before.
				if (!currentWay || outerItr->second.consumed) continue;

				// This way has now been used - 'consumed'
				outerItr->second.consumed = true;
				// Hold nodes that are definitely part of this road segment which is on the map border.
				std::vector<Vertex*> foundNodes;
				// Holds tentative nodes which might be part of this road segment.
				std::vector<Vertex*> rejectedNodes;

				// Iterate over node ids held by the current way.
				for (uint32_t k = 0; k < currentWay->nodeIds.size(); ++k)
				{
					Vertex* nextNode = &_osm.tiles[i][j].nodes.find(currentWay->nodeIds[k])->second;

					// Check if node is on a tile border.
					if (!nextNode->tileBoundNode) continue;

					// Check if node is on the exterior edge of the map.
					if (compareReal(nextNode->coords.x, _osm.bounds.min.x) || compareReal(nextNode->coords.x, _osm.bounds.max.x) ||
						compareReal(nextNode->coords.y, _osm.bounds.min.y) || compareReal(nextNode->coords.y, _osm.bounds.max.y))
						foundNodes.emplace_back(nextNode);
				}

				if (foundNodes.size() == 0)
				{
					Log(LogLevel::Error, "Could not calculate texture co-ordinates for a bounding node.");
					continue;
				}

				// Find the static axis.
				bool xEqual = (compareReal(foundNodes[0]->coords.x, _osm.bounds.min.x) || compareReal(foundNodes[0]->coords.x, _osm.bounds.max.x));
				bool yEqual = (compareReal(foundNodes[0]->coords.y, _osm.bounds.min.y) || compareReal(foundNodes[0]->coords.y, _osm.bounds.max.y));

				// Sort along x or y axis depending on which axis is fixed.
				if (xEqual)
					std::sort(foundNodes.begin(), foundNodes.end(), NavDataProcess::compareY);
				else if (yEqual)
					std::sort(foundNodes.begin(), foundNodes.end(), NavDataProcess::compareX);

				double roadWidth = currentWay->width;

				// Inner iterator.
				for (auto innerItr = _osm.boundaryNodes[i][j].begin(); innerItr != _osm.boundaryNodes[i][j].end(); ++innerItr)
				{
					// The next way to check against the current way for nodes which are part of the same road as the current way.
					Way* nextWay = &_osm.tiles[i][j].roadWays[innerItr->second.index];

					// Check if way is valid and not already consumed.
					if (nextWay->id == currentWay->id || nextWay->roadType != currentWay->roadType || innerItr->second.consumed) continue;

					// Roads are commonly made from two ways with ids which are 'next to' each other.
					bool nextTo = glm::abs(std::int64_t(currentWay->id) - std::int64_t(nextWay->id)) < 2;

					// Iterate through next way nodes.
					for (uint32_t k = 0; k < nextWay->nodeIds.size(); ++k)
					{
						Vertex* nextNode = &_osm.tiles[i][j].nodes.find(nextWay->nodeIds[k])->second;

						// Check that the node is on boundary.
						if (!nextNode->tileBoundNode) continue;

						// Check the fixed axis against the current ways fixed axis.
						if (xEqual && (compareReal(nextNode->coords.x, _osm.bounds.min.x) || compareReal(nextNode->coords.x, _osm.bounds.max.x)))
						{
							if (nextNode->coords.y > foundNodes.back()->coords.y)
							{
								// Check if the way has an id 'close' to the current way.
								if (nextTo)
								{
									// Place node at the end of the vector.
									foundNodes.emplace_back(nextNode);
									innerItr->second.consumed = true;
								}
								// Check for absolute distance between nodes and compare to road width.
								else if (glm::abs(foundNodes[0]->coords.y - nextNode->coords.y) < roadWidth)
								{
									// Place node at the end of the vector.
									foundNodes.emplace_back(nextNode);
									innerItr->second.consumed = true;
								}
								else
								{
									// Add to the rejected node array - might be used for later computation.
									rejectedNodes.emplace_back(nextNode);
								}
							}
							else if (nextNode->coords.y < foundNodes[0]->coords.y)
							{
								// Check if the way has an id 'close' to the current way.
								if (nextTo)
								{
									// Place node at the beginning of vector to maintain ordering.
									foundNodes.insert(foundNodes.begin(), nextNode);
									innerItr->second.consumed = true;
								}
								else if (glm::abs(foundNodes.back()->coords.y - nextNode->coords.y) < roadWidth)
								{
									foundNodes.insert(foundNodes.begin(), nextNode);
									innerItr->second.consumed = true;
								}
								else
								{
									// Add to the rejected node array - might be used for later computation.
									rejectedNodes.emplace_back(nextNode);
								}
							}
							else
							{
								// Place node in the 'middle' doesn't matter about ordering of 'middle' elements.
								foundNodes.insert(foundNodes.begin() + 1, nextNode);
								innerItr->second.consumed = true;
							}
						}
						else if (yEqual && (compareReal(nextNode->coords.y, _osm.bounds.min.y) || compareReal(nextNode->coords.y, _osm.bounds.max.y)))
						{
							if (nextNode->coords.x > foundNodes.back()->coords.x)
							{
								// Check if the way has an id 'close' to the current way.
								if (nextTo)
								{
									// Place node at the end of the vector.
									foundNodes.emplace_back(nextNode);
									innerItr->second.consumed = true;
								}
								else if (glm::abs(foundNodes[0]->coords.x - nextNode->coords.x) < roadWidth)
								{
									// Place node at the end of the vector.
									foundNodes.emplace_back(nextNode);
									innerItr->second.consumed = true;
								}
								else
								{
									// Add to the rejected node array - might be used for later computation.
									rejectedNodes.emplace_back(nextNode);
								}
							}
							else if (nextNode->coords.x < foundNodes[0]->coords.x)
							{
								// Check if the way has an id 'close' to the current way.
								if (nextTo)
								{
									// Place node at the beginning of vector to maintain ordering.
									foundNodes.insert(foundNodes.begin(), nextNode);
									innerItr->second.consumed = true;
								}
								else if (glm::abs(foundNodes.back()->coords.x - nextNode->coords.x) < roadWidth)
								{
									// Place node at the beginning of vector.
									foundNodes.insert(foundNodes.begin(), nextNode);
									innerItr->second.consumed = true;
								}
								else
								{
									// Add to the rejected node array - might be used for later computation.
									rejectedNodes.emplace_back(nextNode);
								}
							}
							else
							{
								// Place node in the 'middle' doesn't matter about ordering of 'middle' elements.
								foundNodes.insert(foundNodes.begin() + 1, nextNode);
								innerItr->second.consumed = true;
							}
						}
					}
				}

				// Check we have enough nodes to calculate the texture co-ordinates.
				if (foundNodes.size() < 2)
				{
					Log(LogLevel::Error, "Could not calculate texture co-ordinates for a bounding node.");
					continue;
				}

				// Get the total width of the road.
				double t_dist = glm::distance(foundNodes[0]->coords, foundNodes.back()->coords);

				/*Check the road is 'wide' enough, if not then select the closest
				node to the current edges of the road and add it to the found nodes array.*/
				if (t_dist < roadWidth && rejectedNodes.size() > 0)
				{
					// Delta distance.
					double delta = glm::abs(t_dist - roadWidth);
					double currentClosest = std::numeric_limits<double>::max();
					Vertex* node = nullptr;

					if (xEqual)
					{
						// Move co-ordinate by delta along the Y axis.
						double p1 = foundNodes[0]->coords.y - delta;
						double p2 = foundNodes.back()->coords.y + delta;

						/*Iterate over rejected nodes array, finding the node which
						resides closest to the current edges of the road.*/
						for (uint32_t k = 0; k < rejectedNodes.size(); ++k)
						{
							double d1 = glm::abs(rejectedNodes[k]->coords.y - p1);
							double d2 = glm::abs(rejectedNodes[k]->coords.y - p2);

							if (d1 < currentClosest || d2 < currentClosest)
							{
								currentClosest = glm::min(d1, d2);
								node = rejectedNodes[k];
							}
						}

						// Determine whether to insert the new node at the beginning or end of the array to preserve partial ordering.
						if (node->coords.y <= foundNodes[0]->coords.y)
							foundNodes.insert(foundNodes.begin(), node);
						else
							foundNodes.emplace_back(node);
					}
					else if (yEqual)
					{
						// Move co-ordinate by delta along the Y axis.
						double p1 = foundNodes[0]->coords.x - delta;
						double p2 = foundNodes.back()->coords.x + delta;

						/*Iterate over rejected nodes array, finding the node which
						resides closest to the current edges of the road.*/
						for (uint32_t k = 0; k < rejectedNodes.size(); ++k)
						{
							double d1 = glm::abs(rejectedNodes[k]->coords.x - p1);
							double d2 = glm::abs(rejectedNodes[k]->coords.x - p2);

							if (d1 < currentClosest || d2 < currentClosest)
							{
								currentClosest = glm::min(d1, d2);
								node = rejectedNodes[k];
							}
						}

						// Determine whether to insert the new node at the beginning or end of the array to preserve partial ordering.
						if (node->coords.x <= foundNodes[0]->coords.x)
							foundNodes.insert(foundNodes.begin(), node);
						else
							foundNodes.emplace_back(node);
					}
					// Re-calculate distance.
					t_dist = glm::distance(foundNodes[0]->coords, foundNodes.back()->coords);
				}

				// Remove any duplicate nodes.
				foundNodes.erase(std::unique(foundNodes.begin(), foundNodes.end(), NavDataProcess::compareID), foundNodes.end());

				// If necessary correct the 'road edge' nodes texture co-ordinates.
				if (compareReal(foundNodes[0]->texCoords.x, foundNodes.back()->texCoords.x))
				{
					if (compareReal(foundNodes[0]->texCoords.x, -0.05f))
						foundNodes.back()->texCoords = glm::vec2(0.55f, 0.245f);
					else
						foundNodes.back()->texCoords = glm::vec2(-0.05f, 0.245f);
				}

				float lhs = foundNodes[0]->texCoords.x;
				float rhs = foundNodes.back()->texCoords.x;

				// Iterate over the 'middle' nodes and calculate texture co-ordinates based on linear interpolation.
				for (uint32_t k = 1; k < foundNodes.size() - 1; ++k)
				{
					// Partial distance
					double p_dist = glm::distance(foundNodes[0]->coords, foundNodes[k]->coords);
					// How far along the edge is the 'middle' node.
					double weight = p_dist / t_dist;

					// Lerp
					float u = glm::mix(lhs, rhs, weight);
					foundNodes[k]->texCoords = glm::vec2(u, 0.245f);
				}
			}
		}
	}
}

/// <summary>Calculates the texture co-ordinates for nodes that make up a junction i.e. the connecting points between one or more roads.</summary>
void NavDataProcess::calculateJunctionTexCoords()
{
	// Iterate over intersection nodes and calculate appropriate texture co-ordinates for the junction.
	for (std::map<uint64_t, IntersectionData>::iterator itr = _osm.intersectionNodes.begin(); itr != _osm.intersectionNodes.end(); ++itr)
	{
		std::map<uint64_t, std::pair<Vertex*, Vertex*>> uniqueFoundNodes;
		std::vector<std::pair<glm::uvec2, Way*>> junctionWays;
		std::map<uint64_t, Way*> foundWays;

		bool isRoundabout = false;
		bool isFork = false;

		// Iterate over all ways that represent the junction - usually just one, unless the junction is a crossroad or on a tile boundary.
		for (uint32_t i = 0; i < itr->second.junctionWays.size(); ++i)
		{
			glm::uvec2 currentTile = itr->second.junctionWays[i].second;

			// Check way index is within the array limits.
			if (itr->second.junctionWays[i].first > _osm.tiles[currentTile.x][currentTile.y].roadWays.size()) continue;

			// Get reference to way using way index.
			Way* way = &_osm.tiles[currentTile.x][currentTile.y].roadWays[itr->second.junctionWays[i].first];
			junctionWays.emplace_back(std::pair<glm::uvec2, Way*>(currentTile, way));
			isRoundabout = way->isRoundabout;
			isFork = way->isFork;

			// Iterate over nodes that make up the junction - usually 3 unless the junction is a crossroad or on a tile boundary.
			for (uint32_t j = 0; j < itr->second.nodes.size(); ++j)
			{
				Tile& myTile = _osm.tiles[currentTile.x][currentTile.y];

				std::map<uint64_t, Vertex>::iterator it = myTile.nodes.find(itr->second.nodes[j]);
				if (it == myTile.nodes.end()) { continue; }
				Vertex* currentNode = &it->second;

				bool done = false;
				// Iterate over road ways in the tile that the junction resides in and find any ways which contain nodes
				// that are part of the junction, also find any nodes which share the same co-ordinates as the nodes that define the junction.
				for (Way& w : _osm.tiles[currentTile.x][currentTile.y].roadWays)
				{
					for (uint32_t k = 0; k < w.nodeIds.size(); ++k)
					{
						Vertex* next = &_osm.tiles[currentTile.x][currentTile.y].nodes.find(w.nodeIds[k])->second;

						if (currentNode->id == next->id && w.id != itr->first) foundWays[w.id] = &w;

						if (!done && currentNode->id != next->id && compareReal(next->coords.x, currentNode->coords.x) && compareReal(next->coords.y, currentNode->coords.y))
						{
							uniqueFoundNodes[currentNode->id] = std::pair<Vertex*, Vertex*>(currentNode, next);
							done = true;
						}
					}
				}
			}
		}

		// Check there are enough nodes and the 'way' is valid before continuing.
		if (uniqueFoundNodes.size() > 2)
		{
			std::vector<std::pair<Vertex*, Vertex*>> foundNodes;
			// Move nodes into vector for simpler/faster indexing.
			for (auto it = uniqueFoundNodes.begin(); it != uniqueFoundNodes.end(); ++it) foundNodes.emplace_back(it->second);

			// Check if list of nodes are clockwise wound.
			if (checkWinding(std::vector<glm::dvec2>() = { foundNodes[0].first->coords, foundNodes[1].first->coords, foundNodes[2].first->coords }) == pvr::PolygonWindingOrder::FrontFaceCW)
				std::reverse(foundNodes.begin(), foundNodes.end());

			// Indices for a triangle that defines a junction (excluding crossroad).
			static const std::vector<std::vector<uint32_t>> indices = { std::vector<uint32_t>() = { 0, 2, 1 }, std::vector<uint32_t>() = { 1, 0, 2 },
				std::vector<uint32_t>() = { 2, 1, 0 } };

			static const std::vector<uint32_t> remappedIndices = { 1, 2, 0 };
			uint32_t currentIndex = 0;

			// Determine the correct indexing for 'found nodes' in order to correctly build the junction / calculate texture co-ordinates.
			if (isRoundabout) // Handle roundabouts.
				currentIndex = calculateRoundaboutTexCoordIndices(foundWays, foundNodes);
			else if (foundNodes.size() == 4) // Handle crossroads.
			{
				calculateCrossRoadJunctionTexCoords(foundNodes, junctionWays);
				continue;
			}
			else // Handle T-junctions
				currentIndex = calculateT_JunctionTexCoordIndices(foundWays, foundNodes, junctionWays[0].second);

			if (isFork && !isRoundabout) currentIndex = remappedIndices[currentIndex];

			uint32_t index_1 = indices[currentIndex][0];
			uint32_t index_2 = indices[currentIndex][1];
			uint32_t index_3 = indices[currentIndex][2];

			bool texCoordFlippedEdgeCase = !compareReal(foundNodes[index_3].second->texCoords.x, foundNodes[index_3].first->texCoords.x);
			bool roundAboutEdgeCase1 = isRoundabout && compareReal(foundNodes[index_1].first->texCoords.x, foundNodes[index_3].first->texCoords.x);
			bool roundAboutEdgeCase2 = isRoundabout && compareReal(foundNodes[index_2].first->texCoords.x, foundNodes[index_3].first->texCoords.x);

			for (uint32_t i = 0; i < junctionWays.size(); ++i)
			{
				glm::uvec2 currentTile = junctionWays[i].first;
				Way* way = junctionWays[i].second;

				// Clear existing nodes from way to prevent artefacts.
				way->nodeIds.clear();

				Vertex newNode0 = *foundNodes[index_1].first;
				newNode0.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;
				_osm.tiles[currentTile.x][currentTile.y].nodes[newNode0.id] = newNode0;

				Vertex newNode1 = *foundNodes[index_3].first;
				newNode1.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;
				_osm.tiles[currentTile.x][currentTile.y].nodes[newNode1.id] = newNode1;

				Vertex newNode2 = *foundNodes[index_2].first;
				newNode2.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;
				newNode2.texCoords = foundNodes[index_2].second->texCoords;
				_osm.tiles[currentTile.x][currentTile.y].nodes[newNode2.id] = newNode2;

				// Extra point used to cover gaps and prevent artefacts in junction.
				Vertex newNode3 = newNode2;
				newNode3.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;
				newNode3.coords = calculateMidPoint(foundNodes[index_2].first->coords, foundNodes[index_1].first->coords, foundNodes[index_3].first->coords);
				newNode3.texCoords = glm::vec2(glm::mix(-0.05f, 0.55f, 0.5f), 0.245f);
				_osm.tiles[currentTile.x][currentTile.y].nodes[newNode3.id] = newNode3;

				// Insert new nodes into way to create new triangles.
				// A
				way->nodeIds.emplace_back(newNode0.id);
				way->nodeIds.emplace_back(newNode1.id);
				way->nodeIds.emplace_back(newNode3.id);

				// B
				way->nodeIds.emplace_back(newNode3.id);
				way->nodeIds.emplace_back(newNode1.id);
				way->nodeIds.emplace_back(newNode2.id);

				// Extra triangle needed to cover road line through intersection - check which edge of the intersection has different tex coord's to prevent artefacts.
				if (!compareReal(foundNodes[index_1].first->texCoords.x, foundNodes[index_1].second->texCoords.x) || roundAboutEdgeCase1 ||
					(texCoordFlippedEdgeCase && compareReal(foundNodes[index_3].first->texCoords.x, foundNodes[index_1].second->texCoords.x)))
				{
					Vertex newNode = newNode0;
					newNode.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;

					if (texCoordFlippedEdgeCase || roundAboutEdgeCase1)
						newNode.texCoords = foundNodes[index_2].second->texCoords;
					else
						newNode.texCoords = foundNodes[index_1].second->texCoords;

					_osm.tiles[currentTile.x][currentTile.y].nodes[newNode.id] = newNode;

					// A-A
					way->nodeIds.emplace_back(newNode.id);
					way->nodeIds.emplace_back(newNode1.id);
					way->nodeIds.emplace_back(newNode3.id);
				}
				else if (!compareReal(foundNodes[index_2].first->texCoords.x, foundNodes[index_2].second->texCoords.x) || roundAboutEdgeCase2 ||
					(texCoordFlippedEdgeCase && compareReal(foundNodes[index_3].first->texCoords.x, foundNodes[index_2].first->texCoords.x)))
				{
					Vertex newNode = newNode2;
					newNode.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;

					if (texCoordFlippedEdgeCase || roundAboutEdgeCase2)
						newNode.texCoords = foundNodes[index_1].first->texCoords;
					else
						newNode.texCoords = foundNodes[index_2].first->texCoords;

					_osm.tiles[currentTile.x][currentTile.y].nodes[newNode.id] = newNode;

					// B-B
					way->nodeIds.emplace_back(newNode3.id);
					way->nodeIds.emplace_back(newNode1.id);
					way->nodeIds.emplace_back(newNode.id);
				}
				else
				{
					Vertex newNode = newNode2;
					newNode.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;

					if (compareReal(newNode.texCoords.x, -0.05f))
						_osm.tiles[currentTile.x][currentTile.y].nodes[newNode2.id].texCoords = glm::vec2(0.55f, 0.245f);
					else
						_osm.tiles[currentTile.x][currentTile.y].nodes[newNode2.id].texCoords = glm::vec2(-0.05f, 0.245f);

					_osm.tiles[currentTile.x][currentTile.y].nodes[newNode.id] = newNode;

					way->nodeIds.emplace_back(newNode3.id);
					way->nodeIds.emplace_back(newNode1.id);
					way->nodeIds.emplace_back(newNode.id);
				}

				// C
				way->nodeIds.emplace_back(newNode0.id);
				way->nodeIds.emplace_back(newNode3.id);
				way->nodeIds.emplace_back(newNode2.id);
			}
		}
	}
}

/// <summary>Calculates the index into the indices array for a junction which is a roundabout, this index is determined by finding the two nodes
/// in 'foundNodes' which share the same co-ordinates as two nodes which are not part of the junction i.e. the connecting road way.</summary>
/// <param name="foundWays">Relevant ways (i.e. ways holding nodes that are connected to the junction) generated by the calling function.</param>
/// <param name="foundNodes">Relevant nodes (i.e. the nodes that make up the junction) generated by the calling function.</param>
void NavDataProcess::calculateCrossRoadJunctionTexCoords(std::vector<std::pair<Vertex*, Vertex*>>& foundNodes, std::vector<std::pair<glm::uvec2, Way*>>& junctionWays)
{
	// Store array of indices of nodes and the vector that is created between the two nodes.
	std::vector<std::pair<std::vector<uint32_t>, glm::dvec2>> vectors;

	// Collect vectors between pairs of points in found nodes.
	for (uint32_t i = 0; i < foundNodes.size(); ++i)
	{
		for (uint32_t j = i + 1; j < foundNodes.size(); ++j)
		{
			vectors.emplace_back(
				std::pair<std::vector<uint32_t>, glm::dvec2>(std::vector<uint32_t>() = { j, i }, glm::normalize(foundNodes[i].first->coords - foundNodes[j].first->coords)));
		}
	}

	double currentClosestParallel = std::numeric_limits<double>::max();
	std::vector<uint32_t> indices;

	/*Run through calculated vectors and find the two closest to parallel,
	this finds the correct order of indices for the 'quad' that makes up the junction for a crossroad. */
	for (uint32_t i = 0; i < vectors.size(); ++i)
	{
		for (uint32_t j = i + 1; j < vectors.size(); ++j)
		{
			double r1 = glm::dot(vectors[i].second, vectors[j].second);
			double r2 = 1.0 - glm::abs(r1);

			if (r2 < currentClosestParallel)
			{
				currentClosestParallel = r2;
				indices.clear();
				indices.insert(indices.end(), vectors[i].first.begin(), vectors[i].first.end());
				indices.insert(indices.end(), vectors[j].first.begin(), vectors[j].first.end());

				// Vectors pointing in the opposite direction to each other so swap the indices to prevent holes in the junction.
				if (r1 < 0.0) std::swap(indices[0], indices[1]);
			}
		}
	}

	// Calculate middle node co-ordinates.
	glm::dvec2 v1 = foundNodes[indices[0]].first->coords - foundNodes[indices[3]].first->coords;
	double len = glm::length(v1);
	v1 /= len;

	for (uint32_t i = 0; i < junctionWays.size(); ++i)
	{
		glm::uvec2 currentTile = junctionWays[i].first;
		Way* way = junctionWays[i].second;

		way->nodeIds.clear();

		// Mid point node.
		Vertex newNode;
		newNode.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;
		newNode.coords = foundNodes[indices[0]].first->coords - (v1 * (len / 2.0));
		newNode.texCoords = glm::vec2(glm::mix(-0.05f, 0.55f, 0.5f), 0.245f);
		newNode.height = 0.000075f;
		_osm.tiles[currentTile.x][currentTile.y].nodes[newNode.id] = newNode;

		/* Nodes to create extra triangles for the junction. */
		Vertex newNode0 = *foundNodes[indices[0]].first;
		newNode0.texCoords = glm::vec2(-0.05f, 0.245f);
		newNode0.height = 0.00005f;
		newNode0.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;
		_osm.tiles[currentTile.x][currentTile.y].nodes[newNode0.id] = newNode0;

		Vertex newNode1 = *foundNodes[indices[1]].first;
		newNode1.texCoords = glm::vec2(-0.05f, 0.245f);
		newNode1.height = 0.000075f;
		newNode1.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;
		_osm.tiles[currentTile.x][currentTile.y].nodes[newNode1.id] = newNode1;

		Vertex newNode2 = *foundNodes[indices[2]].first;
		newNode2.texCoords = glm::vec2(0.55f, 0.245f);
		newNode2.height = 0.00005f;
		newNode2.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;
		_osm.tiles[currentTile.x][currentTile.y].nodes[newNode2.id] = newNode2;

		Vertex newNode3 = *foundNodes[indices[3]].first;
		newNode3.texCoords = glm::vec2(0.55f, 0.245f);
		newNode3.height = 0.000075f;
		newNode3.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;
		_osm.tiles[currentTile.x][currentTile.y].nodes[newNode3.id] = newNode3;

		/* Nodes to create quad (2 triangles) to fill gaps in the junction. */
		Vertex newNode4 = *foundNodes[indices[0]].second;
		newNode4.texCoords = glm::vec2(-0.05f, 0.245f);
		newNode4.height = 0.00005f;
		newNode4.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;
		_osm.tiles[currentTile.x][currentTile.y].nodes[newNode4.id] = newNode4;

		Vertex newNode5 = *foundNodes[indices[1]].first;
		newNode5.texCoords = glm::vec2(0.55f, 0.245f);
		newNode5.height = 0.000075f;
		newNode5.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;
		_osm.tiles[currentTile.x][currentTile.y].nodes[newNode5.id] = newNode5;

		Vertex newNode6 = *foundNodes[indices[2]].first;
		newNode6.texCoords = glm::vec2(-0.05f, 0.245f);
		newNode6.height = 0.00005f;
		newNode6.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;
		_osm.tiles[currentTile.x][currentTile.y].nodes[newNode6.id] = newNode6;

		Vertex newNode7 = *foundNodes[indices[3]].second;
		newNode7.texCoords = glm::vec2(0.55f, 0.245f);
		newNode7.height = 0.000075f;
		newNode7.id = _osm.tiles[currentTile.x][currentTile.y].nodes.rbegin()->first + 1;
		_osm.tiles[currentTile.x][currentTile.y].nodes[newNode7.id] = newNode7;

		/* Quad to fill holes in junction. */
		way->nodeIds.emplace_back(newNode4.id);
		way->nodeIds.emplace_back(newNode6.id);
		way->nodeIds.emplace_back(newNode7.id);

		way->nodeIds.emplace_back(newNode7.id);
		way->nodeIds.emplace_back(newNode5.id);
		way->nodeIds.emplace_back(newNode4.id);

		/* 1st new triangle to create junction. */
		way->nodeIds.emplace_back(newNode1.id);
		way->nodeIds.emplace_back(newNode.id);
		way->nodeIds.emplace_back(newNode3.id);

		/* 2nd new triangle to create junction. */
		way->nodeIds.emplace_back(newNode2.id);
		way->nodeIds.emplace_back(newNode.id);
		way->nodeIds.emplace_back(newNode0.id);
	}
}

/// <summary>Calculates the index into the indices array for a junction which is a roundabout, this index is determined by finding the two nodes
/// in 'foundNodes' which share the same co-ordinates as two nodes which are not part of the junction i.e. the connecting road way.</summary>
/// <param name="foundWays">Relevant ways (i.e. ways holding nodes that are connected to the junction) generated by the calling function.</param>
/// <param name="foundNodes">Relevant nodes (i.e. the nodes that make up the junction) generated by the calling function.</param>
/// <returns>Calculated index into indices array.</returns>
uint32_t NavDataProcess::calculateRoundaboutTexCoordIndices(std::map<uint64_t, Way*>& foundWays, std::vector<std::pair<Vertex*, Vertex*>>& foundNodes)
{
	bool i0 = false;
	bool i1 = false;
	bool i2 = false;

	// Iterate over all 'found ways' (ways which share nodes with the junction) and find the two nodes that connect to the road which is not a roundabout.
	for (auto it = foundWays.begin(); it != foundWays.end(); ++it)
	{
		// Skip any ways which are also roundabouts leaving only the connecting road.
		if (it->second->isRoundabout) continue;

		// Iterate through nodes in way and compare the co-ordinates to the previously 'found nodes'.
		for (uint32_t j = 0; j < it->second->nodeIds.size(); ++j)
		{
			Vertex* next = &_osm.nodes.find(it->second->nodeIds[j])->second;

			if (compareReal(next->coords.x, foundNodes[0].first->coords.x) && compareReal(next->coords.y, foundNodes[0].first->coords.y))
				i0 = true;
			else if (compareReal(next->coords.x, foundNodes[1].first->coords.x) && compareReal(next->coords.y, foundNodes[1].first->coords.y))
				i1 = true;
			else if (compareReal(next->coords.x, foundNodes[2].first->coords.x) && compareReal(next->coords.y, foundNodes[2].first->coords.y))
				i2 = true;
		}
	}

	// Determine index based on which boolean flags have been set.
	if (i0 && i1 && !i2)
		return 1;
	else if (i0 && i2 && !i1)
		return 0;
	else
		return 2;
}

/// <summary>Calculates the index into the indices array for a junction which is a T-junction, this index is determined by finding the
/// smallest angle between pairs of vectors created from found nodes and nodes which reside in found ways, the smallest angle found represents
/// the line where the two roads intersect i.e. the connection points between the two roads.</summary>
/// <param name="foundWays">Relevant ways(i.e.ways holding nodes that are connected to the junction) generated by the calling function.</param>
/// <param name="foundNodes">Relevant nodes(i.e.the nodes that make up the junction) generated by the calling function.</param>
/// <param name="way">The current way which represents the junction.</param>
/// <returns> Calculated index into indices array.</returns>
uint32_t NavDataProcess::calculateT_JunctionTexCoordIndices(std::map<uint64_t, Way*>& foundWays, std::vector<std::pair<Vertex*, Vertex*>>& foundNodes, Way* way)
{
	double currentClosestAngle = std::numeric_limits<double>::max();
	uint32_t currentIndex = 0;

	std::string baseName = getAttributeName(way->tags.data(), way->tags.size());

	for (auto it = foundWays.begin(); it != foundWays.end(); ++it)
	{
		// Ignore ways which do not have the same road name or are not of the same type.
		std::string name = getAttributeName(it->second->tags.data(), it->second->tags.size());
		if ((((!baseName.empty() && !name.empty()) && baseName.compare(name) != 0) && (way->roadType != it->second->roadType)) || (way->roadType != it->second->roadType)) continue;

		for (uint32_t j = 0; j < it->second->nodeIds.size(); ++j)
		{
			Vertex* next = &_osm.nodes.find(it->second->nodeIds[j])->second;

			// Compute vector between found nodes and the 'next' node which resides in 'found ways'.
			glm::dvec2 v1 = foundNodes[0].first->coords - next->coords;
			glm::dvec2 v2 = foundNodes[1].first->coords - next->coords;
			glm::dvec2 v3 = foundNodes[2].first->coords - next->coords;

			// Calculate length2 to check that the vector is non-zero.
			double len1 = glm::length2(v1);
			double len2 = glm::length2(v2);
			double len3 = glm::length2(v3);

			// If the vector length is non zero, calculate the angle and check if it is smaller that the current smallest angle found so far.
			if (!compareReal(len1, 0.0) && !compareReal(len2, 0.0))
			{
				v1 = glm::normalize(v1);
				v2 = glm::normalize(v2);
				double angle = glm::acos(glm::dot(v1, v2));

				if (angle < currentClosestAngle)
				{
					currentClosestAngle = angle;
					currentIndex = 1;
				}
			}
			if (!compareReal(len1, 0.0) && !compareReal(len3, 0.0))
			{
				v1 = glm::normalize(v1);
				v3 = glm::normalize(v3);
				double angle = glm::acos(glm::dot(v1, v3));

				if (angle < currentClosestAngle)
				{
					currentClosestAngle = angle;
					currentIndex = 0;
				}
			}
			if (!compareReal(len2, 0.0) && !compareReal(len3, 0.0))
			{
				v2 = glm::normalize(v2);
				v3 = glm::normalize(v3);
				double angle = glm::acos(glm::dot(v2, v3));

				if (angle < currentClosestAngle)
				{
					currentClosestAngle = angle;
					currentIndex = 2;
				}
			}
		}
	}
	return currentIndex;
}

/// <summary>Calculates an end cap (i.e. the smoothed/curved end to a road segment when it does not connect to another road) for a given road segment.</summary>
/// <param name="first">The first node at the start/end of the road segment.</param>
/// <param name="second">The second node at the start/end of the road segment.</param>
/// <param name="width">This directly affects the cap size.</param>
/// <returns>Array The newly generated nodes that create the end cap.</returns>
std::array<uint64_t, 2> NavDataProcess::calculateEndCaps(Vertex& first, Vertex& second, double width)
{
	// Calculate vector between end nodes.
	glm::dvec2 v1 = first.coords - second.coords;
	// Calculate a perpendicular vector.
	glm::vec3 perp = glm::normalize(glm::cross(glm::vec3(v1, 0), glm::vec3(0, 0, 1)));
	// Project the perpendicular vector by width / 2
	v1 = glm::dvec2(perp) * (width / 2.0);

	first.texCoords.y = second.texCoords.y = 0.45f;

	Vertex newNode1 = first;
	Vertex newNode2 = second;

	// Setup new nodes used for end cap.
	newNode1.coords -= v1;
	newNode1.texCoords.y = 1.0f;
	newNode1.id = _osm.nodes.rbegin()->first + 1;
	_osm.nodes[newNode1.id] = newNode1;

	newNode2.coords -= v1;
	newNode2.texCoords.y = 1.0f;
	newNode2.id = _osm.nodes.rbegin()->first + 1;
	_osm.nodes[newNode2.id] = newNode2;

	std::array<uint64_t, 2> retval;
	retval[0] = newNode1.id;
	retval[1] = newNode2.id;
	return retval;
}

/// <summary>Find the centre point of the triangle based in its height / 2.</summary>
/// <param name"p1">first point of triangle.</param>
/// <param name"p2">second point of triangle.</param>
/// <param name"p3">third point of triangle.</param>
/// <returns>Vector2 with a newly generated point.</returns>
glm::dvec2 NavDataProcess::calculateMidPoint(glm::dvec2 p1, glm::dvec2 p2, glm::dvec2 p3) const
{
	// Get centre of line.
	glm::dvec2 point = glm::dvec2((p1.x + p2.x) / 2.0, (p1.y + p2.y) / 2.0);
	// Vector from centre of line to third point in triangle.
	glm::dvec2 v1 = p3 - point;
	// Calculate vector length, for normalisation and projection.
	double len = glm::length(v1);
	v1 /= len;
	// Project the new point along v1 by the projection length.
	return point += (v1 * (len / 2.0));
}

/// <summary>Finds intersections with map bounds (if there are any) for a way that crosses a map without a node within bounds.</summary>
/// <param name="point1">Out of map bounds point.</param>
/// <param name="point2">Out of map bounds point.</param>
bool NavDataProcess::findMapIntersect(glm::dvec2& point1, glm::dvec2& point2) const
{
	glm::dvec2 newPoint1;
	glm::dvec2 newPoint2;
	double m = (point1.y - point2.y) / (point1.x - point2.x);
	double c = point1.y - m * point1.x;
	double minX = glm::min(point1.x, point2.x);
	double maxX = glm::max(point1.x, point2.x);
	double minY = glm::min(point1.y, point2.y);
	double maxY = glm::max(point1.y, point2.y);
	bool mapIntersect = false;

	// Check if there is an intersection on the left side
	double y = m * _osm.bounds.min.x + c;
	if ((y >= _osm.bounds.min.y) && (y <= _osm.bounds.max.y) && (y > minY) && (y < maxY))
	{
		newPoint1 = glm::dvec2(_osm.bounds.min.x, y);
		mapIntersect = true;
	}

	// Check if there is an intersection on the top side
	double x = (_osm.bounds.max.y - c) / m;
	if ((x >= _osm.bounds.min.x) && (x <= _osm.bounds.max.x) && (x > minX) && (x < maxX))
	{
		newPoint2 = glm::dvec2(x, _osm.bounds.max.y);
		mapIntersect = true;
	}

	// Check if there is an intersection on the right side
	y = m * _osm.bounds.max.x + c;
	if ((y >= _osm.bounds.min.y) && (y <= _osm.bounds.max.y) && (y > minY) && (y < maxY))
	{
		newPoint1 = glm::dvec2(_osm.bounds.max.x, y);
		mapIntersect = true;
	}

	// Check if there is an intersection on the bottom side
	x = (_osm.bounds.min.y - c) / m;
	if ((x >= _osm.bounds.min.x) && (x <= _osm.bounds.max.x) && (x > minX) && (x < maxX))
	{
		newPoint2 = glm::dvec2(x, _osm.bounds.min.y);
		mapIntersect = true;
	}

	// If there is a map intersection, update the coordinates of the points
	if (mapIntersect)
	{
		glm::dvec2 vec1 = newPoint1 - point1;
		glm::dvec2 vec2 = newPoint2 - point1;

		point1 = (glm::length(vec1) < glm::length(vec2)) ? newPoint1 : newPoint2;
		point2 = (point1 == newPoint1) ? newPoint2 : newPoint1;
	}
	return mapIntersect;
}
